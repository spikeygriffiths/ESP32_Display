// socket

#include <WiFi.h>
const char ssid[] = "SpikeyWiFi";
const char pass[] = "spikeynonet";
const IPAddress server(192,168,1,101); // numeric IP for Raspberry Pi
const int port = 54321; // Get report from Pi on this port
WiFiClient client;
int wiFiStatus;
SCKSTATE sckState;
int sckTimerS, rptTimerS;
const char RadioAnimation[] = "/RadioAnimation";
int animateWifiMs;
int animationIndex;
bool firstConnection;

bool GetReport(char* serverReport)
{
  unsigned int reportIndex;
  char newReport[MAX_REPORT];
  reportIndex = 0;
  DebugLn("Get Report");
  newReport[reportIndex] = '\0';  // Clear buffer ready to receive new report
  if (client.available()) {  // If there's some text waiting from the socket...
    while (client.available()) newReport[reportIndex++] = client.read();  // Get all the text waiting for me
    strcpy(serverReport, newReport);  // Should guard this to stop report parsing during this operation
    return true;
  }
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
    OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_JOINING));
    rptTimerS = REPORT_TIMEOUTS; // Get report shortly after connecting
    animationIndex = 0; // So that animation starts with first icon
    animateWifiMs = 500;  // Draw animation ASAP
    firstConnection = true;
    break;
  case EVENT_TICK:
    if (sckState < SCKSTATE_CONNECTED) {
      char animationIcon[30], strVal[3];
      if ((animateWifiMs += eventArg) > 250) {
        animateWifiMs = 0;
        if (++animationIndex > 4) animationIndex = 1;
        strcpy(animationIcon, RadioAnimation);
        strcat(animationIcon, itoa(animationIndex, strVal, 10));
        strcat(animationIcon, ".jpg");
        fex.drawJpeg(animationIcon, (TFT_HEIGHT/2) - 48,3, nullptr);  // Draw JPEG directly to screen (JPEG is 96x96, hence 48 for middle)
      }
    } else if (firstConnection) {
      firstConnection = false;  // Don't show this again
      RenderHappyFace("Waiting for report...");
    }
    break;
  case EVENT_SEC:
    switch (sckState) {
    case SCKSTATE_JOINING:
      if (wiFiStatus != WL_CONNECTED) {
        Debug("WiFi.begin status:"); DebugDecLn(wiFiStatus);
        wiFiStatus = WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network:
        if ((sckTimerS += eventArg) > WIFI_JOINING_TIMEOUTS) {
          DebugLn("Timed out joining net - restart");
          OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_DISCONNECTING));  // Giving up and restarting from scratch
        }
      } else {
        IPAddress ip = WiFi.localIP();
        DebugLn("WiFi joined!");
        //Debug("IP:"); DebugLn(ip);
        if ((ip[0] == 192) && (ip[1] == 168)) { // Check that address allocated looks plausible
          DebugLn("Good IP");
          OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_CONNECTING)); // We've joined now, so next start connecting to the socket
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
    case SCKSTATE_CONNECTING:
      if (client.connect(server, port)) { // Taken from https://www.arduino.cc/en/Tutorial/WiFiWebClient
        OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_CONNECTED));
        DebugLn("Connected!");
      } else {
        DebugLn("Still connecting...");
        if ((sckTimerS += eventArg) > WIFI_CONNECTION_TIMEOUTS) OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_DISCONNECTING));
      }
      break;
    case SCKSTATE_RECONNECTING:
      if (client.connect(server, port)) { // Taken from https://www.arduino.cc/en/Tutorial/WiFiWebClient
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
          if (GetReport(serverReport)) {
            OSIssueEvent(EVENT_REPORT, serverReport);
            NewSckState(SCKSTATE_RECONNECTING); // Will need to re-connect after accepting report (don't know why)
          } else {
            if (rptTimerS > REPORT_TIMEOUTS + 5) OSIssueEvent(EVENT_REPORT, false); // We've failed to get a report from the server even after a few extra attempts, so tell system
          }
        }
      }
      break;
    }
    break;
  case EVENT_REPORT:
    if (eventArg) {
      Debug("New report from server:"); DebugLn(serverReport);
      rptTimerS = 0;
    } else {
      if (rptTimerS > REPORT_TIMEOUTS * 2) OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_DISCONNECTING)); // After a while of failing to get a report, 
      Debug("Server Fail:"); DebugDecLn(rptTimerS);
    }
    break;
  }
}
