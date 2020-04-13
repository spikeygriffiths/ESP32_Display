// socket

#include <WiFi.h>
//#include <ESP32WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESPmDNS.h>        // Include the mDNS library

const char serverName[] = "vestapi"; // Use string URL to find vesta server on the network - can cope with router changing the address of the server
IPAddress serverIp(0,0,0,0);   // Will be updated using MDNS 
const char friendlyName[] = "esp32";  // For setHstName().  Should be adjustable!
IPAddress localIp(0,0,0,0);   // Will be udpated using WiFi.localIP()
const int port = 12345; // Get report from Pi on this port
WiFiClient client;
int wiFiStatus;
SCKSTATE sckState;
int sckTimerS, rptTimerS, sckAttempts, sckTimeoutMs;
const char RadioAnimation[] = "/RadioAnimation";
char ssid[64], pass[64];  // Assume ssid and pass fit into these strings

SCKSTATE NewSckState(SCKSTATE newState)
{
  sckTimerS = 0;  // Restart timer whenever we change state
  sckState = newState;
  OSIssueEvent(EVENT_SOCKET, sckState); // Tell system whenever we change socket state
  return sckState;
}

void WiFiEventHandler(EVENT eventId, long eventArg)
{
  switch (eventId) {
  case EVENT_POSTINIT:
    wiFiStatus = WL_IDLE_STATUS;     // the Wifi radio's status
    NewSckState(SCKSTATE_GETCREDS);
    rptTimerS = REPORT_TIMEOUTS; // Get report shortly after connecting
    break;
  case EVENT_TICK:
    switch (sckState) {
    case SCKSTATE_CONNECT:
      if (client.connect(serverIp, port)) { // Taken from https://www.arduino.cc/en/Tutorial/WiFiWebClient
        sckTimeoutMs = 1000;  // Stay connected for 1000ms at most
        NewSckState(SCKSTATE_CONNECTED);
        DebugLn("Connected!");
      } else {
        DebugLn("Failed to connect");
        NewSckState(SCKSTATE_RECONNECTING);
      }
      break;
    case SCKSTATE_CONNECTED:
      if (!client.connected()) {
        DebugLn("Lost connection before req!");
        NewSckState(SCKSTATE_RECONNECTING);
      } else {
        char reqSvr[30]; // Enough space to hold command and MAC (Typically "ReqRpt <mac>")
        byte mac[6];
        char myMacStr[18];
        WiFi.macAddress(mac);
        sprintf(myMacStr, "%02x:%02x:%02x:%02x:%02x:%02x", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
        sprintf(reqSvr, "reqrpt %s", myMacStr);
        Debug("Sending: "); Debug(reqSvr); DebugLn("");
        client.print(reqSvr); // Tell server that we would like a personalised report for us
        //newReport[reportIndex] = '\0';  // Clear buffer ready to receive new report
        NewSckState(SCKSTATE_READRPT);
      }
      break;
    case SCKSTATE_READRPT:
      if (!client.connected()) {
        DebugLn("Lost connection before reading!");
        NewSckState(SCKSTATE_RECONNECTING);
      } else {
        String strReport;
        if (client.available()) {
          strReport = client.readStringUntil('\n');
          strReport.toCharArray(serverReport, sizeof(serverReport));  // Should guard this to stop report parsing during this operation
          OSIssueEvent(EVENT_REPORT, true);
          NewSckState(SCKSTATE_CLOSE);
        } else {
          if ((sckTimeoutMs -= eventArg) <= 0) {
            OSIssueEvent(EVENT_REPORT, false);  // Didn't get a report before giving up
            NewSckState(SCKSTATE_CLOSE);
          }
        }
      }
      break;
    case SCKSTATE_CLOSE:   // Close the socket once we'ev read the report or given up on the server
      client.stop();
      NewSckState(SCKSTATE_FOUNDSVR);
      break;
    }
    break;
  case EVENT_SEC:
    switch (sckState) {
    case SCKSTATE_GETCREDS:
      if (GetWiFiCredentials(ssid, pass)) {
        NewSckState(SCKSTATE_JOINING);
      }
      break;
    case SCKSTATE_JOINING:
      if (wiFiStatus != WL_CONNECTED) {
        Debug("WiFi.begin status:"); DebugDecLn(wiFiStatus);
        Debug("WiFi ssid:"); Debug(ssid); /*Debug(", pass:"); Debug(pass);*/ DebugLn("");
        wiFiStatus = WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network:
        if ((sckTimerS += eventArg) > WIFI_JOINING_TIMEOUTS) {
          DebugLn("Timed out joining net - restart");
          NewSckState(SCKSTATE_DISCONNECTING);  // Giving up and restarting from scratch
        }
      } else {
        localIp = WiFi.localIP();
        DebugLn("WiFi joined! IP:"); Serial.println(localIp.toString());
        if ((localIp[0] == 192) && (localIp[1] == 168)) { // Check that address allocated looks plausible
          DebugLn("Good IP");
          NewSckState(SCKSTATE_STARTINGMDNS); // We've joined now, so next start MDNS and then find the server
        } else {
          DebugLn("Silly IP - restart");
          NewSckState(SCKSTATE_DISCONNECTING);  // Start all over again
        }
      }
      break;
    case SCKSTATE_DISCONNECTING:
      DebugLn("Wifi.disconnect");
      WiFi.disconnect();
      wiFiStatus = WL_IDLE_STATUS;
      NewSckState(SCKSTATE_JOINING);
      break;
    case SCKSTATE_STARTINGMDNS:
      //WiFi.setHostName(friendlyName); // Won't compile
      if (MDNS.begin(friendlyName))  // ToDo: Allow for failure count, to restart after too many fails?
        NewSckState(SCKSTATE_FINDINGSVR);   // Named our device, so now find Vesta
      break;
    case SCKSTATE_FINDINGSVR:
      serverIp = MDNS.queryHost(serverName);
      if (serverIp[0] == localIp[0]) { // If the first byte of the IP address of the server is also our IP address, then we're on the same network
        Debug("VestaPi's ipaddress:"); Serial.println(serverIp.toString());
        NewSckState(SCKSTATE_FOUNDSVR);
      } // else after a while try and re-start WiFi?
      break;
    case SCKSTATE_FOUNDSVR:
      if ((rptTimerS += eventArg) > REPORT_TIMEOUTS) {
        sckAttempts = 0;  // Clear counter of attempts to connect to socket
        rptTimerS = 0;  // Make sure we don't try and get another report for a while
        NewSckState(SCKSTATE_CONNECT);  // So that we can request a report
      }
      break;
    case SCKSTATE_RECONNECTING:
      if (sckAttempts++ < MAX_SCK_ATTEMPTS) {
        NewSckState(SCKSTATE_CONNECT);  // So that we can request a report
      } else {
        NewSckState(SCKSTATE_DISCONNECTING);
      }
      break;
    }
    if (!*ssid || !*pass) {
      RenderSadFace("No wifi details!");
      NewSckState(SCKSTATE_GETCREDS); // Keep trying to get wifi credentials from file
    }
    break;
  case EVENT_REPORT:
    if (eventArg) {
      Debug("New report from server:"); DebugLn(serverReport);
    } else {
      if (rptTimerS > REPORT_TIMEOUTS * 2) NewSckState(SCKSTATE_DISCONNECTING); // After a while of failing to get a report, 
      Debug("Server Fail:"); DebugDecLn(rptTimerS);
    }
    break;
  }
}

bool GetWiFiCredentials(char* ssid, char* pass)
{
 File fileHdl = SPIFFS.open("/wifi.txt");
 char fileText[64]; // Assume no wifi details are longer than this!
 
  if (!fileHdl) {
    Debug("Failed to open wifi.txt for reading");
    return false;
  }
  while(fileHdl.available()) {
    int len = (fileHdl.readBytesUntil('\n', fileText, sizeof(fileText)));
    fileText[len] = 0;  // Ensure string terminated
    if (fileText[len-1] == '\r') fileText[len-1] = 0;  // If '\r' before '\n', then remove it for Windows
    Debug(fileText);
    if (0 == strncmp(fileText, "ssid", 4)) strcpy(ssid, fileText+5);
    if (0 == strncmp(fileText, "pass", 4)) strcpy(pass, fileText+5);
  }
  fileHdl.close();
  Debug("WiFi ssid:"); Debug(ssid);
  return true;
}
