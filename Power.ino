// Power

char powerNow[10], energyToday[10];
int oldPowerDetail;

void RenderPower(char* report, bool reportChange)
{
  char powerTxt[10];
  char jpegName[40];  // jpegName must be at least 5 chars longer than icon
  int powerVal;
  int suffixWidth;

  reportChange |= CmpDictVal(report, "powerNow", powerNow); // Check to see if any of the values we care about have changed since last time
  reportChange |= CmpDictVal(report, "energyToday", energyToday);
  if (!reportChange) return;  // Exit if nothing changed
  // Parse the report as Python dict, as {<key>:<value>,...}
  if (!GetDictVal(report, "powerNow", powerNow)) return;
  if (!GetDictVal(report, "energyToday", energyToday)) return;
  if (strlen(powerNow) > 3) { // Convert to kW if > 1000W
    strcpy(powerTxt, powerNow);
    strcpy(powerTxt+1, ".");  // Assume just single digit of kW
    strcpy(powerTxt+2, powerNow+1);
    powerTxt[4] = 0;  // Lose units
    suffixWidth = 15; // Allow for small "kW" after digits
  } else { // 3 or fewer digits - show in W
    strcpy(powerTxt, powerNow);
    suffixWidth = 10; // Allow for small "kW" after digits
  }
  powerVal = atoi(powerNow);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  jpegName[0] = '\0';
  strcpy(jpegName, "/Meter_");
  if (powerVal < 500) {
    strcat(jpegName, "33");
  } else if (powerVal < 1000) {
    strcat(jpegName, "67");
  } else {
    strcat(jpegName, "100");
  }
  strcat(jpegName, ".jpg");
  Debug("Power Jpeg:"); DebugLn(jpegName);
  fex.drawJpeg(jpegName, 15,0, nullptr);  // Draw JPEG directly to screen
  tft.loadFont("Cambria-48");   // Name of font file (library adds leading / and .vlw)
  //PrettyLine(powerTxt, 50, JUSTIFY_CENTRE);
  int startX = (TFT_HEIGHT/2) - (tft.textWidth(powerTxt) / 2) - suffixWidth;  // Must calculate this while using correct font.  -suffixWidth to allow for "W" or "kW"
  int endX = (TFT_HEIGHT/2) + (tft.textWidth(powerTxt) / 2) - suffixWidth;  // Must calculate this while using correct font
  tft.setCursor(startX, 50);
  tft.print(powerTxt);
  tft.unloadFont(); // To recover RAM
  tft.loadFont("Cambria-24");   // Name of font file (library adds leading / and .vlw)
  tft.setCursor(endX+2, 50+24-6);
  if (strlen(powerNow) > 3) { // Convert to kW if > 1000W
    tft.print("kW"); // Lose units digit of power (otherwise string gets too long!)
  } else { // 3 or fewer digits - show in W
    tft.print("W");
  }
  tft.unloadFont(); // To recover RAM
}

void DisplayPower(char* report, int powerDetail, bool forceRedraw)
{
  bool reportChange = (powerDetail != oldPowerDetail) | forceRedraw;
  oldPowerDetail = powerDetail;
  //if (powerDetail) {
    RenderPower(report, reportChange);
}
