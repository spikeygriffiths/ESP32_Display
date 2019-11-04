// BLE
// Modified from https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/examples/BLE_scan/BLE_scan.ino

/*#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5; //In seconds
int bleTimerS;
BLEScan* pBLEScan;

class BleScanCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //Serial.printf("Found Device: %s \n", advertisedDevice.toString().c_str());
    }
};

void BleEventHandler(EVENT eventId, long eventArg)
{
  switch (eventId) {
  case EVENT_POSTINIT:
    BLEDevice::init("SpikeyEsp32");
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new BleScanCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);  // less or equal setInterval value
    bleTimerS = 10; // Scan at this rate
    break;
  case EVENT_SEC:
    if ((bleTimerS += eventArg) > 10) {
      bleTimerS = 0;
      BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
      for(int idx(0); idx < foundDevices.getCount(); ++idx)
      {
        std::string deviceName;
        BLEAddress deviceAddr = foundDevices.getDevice(idx).getAddress();
        if (foundDevices.getDevice(idx).haveName()) {
          deviceName = foundDevices.getDevice(idx).getName();
        } else {
          deviceName = "No name";
        }
        std::string devAddrStr = deviceAddr.toString();
        Debug((char*)devAddrStr.c_str());
        DebugLn((char*)deviceName.c_str());
      }
    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
    }
    break;
  }
}
*/
