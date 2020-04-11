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
int sckTimerS, rptTimerS, rptAttemps;
const char RadioAnimation[] = "/RadioAnimation";
char ssid[64], pass[64];  // Assume ssid and pass fit into these strings
int animateWifiMs;
int animationIndex;
bool firstConnection;

bool GetReport(char* serverReport)
{
  unsigned int reportIndex = 0;
  String strReport;
  char newReport[1024];
  char reqSvr[30]; // Enough space to hold command and MAC (Typically "ReqRpt <mac>")
  byte mac[6];
  char myMacStr[18];
  WiFi.macAddress(mac);
  sprintf(myMacStr, "%02x:%02x:%02x:%02x:%02x:%02x", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  sprintf(reqSvr, "reqrpt %s", myMacStr);
  Debug("Sending: "); Debug(reqSvr); DebugLn("");
  //client.write(reqSvr, strlen(reqSvr)); // Tell server that we would like a personalised report for us
  client.print(reqSvr); // Tell server that we would like a personalised report for us
  newReport[reportIndex] = '\0';  // Clear buffer ready to receive new report
  if (client.available()) {  // If there's some text waiting from the socket...
    while (client.available()) newReport[reportIndex++] = client.read();  // Get all the text waiting for me
    strcpy(serverReport, newReport);  // Should guard this to stop report parsing during this operation
    return true;
  }
  /*if (client.available()) {
    strReport = client.readStringUntil('\n');
    strReport.toCharArray(serverReport, sizeof(serverReport));  // Should guard this to stop report parsing during this operation
    return true;
  }*/
  return false;
}

SCKSTATE NewSckState(SCKSTATE newState)
{
  sckTimerS = 0;  // Restart timer whenever we change state
  sckState = newState;
  return sckState;
}

void WiFiEventHandler(EVENT eventId, long eventArg)
{
  switch (eventId) {
  case EVENT_POSTINIT:
    wiFiStatus = WL_IDLE_STATUS;     // the Wifi radio's status
    OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_GETCREDS));
    rptTimerS = REPORT_TIMEOUTS; // Get report shortly after connecting
    rptAttemps = 0;
    animationIndex = 0; // So that animation starts with first icon
    animateWifiMs = 500;  // Draw animation ASAP
    firstConnection = true;
    
    break;
  case EVENT_TICK:
    if (sckState == SCKSTATE_GETCREDS) {
    } else if (sckState >= SCKSTATE_JOINING && sckState < SCKSTATE_CONNECTED && *ssid) {
      char animationIcon[30], strVal[3];
      if ((animateWifiMs += eventArg) > 250) {
        animateWifiMs = 0;
        if (++animationIndex > 4) animationIndex = 0;
        if (animationIndex) {
          strcpy(animationIcon, RadioAnimation);
          strcat(animationIcon, itoa(animationIndex, strVal, 10));
          strcat(animationIcon, ".jpg");
          fex.drawJpeg(animationIcon, (TFT_HEIGHT/2) - 60,3, nullptr);  // Draw JPEG directly to screen (JPEG is 120x96, hence 60 for middle)
        } else {
          tft.fillRect((TFT_HEIGHT/2) - 60,3, 120,94, TFT_WHITE);// Draw blank rectangle
        }
        tft.setTextColor(TFT_BLACK, TFT_WHITE);
        tft.loadFont("Cambria-24");   // Name of font file (library adds leading / and .vlw)
        if (sckState <= SCKSTATE_DISCONNECTING) {
          PrettyLine("   Joining WiFi   ", 100, JUSTIFY_CENTRE);
        } else {
          PrettyLine("  Finding Vesta  ", 100, JUSTIFY_CENTRE);
        }
        tft.unloadFont(); // To recover RAM
      }
    } else if (sckState >= SCKSTATE_CONNECTED && firstConnection) {
      firstConnection = false;  // Don't show this again
      RenderHappyFace("Waiting for report...");
    }
    break;
  case EVENT_SEC:
    if (!*ssid || !*pass) {
      RenderSadFace("No wifi details!");
      NewSckState(SCKSTATE_GETCREDS); // Get wifi credentials from file
    }
    switch (sckState) {
    case SCKSTATE_GETCREDS:
      if (GetWiFiCredentials(ssid, pass)) {
        OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_JOINING));
      }
      break;
    case SCKSTATE_JOINING:
      if (wiFiStatus != WL_CONNECTED) {
        Debug("WiFi.begin status:"); DebugDecLn(wiFiStatus);
        Debug("WiFi ssid:"); Debug(ssid); Debug(", pass:"); DebugLn(pass);
        wiFiStatus = WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network:
        if ((sckTimerS += eventArg) > WIFI_JOINING_TIMEOUTS) {
          DebugLn("Timed out joining net - restart");
          OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_DISCONNECTING));  // Giving up and restarting from scratch
        }
      } else {
        localIp = WiFi.localIP();
        DebugLn("WiFi joined! IP:"); Serial.println(localIp.toString());
        if ((localIp[0] == 192) && (localIp[1] == 168)) { // Check that address allocated looks plausible
          DebugLn("Good IP");
          OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_STARTINGMDNS)); // We've joined now, so next start MDNS and then find the server
        } else {
          DebugLn("Silly IP - restart");
          OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_DISCONNECTING));  // Start all over again
        }
      }
      break;
    case SCKSTATE_DISCONNECTING:
      DebugLn("Wifi.disconnect");
      WiFi.disconnect();
      wiFiStatus = WL_IDLE_STATUS;
      OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_JOINING));
      break;
    case SCKSTATE_STARTINGMDNS:
      //WiFi.setHostName(friendlyName); // Won't compile
      if (MDNS.begin(friendlyName))  // ToDo: Allow for failure count, to restart after too many fails?
        OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_FINDINGSVR));   // Named our device, so now find Vesta
      break;
    case SCKSTATE_FINDINGSVR:
      serverIp = MDNS.queryHost(serverName);
      if (serverIp[0] == localIp[0]) { // If the first byte of the IP address of the server is also our IP address, then we're on the same network
        Debug("VestaPi's ipaddress:"); Serial.println(serverIp.toString());
        OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_CONNECTING));
      } // else after a while try and re-start WiFi?
      break;
    case SCKSTATE_CONNECTING:
      if (client.connect(serverIp, port)) { // Taken from https://www.arduino.cc/en/Tutorial/WiFiWebClient
        OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_CONNECTED));
        DebugLn("Connected!");
      } else {
        DebugLn("Still connecting...");
        if ((sckTimerS += eventArg) > WIFI_CONNECTION_TIMEOUTS) OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_DISCONNECTING));
      }
      break;
    case SCKSTATE_RECONNECTING:
      if (client.connect(serverIp, port)) { // Taken from https://www.arduino.cc/en/Tutorial/WiFiWebClient
        NewSckState(SCKSTATE_CONNECTED);  // Silent re-connection (don't issue event)
      } else {
        if ((sckTimerS += eventArg) > WIFI_CONNECTION_TIMEOUTS) OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_DISCONNECTING));
      }
      break;    
    case SCKSTATE_CONNECTED:
      if (!client.connected()) {
        DebugLn("Lost connection!");
        OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_RECONNECTING));
      } else {
        if ((rptTimerS += eventArg) > REPORT_TIMEOUTS) {
          rptTimerS = 0;  // Make sure we don't try and get another report for a while, even if this one fails
          if (GetReport(serverReport)) {
            rptAttemps = 0;
            OSIssueEvent(EVENT_REPORT, serverReport);
            NewSckState(SCKSTATE_RECONNECTING); // Will need to re-connect after accepting report (don't know why)
          } else {
            rptAttemps++;
            if (rptAttemps > 5) OSIssueEvent(EVENT_REPORT, false); // We've failed to get a report from the server even after a few extra attempts, so tell system
          }
        }
      }
      break;
    }
    break;
  case EVENT_REPORT:
    if (eventArg) {
      Debug("New report from server:"); DebugLn(serverReport);
    } else {
      if (rptTimerS > REPORT_TIMEOUTS * 2) OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_DISCONNECTING)); // After a while of failing to get a report, 
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
