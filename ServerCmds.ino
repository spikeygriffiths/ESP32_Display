// ServerCmds

bool backlight;

void ServerCmdEventHandler(EVENT eventId, long eventArg)
{
  char cmd[30]; // Any command should fit in this
  switch (eventId) {
  case EVENT_INIT:
    backlight = true; // Assume backlight turned on at startup
    break;
  case EVENT_BUTTON:
    if (!backlight) {
      SetBacklight(true);  // Turn on backlight if we have any user activity.  Need timeout?
    }
    break;
  case EVENT_REPORT: // Whenever a new report arrives from the server, see if we should execute any commands
    if (!eventArg) break; // If no report available, bail immediately
    if (GetDictVal(serverReport, "display", cmd)) {
      bool valBacklight = !strcmp(cmd, "on"); // Will return true for "on"
      if (backlight != valBacklight) {
        SetBacklight(valBacklight);
      }
    }
    break;
  } // end switch(eventId)
}

void SetBacklight(bool blOn)
{
  backlight = blOn; // Update it ready for future comparisons
  digitalWrite(TFT_BL, blOn ? HIGH : LOW);  // Set backlight pin high (=="on") or low (=="off")
  //tft.writecommand(blOn ? TFT_DISPON : TFT_DISPOFF);  // Turns TFT driver on or off
}
