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
//#include "esp_adc_cal.h"
//#include "bmp.h"
#include "ESP32_Display.h"

typedef enum {
  DISPLAY_WEATHER,
  DISPLAY_TIME, // Could add more functionIds here (eg House info, House power consumption, etc.)
} DisplayFunction;

#define MAX_REPORT 1024 // Arbitrary maximum length of serverReport

// Global variables
TFT_eSPI tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
TFT_eFEX fex = TFT_eFEX(&tft);    // Create TFT_eFX object "efx" with pointer to "tft" object
char serverReport[MAX_REPORT];
unsigned long StartTime = millis();
unsigned long oldMillis;
unsigned long serverFail; // Count of failures to get a response from the server
unsigned long millisUntilReport = 0;
bool topBtnLong, topBtnTap;
bool btmBtnLong, btmBtnTap;
bool more = false;
DisplayFunction fn = DISPLAY_WEATHER;
DisplayFunction oldFn;

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
void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
  tft.init();
  tft.setSwapBytes(true);

  OpenSocket();
  button_init();
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  fex.listSPIFFS(); // Lists the files so you can see what is in the SPIFFS
  serverFail = 0;
}

void loop()
{
  unsigned long newMillis, elapsedMillis;
  bool redraw;

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
    if (GetReport(serverReport)) {
      serverFail = 0;
      redraw = true;  // Got a new report, so force a redraw
      Serial.print("New report from server:"); Serial.println(serverReport);
    } else {
      serverFail++;
      if (serverFail > 30) redraw = true; // Force a redraw
      Serial.print("Server Fail:"); Serial.println(serverFail);
      millisUntilReport = 1000; // Try again in a second if there's no report
    }
  }
  if (redraw) {
    if (serverFail > 30) {
      RenderSadFace("Server down");
    } else {
      switch (fn) {
      case DISPLAY_WEATHER:
        DisplayWeather(serverReport, more, (fn != oldFn));
        break;
      case DISPLAY_TIME:
        DisplayDateTime(serverReport, more, (fn != oldFn));
        break;
      } // end switch()
      oldFn = fn;
    } // end if serverFail
  } // end if redraw
}
