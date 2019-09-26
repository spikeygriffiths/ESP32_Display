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

// Function prototypes
void wifi_scan();

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
//TFT_eSPI tft = TFT_eSPI();            // Invoke custom library
TFT_eFEX fex = TFT_eFEX(&tft);    // Create TFT_eFX object "efx" with pointer to "tft" object

//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}

void showVoltage()
{
  static uint64_t timeStamp = 0;
  tft.setTextColor(TFT_YELLOW, TFT_BLUE);
  tft.setTextSize(2);
  if (millis() - timeStamp > 1000) {
    timeStamp = millis();
    uint16_t v = analogRead(ADC_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    String voltage = String(battery_voltage) + "V";
    Serial.println(voltage);
    tft.fillScreen(TFT_BLACK);
    tft.fillRoundRect(15, 88, 105, 16 + 16 + 6 + 24, 10, TFT_BLUE);
    tft.setCursor(28, 100); // (0,0) = Set cursor at top left of screen
    tft.println("Voltage");
    tft.setCursor(40, 122);
    tft.println(voltage);
    //tft.setTextDatum(MC_DATUM);
    //tft.drawString(voltage,  tft.width() / 2, tft.height() / 2 );
  }
}

void button_init()
{
  btn1.setLongClickHandler([](Button2 & b) {
    btnClick = false;
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
    Serial.println("Detect Voltage..");
    btnClick = true;
  });

  btn2.setPressedHandler([](Button2 & b) {
    btnClick = false;
    Serial.println("btn press wifi scan");
    wifi_scan();
  });
}

void button_loop()
{
  btn1.loop();
  btn2.loop();
}

bool wifi_join(void)
{
  int numTries = 5;
  tft.setRotation(1);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.print("Connecting to:");
  tft.println(ssid);
  tft.print("pass:");
  tft.println(pass);
  while ((status != WL_CONNECTED) && (--numTries != 0)) {
    status = WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network:
    delay(2000);    // wait for connection:
  }
  if (status == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    tft.print("Wifi:");
    tft.println(ssid);
    tft.print("IP:");
    tft.println(ip);
    return true;
  } else {
    tft.print("Failed to connect...");
    return false;
  }
}

void wifi_scan()
{
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);

  tft.drawString("Scan Network", tft.width() / 2, tft.height() / 2);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int16_t n = WiFi.scanNetworks();
  tft.fillScreen(TFT_BLACK);
  if (n == 0) {
    tft.drawString("no networks found", tft.width() / 2, tft.height() / 2);
  } else {
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(0, 0);
    Serial.printf("Found %d net\n", n);
    for (int i = 0; i < n; ++i) {
      sprintf(buff,
              "%s(%d)",
              WiFi.SSID(i).c_str(),
              WiFi.RSSI(i));
      tft.println(buff);
    }
  }
  WiFi.mode(WIFI_OFF);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
  tft.init();
  tft.setRotation(1);
  //tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0,  240, 135, ttgo);
  espDelay(5000);
  tft.setRotation(0);
  tft.fillScreen(TFT_GREEN);
  //espDelay(1000);

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

  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
  //Check type of calibration value used to characterize ADC
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
    vref = adc_chars.vref;
  } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
    Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
  } else {
    Serial.println("Default Vref: 1100mV");
  }
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  fex.listSPIFFS(); // Lists the files so you can see what is in the SPIFFS
}

char* GetNextItem(char* dict)
{
  char ch;
  Serial.print("In GetNextItem");
  while (ch = *dict++) {
    if (ch == ',') break; // Advance beyond ',' to next <item>:<val> pair
    if (('}' == ch) || ('{' == ch)) {
      Serial.println(", found start or end of dict");
      return dict-1;  // Stop at start or end of dict
    }
  }
  Serial.println(", found comma");
  return dict;  // Now pointing to next <item>
}

bool CmpItem(char* target, char* dict)
{
  Serial.print("CmpItem:"); Serial.print(target);
  while (*dict++ != '\'') ; // Get opening quote
  Serial.print(" in "); Serial.println(dict);
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
        Serial.println("Got match!");
        while (*dict++ != ':') ; // Advance beyond ':'
        while (*dict++ != '\'') ; // and opening quote, to get to start of <val>
        return dict;  // Now pointing to <val> associated with <item>
      } else Serial.println("Advance to next item...");
      dict++; // Advance beyond start of this item in order to get to next one...
    }
  }
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

void RenderWeather(char* weatherReport)
{
  char icon[32], jpegName[40];  // jpegName must be at least 5 chars longer than icon
  char period[6], synopsis[64];
  char minTemp[5], maxTemp[5];
  
  Serial.print("Weather Report:"); Serial.println(weatherReport);
  if (*weatherReport != '{') {
    Serial.println("Not a dict!");
    return;
  }
  // Parse the weatherReport (as Python dict, as <key>:<value>,<key>:<value>)
  GetDictVal(weatherReport, "period", period);
  GetDictVal(weatherReport, "icon", icon);
  GetDictVal(weatherReport, "maxTemp", maxTemp);
  GetDictVal(weatherReport, "minTemp", minTemp);
  // Display the results
  tft.fillScreen(TFT_WHITE);
  tft.loadFont("NotoSansBold36");   // Name of font file (library adds leading / and .vlw)
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  //tft.setTextSize(4);
  jpegName[0] = '\0';
  strcpy(jpegName, "/");
  strcat(jpegName, icon);
  strcat(jpegName, ".jpg");
  Serial.print("Jpeg:"); Serial.println(jpegName);
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
  char weatherReport[512];  // Arbitrary maximum length of weatherReport
  if (btnClick) {
    showVoltage();
  }
  button_loop();
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
        tft.println("Cannot re-connect");
        while (1);  // Loop forever
      }// else Serial.println("Reconnected!");
    }
    reportIndex = 0;
    weatherReport[reportIndex] = '\0';
    if (client.available()) {  // If there's some text waiting from the socket...
      millisUntilReport = 10*60*1000; // 10 mins until next report, now that we've seen the report
      while (client.available()) {  // If there's some text waiting from the socket...
        char ch = client.read();
        weatherReport[reportIndex++] = ch;
      }
    } else millisUntilReport = 1000; // Try again in a second if there's no report
    RenderWeather(weatherReport);
  }
  //} else Serial.println("Client not connected via WiFi");
}
