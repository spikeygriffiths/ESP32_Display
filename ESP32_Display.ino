#include <TFT_eSPI.h>
#include <TFT_eFEX.h>
#include <SPI.h>

// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>

#include "SPIFFS.h" // Needed for ESP32 only
#include "WiFi.h"
#include <Wire.h>
#include <Button2.h>
#include "esp_adc_cal.h"
#include "bmp.h"

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23

#define TFT_BL          4  // Display backlight control pin
#define ADC_EN          14
#define ADC_PIN         34
#define BUTTON_1        35
#define BUTTON_2        0

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

typedef enum {
  DISPLAY_WEATHER,
  DISPLAY_TIME
} DisplayFunction;

// Function prototypes

// Global variables
char buff[512];
int vref = 1100;
int btnClick = false;
int status = WL_IDLE_STATUS;     // the Wifi radio's status
const char ssid[] = "SpikeyWiFi";
const char pass[] = "spikeynonet";
const IPAddress server(192,168,1,100); // numeric IP for Raspberry Pi
const int port = 54321;
WiFiClient client;
unsigned long StartTime = millis();
unsigned long oldMillis;
unsigned long millisUntilReport = 0;
bool topBtnLong, topBtnTap;
bool btmBtnLong, btmBtnTap;
bool more = false;
DisplayFunction fn = DISPLAY_WEATHER;

//TFT_eSPI tft = TFT_eSPI();            // Invoke custom library
TFT_eFEX fex = TFT_eFEX(&tft);    // Create TFT_eFX object "efx" with pointer to "tft" object

//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}

void button_init()
{
  btn1.setLongClickHandler([](Button2 & b) {
    topBtnLong = true;

    int r = digitalRead(TFT_BL);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Press again to wake up",  tft.width() / 2, tft.height() / 2 );
    espDelay(6000);
    digitalWrite(TFT_BL, !r);

    tft.writecommand(TFT_DISPOFF);
    tft.writecommand(TFT_SLPIN);
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
  });
  btn1.setPressedHandler([](Button2 & b) {
    Serial.println("Top Btn tap");
    topBtnTap = true;
  });

  btn2.setPressedHandler([](Button2 & b) {
    Serial.println("Btm Btn tap");
    btmBtnTap = true;
  });
}

void button_loop()
{
  btn1.loop();
  btn2.loop();
}

bool wifi_join(void)
{
  int numTries = 10;
  tft.setRotation(1);
  while (true) {
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.println("Connecting to:"); tft.println(ssid);
    //tft.print("pass:"); tft.println(pass); // Only print password to display for testing...
    while ((status != WL_CONNECTED) && (--numTries != 0)) {
      status = WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network:
      delay(1000);    // wait for connection:
    }
    if (status == WL_CONNECTED) {
      IPAddress ip = WiFi.localIP();
      tft.print("IP:"); tft.println(ip);
      if ((ip[0] == 192) && (ip[1] == 168)) { // Check that address allocated looks plausible
        tft.println("Connected!");
        return true;
      } else tft.println("Silly IP");
    } else tft.print("Failed to connect...");
    WiFi.disconnect(); // was return false;, but we always want to keep retrying
    delay(1000);    // wait for disconnection...
    tft.println("Re-trying...");
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0,  240, 135, ttgo);
  espDelay(5000);

  if (wifi_join()) {
    if (client.connect(server, port)) { // Taken from https://www.arduino.cc/en/Tutorial/WiFiWebClient
      tft.print("Sckt RPi/");
      tft.println(port);
      //client.println("Hello from ESP32!"); // Send text to socket on far side
    } else {
      tft.println("Server not available");
    }
  }
  button_init();
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  fex.listSPIFFS(); // Lists the files so you can see what is in the SPIFFS
}

char* GetNextItem(char* dict)
{
  char ch;
  //Serial.print("In GetNextItem");
  while (ch = *dict++) {
    if (ch == ',') break; // Advance beyond ',' to next <item>:<val> pair
    if (('}' == ch) || ('{' == ch)) {
      //Serial.println(", found start or end of dict");
      return dict-1;  // Stop at start or end of dict
    }
  }
  //Serial.println(", found comma");
  return dict;  // Now pointing to next <item>
}

bool CmpItem(char* target, char* dict)
{
  //Serial.print("CmpItem:"); Serial.print(target);
  while (*dict++ != '\'') ; // Get opening quote
  //Serial.print(" in "); Serial.println(dict);
  while (*target++ == *dict++) ;  // Keep advancing until we have a mismatch
  return ((*--target == '\0') && (*--dict == '\''));  // Return true if the target is finished and the source dict string is also finished, thus a perfect match
  /*if ((*--target == '\0') && (*--dict == '\'')) {
    Serial.println("Match");
    return true;
  } else {
    Serial.print("No match with ");
    Serial.println(*dict);
    return false;
  }*/
}

char* FindItem(char* dict, char* item)
{
  Serial.print("FindItem:"); Serial.println(item);
  if ('{' == *dict) { // First char of dict is opening curly brace, so skip it
    while ('}' != *dict) {  // Exit if we reach the end of the dict
      dict = GetNextItem(dict); // Advance dict to start of <item> (after comma), or to close curly brace
      if ('}' == *dict) break;  // Stop at end of dict
      if (CmpItem(item, dict)) {
        //Serial.println("Got match!");
        while (*dict++ != ':') ; // Advance beyond ':'
        while (*dict++ != '\'') ; // and opening quote, to get to start of <val>
        return dict;  // Now pointing to <val> associated with <item>
      } //else Serial.println("Advance to next item...");
      dict++; // Advance beyond start of this item in order to get to next one...
    }
  }
  Serial.println("Not a dict!");
  return 0; // Indicate we've not found the item
}

void GetDictVal(char* dict, char* item, char* val)
{
  char* answer;
  answer = FindItem(dict, item);  // Try and find item in dict.  Result is pointer to beginning of <val>, immediately after opening quote
  if (answer) {
    while (*answer != '\'') *val++ = *answer++; // Copy each character from <val> until we hit the terminating quote
    *val = '\0';  // Terminate result by replacing close quote with \0
  } else {
    strcpy("N/A", val);
  }
}

void RenderTimeDetail(char* report)
{
  char timeText[32];
  char monthText[20], dayOfMonthText[5], dayOfWeekText[12];
  int pix;
  
  Serial.println("Time text");
  // Parse the report as Python dict, as {<key>:<value>,...}
  GetDictVal(report, "timeText", timeText);
  GetDictVal(report, "dayOfWeekText", dayOfWeekText);
  GetDictVal(report, "dayOfMonthText", dayOfMonthText);
  GetDictVal(report, "monthText", monthText);
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.loadFont("Cambria-24");   // Name of font file (library adds leading / and .vlw)
  pix = tft.textWidth(timeText);
  tft.setCursor((120 - pix/ 2), 20);
  tft.print(timeText);
  tft.setCursor(10, 110);
  tft.print(dayOfWeekText);
  tft.print(", ");
  tft.print(dayOfMonthText);
  tft.print(" ");
  tft.print(monthText);
  tft.unloadFont(); // To recover RAM 
}

void RenderTimeDigits(char* report)
{
  char timeDigits[6];
  char monthText[20], dayOfMonthText[5], dayOfWeekText[12];
  
  Serial.println("Time digits");
  // Parse the report as Python dict, as {<key>:<value>,...}
  GetDictVal(report, "dayOfWeekText", dayOfWeekText);
  GetDictVal(report, "dayOfMonthText", dayOfMonthText);
  GetDictVal(report, "monthText", monthText);
  GetDictVal(report, "timeDigits", timeDigits);
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.loadFont("Cambria-Bold-72");   // Name of font file (library adds leading / and .vlw)
  //tft.setTextSize(8); // Very blocky
  tft.setCursor(20, 20);
  tft.print(timeDigits);
  tft.unloadFont(); // To recover RAM
  tft.loadFont("Cambria-24");   // Name of font file (library adds leading / and .vlw)
  tft.setCursor(10, 110);
  tft.print(dayOfWeekText);
  tft.print(", ");
  tft.print(dayOfMonthText);
  tft.print(" ");
  tft.print(monthText);
  tft.unloadFont(); // To recover RAM
}

void RenderWeatherDetail(char* report)
{
  char icon[32], jpegName[40];  // jpegName must be at least 5 chars longer than icon
  char period[6], cloudText[64];
  char windDir[6], windSpeed[6], windText[64];
  int windDegrees;
  
  Serial.println("Weather Detail");
  // Parse the report as Python dict, as {<key>:<value>,...}
  GetDictVal(report, "period", period);
  GetDictVal(report, "windDir", windDir);
  GetDictVal(report, "windSpeed", windSpeed);
  GetDictVal(report, "windText", windText);
  GetDictVal(report, "cloudText", cloudText);
  if (!strcmp(cloudText, "N/A")) return;  // Bail if no weather!
  // Convert windDir into icon name for wind arrows
  windDegrees = ((atoi(windDir)+22) / 45) % 8; // Get direction to nearest 45'
  switch (windDegrees) {
    case 0: strcpy(icon, "North"); break;
    case 1: strcpy(icon, "NorthEast"); break;
    case 2: strcpy(icon, "East"); break;
    case 3: strcpy(icon, "SouthEast"); break;
    case 4: strcpy(icon, "South"); break;
    case 5: strcpy(icon, "SouthWest"); break;
    case 6: strcpy(icon, "West"); break;
    case 7: strcpy(icon, "NorthWest"); break;
  }
  Serial.println(icon);
  // Display the results
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  jpegName[0] = '\0';
  strcpy(jpegName, "/");
  strcat(jpegName, icon);
  strcat(jpegName, ".jpg");
  Serial.print("Wind Jpeg:"); Serial.println(jpegName);
  fex.drawJpeg(jpegName, 0,3, nullptr);  // Draw JPEG directly to screen
  tft.loadFont("NotoSansBold36");   // Name of font file (library adds leading / and .vlw)
  tft.setCursor(150, 10);
  tft.print(period);
  tft.unloadFont(); // To recover RAM
  tft.loadFont("Cambria-24");  //was NotoSansBold15");   // Name of font file (library adds leading / and .vlw)
  tft.setCursor(20, 25);  // Middle of wind icon
  tft.print(windSpeed);
  tft.setCursor(10, 72);
  tft.print(windText);
  tft.setCursor(10, 94);
  tft.print(cloudText);
  tft.unloadFont(); // To recover RAM
}

void RenderWeather(char* report)
{
  char icon[32], jpegName[40];  // jpegName must be at least 5 chars longer than icon
  char period[6];
  char minTemp[5], maxTemp[5];
  
  Serial.println("Weather Report");
  // Parse the report as Python dict, as {<key>:<value>,...}
  GetDictVal(report, "period", period);
  GetDictVal(report, "icon", icon);
  GetDictVal(report, "maxTemp", maxTemp);
  GetDictVal(report, "minTemp", minTemp);
  if (!strcmp(icon, "N/A")) return;
  // Display the results
  tft.fillScreen(TFT_WHITE);
  tft.loadFont("NotoSansBold36");   // Name of font file (library adds leading / and .vlw)
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  jpegName[0] = '\0';
  strcpy(jpegName, "/");
  strcat(jpegName, icon);
  strcat(jpegName, ".jpg");
  Serial.print("Cloud Jpeg:"); Serial.println(jpegName);
  fex.drawJpeg(jpegName, 0,3, nullptr);  // Draw JPEG directly to screen
  tft.setCursor(150, 10);
  tft.print(period);
  tft.setCursor(150, 65);
  tft.print(maxTemp);
  tft.setCursor(150, 100);
  tft.print(minTemp);
  tft.unloadFont(); // To recover RAM
}

void loop()
{
  unsigned long newMillis, elapsedMillis;
  unsigned int reportIndex;
  bool redraw;
  char serverReport[512];  // Arbitrary maximum length of serverReport

  button_loop();
  redraw = false;
  if (topBtnTap) {
    topBtnTap = false;  // Acknowledge tap now
    switch (fn) {
      case DISPLAY_WEATHER: fn = DISPLAY_TIME; break;
      case DISPLAY_TIME: fn = DISPLAY_WEATHER; break;
    }
    more = false; // New functions always start with overview
    redraw = true;
  }
  if (btmBtnTap) {
    btmBtnTap = false;  // Acknowledge tap now
    more ^= true; // flip between true & false
    redraw = true;
  }
  newMillis = millis();
  elapsedMillis = newMillis-oldMillis;
  oldMillis = newMillis;
  if (millisUntilReport > elapsedMillis)
    millisUntilReport -= elapsedMillis;
  else {
    // Need to refresh weather report from server once/10 mins
    //if (status == WL_CONNECTED) {
    if (!client.connected()) {
      if (!client.connect(server, port)) { // Taken from https://www.arduino.cc/en/Tutorial/WiFiWebClient
        tft.println("Cannot re-connect!");
        while (1);  // Loop forever.  ToDo: Fix this to re-connect
      }// else Serial.println("Reconnected!");
    }
    reportIndex = 0;
    serverReport[reportIndex] = '\0';
    if (client.available()) {  // If there's some text waiting from the socket...
      millisUntilReport = 10*1000; // 10 secs until next report, now that we've seen the report
      while (client.available()) {  // If there's some text waiting from the socket...
        char ch = client.read();
        serverReport[reportIndex++] = ch;
      }
      redraw = true;  // Got a new report, so force a redraw
      Serial.print("New report from server:"); Serial.println(serverReport);
    } else millisUntilReport = 1000; // Try again in a second if there's no report
  }
  if (redraw) {
    // Should check for more and fn to work out what to display
    switch (fn) {
    case DISPLAY_WEATHER:
      if (more) {
        RenderWeatherDetail(serverReport);
      } else {  // less
        RenderWeather(serverReport);
      }
      break;
    case DISPLAY_TIME:
      Serial.println("Render Time");
      if (more) {
        RenderTimeDigits(serverReport);
      } else { // less
        RenderTimeDetail(serverReport);
      }
      break;
    }
  }
}
