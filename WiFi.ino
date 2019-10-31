// socket

const char ssid[] = "SpikeyWiFi";
const char pass[] = "spikeynonet";
const IPAddress server(192,168,1,101); // numeric IP for Raspberry Pi
const int port = 54321;
WiFiClient client;
int wiFiStatus;
SCKSTATE sckState;
int sckTimerS, rptTimerS;

void GetReport(char* serverReport)
{
  unsigned int reportIndex;
  char newReport[MAX_REPORT];
  reportIndex = 0;
  DebugLn("Get Report");
  newReport[reportIndex] = '\0';  // Clear buffer ready to receive new report
  if (client.available()) {  // If there's some text waiting from the socket...
    while (client.available()) newReport[reportIndex++] = client.read();  // Get all the text waiting for me
    strcpy(serverReport, newReport);  // Should guard this to stop report parsing during this operation
    OSIssueEvent(EVENT_REPORT, serverReport);
  } else OSIssueEvent(EVENT_REPORT, false);
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
    break;
  case EVENT_SEC:
    switch (sckState) {
    case SCKSTATE_JOINING:
      if (wiFiStatus != WL_CONNECTED) {
        Debug("WiFi.begin status:"); DebugDecLn(wiFiStatus);
        wiFiStatus = WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network:
        if ((sckTimerS += eventArg) > 10) {
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
        rptTimerS = 10; // Get report ASAP!
        DebugLn("Connected!");
      } else {
        if ((sckTimerS += eventArg) > 10) OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_CONNECTING));
      }
      break;
    case SCKSTATE_CONNECTED:
      if ((rptTimerS += eventArg) > 10) {
        if (client.connected()) {
          GetReport(serverReport);
        } else { // No longer connected
          DebugLn("Lost connection!");
          OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_CONNECTING));
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
      if (rptTimerS > 30) OSIssueEvent(EVENT_SOCKET, NewSckState(SCKSTATE_DISCONNECTING)); // After a while of failing to get a report, 
      Debug("Server Fail:"); DebugDecLn(rptTimerS);
    }
    break;
  }
}
