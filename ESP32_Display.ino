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

// Global variables
TFT_eSPI tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
TFT_eFEX fex = TFT_eFEX(&tft);    // Create TFT_eFX object "efx" with pointer to "tft" object
char serverReport[MAX_REPORT];
unsigned long elapsedMs = 0;
unsigned long oldMs;

//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}

void button_init()
{
  btn1.setPressedHandler([](Button2 & b) { OSIssueEvent(EVENT_BUTTON, BTN_FUNC_TAP); });
  btn1.setLongClickHandler([](Button2 & b) { OSIssueEvent(EVENT_BUTTON, BTN_FUNC_LONG); });
  btn1.setDoubleClickHandler([](Button2 & b) { OSIssueEvent(EVENT_BUTTON, BTN_FUNC_DOUBLE); });
  btn2.setPressedHandler([](Button2 & b) { OSIssueEvent(EVENT_BUTTON, BTN_CUSTOM_TAP); });
  btn2.setLongClickHandler([](Button2 & b) { OSIssueEvent(EVENT_BUTTON, BTN_CUSTOM_LONG); });
  btn2.setDoubleClickHandler([](Button2 & b) { OSIssueEvent(EVENT_BUTTON, BTN_CUSTOM_DOUBLE); });
}

void _OSIssueEvent(EVENT eventId, long eventArg)
{
  OSEventHandler(eventId, eventArg);
  SocketEventHandler(eventId, eventArg);
  RendererEventHandler(eventId, eventArg);
}

void setup()
{
  OSIssueEvent(EVENT_INIT, 0);
  OSIssueEvent(EVENT_POSTINIT, 0);
}

void loop()
{
  unsigned long newMs, diffMs;

  newMs = millis();
  diffMs = newMs-oldMs;
  OSIssueEvent(EVENT_TICK, diffMs);  // Arg is number of milliseconds since last event_tick
  elapsedMs += diffMs;
  if (elapsedMs > 1000) {
    OSIssueEvent(EVENT_SEC, elapsedMs / 1000);  // Arg is number of seconds since last event_sec
    elapsedMs %= 1000;  // Keep any fragments of seconds for next time
  }
  oldMs = newMs;
}

void OSEventHandler(EVENT eventId, long eventArg)
{
  switch (eventId) {
  case EVENT_INIT:
    Serial.begin(115200); // Start debug
    Debug("Start\r\n");
    button_init();
    break;
  case EVENT_POSTINIT:
    if (!SPIFFS.begin()) {
      Debug("SPIFFS initialisation failed!\r\n");
      // ToDo: Show something on the display!
    } else {
      fex.listSPIFFS(); // Lists the files so you can see what is in the SPIFFS
    }
    break;
  case EVENT_TICK:
    btn1.loop();
    btn2.loop();
    break;
  }
}
