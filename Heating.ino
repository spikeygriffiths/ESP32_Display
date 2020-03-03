// Heating

char houseTemp[10], targetTemp[10];

void RenderHeating(char* report, bool reportChange)
{
  char tempTxt[16];
  reportChange |= CmpDictVal(report, "houseTemp", houseTemp); // Check to see if any of the values we care about have changed since last time
  reportChange |= CmpDictVal(report, "TargetTemp", targetTemp);
  if (!reportChange) return;  // Exit if nothing changed
  // Parse the report as Python dict, as {<key>:<value>,...}
  if (!GetDictVal(report, "houseTemp", houseTemp)) return;
  if (!GetDictVal(report, "TargetTemp", targetTemp)) return;
  strcpy(tempTxt, "House:");
  strcat(tempTxt, houseTemp);
  strcat(tempTxt, "C");
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.loadFont("Cambria-36");   // Name of font file (library adds leading / and .vlw)
  PrettyLine(tempTxt, 20, JUSTIFY_CENTRE);
  strcpy(tempTxt, "Target:");
  strcat(tempTxt, targetTemp);
  strcat(tempTxt, "C");
  PrettyLine(tempTxt, 60, JUSTIFY_CENTRE);
  tft.unloadFont(); // To recover RAM
}

void DisplayHeating(char* report, int powerDetail, bool forceRedraw)
{
  bool reportChange = forceRedraw;
  RenderHeating(report, reportChange);
}
