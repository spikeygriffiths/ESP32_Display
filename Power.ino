// Power

char powerNow[10], energyToday[10];
int oldPowerDetail;

void RenderPower(char* report, bool reportChange)
{
  char powerTxt[10];
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
    strcpy(powerTxt+4, "kW"); // Lose units digit of power (otherwise string gets too long!)
  } else { // 3 or fewer digits - show in W
    strcpy(powerTxt, powerNow);
    strcat(powerTxt, "W");
  }
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.loadFont("Cambria-48");   // Name of font file (library adds leading / and .vlw)
  PrettyLine(powerTxt, 20, JUSTIFY_CENTRE);
  tft.unloadFont(); // To recover RAM
}

void DisplayPower(char* report, int powerDetail, bool forceRedraw)
{
  bool reportChange = (powerDetail != oldPowerDetail) | forceRedraw;
  oldPowerDetail = powerDetail;
  //if (powerDetail) {
    RenderPower(report, reportChange);
}
