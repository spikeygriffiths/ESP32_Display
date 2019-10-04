// socket

int status = WL_IDLE_STATUS;     // the Wifi radio's status
const char ssid[] = "SpikeyWiFi";
const char pass[] = "spikeynonet";
const IPAddress server(192,168,1,100); // numeric IP for Raspberry Pi
const int port = 54321;
WiFiClient client;

bool wifi_join(void)
{
  int numTries = 10;
  tft.setRotation(1);
  while (true) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
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
    status = WL_IDLE_STATUS;
    delay(2000);    // wait for disconnection...
    tft.println("Re-trying...");
  }
}

bool OpenSocket(void)
{
  if (wifi_join()) {
    if (client.connect(server, port)) { // Taken from https://www.arduino.cc/en/Tutorial/WiFiWebClient
      tft.print("Sckt RPi/");
      tft.println(port);
      //client.println("Hello from ESP32!"); // Send text to socket on far side
      return true;
    } else {
      tft.println("Server not available");
      return false;
    }
  }
}

bool GetReport(char* serverReport)
{
  unsigned int reportIndex;
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
    return true;
  }
  return false;
}
