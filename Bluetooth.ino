// Bluetooth

#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
 
bool initBluetooth(const char *deviceName)
{
  if (!btStart()) {
    DebugLn("Failed to initialize controller");
    return false;
  }
 
  if (esp_bluedroid_init()!= ESP_OK) {
    DebugLn("Failed to initialize bluedroid");
    return false;
  }
 
  if (esp_bluedroid_enable()!= ESP_OK) {
    DebugLn("Failed to enable bluedroid");
    return false;
  }
 
  esp_bt_dev_set_device_name(deviceName);
 
  esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
  return true;
}

void BluetoothEventHandler(EVENT eventId, long eventArg)
{
  switch (eventId) {
  case EVENT_POSTINIT:
    if (!initBluetooth("SpikeyEsp32")) {
      DebugLn("Bluetooth init failed");
    }
    break;
  } // end switch (eventId)
}
