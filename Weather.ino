// Weather

char period[6], cloudText[64];
char icon[32];
char minTemp[5], maxTemp[5];
char windDir[6], windSpeed[6], windText[64];

void RenderWeatherDetail(char* report, bool reportChange)
{
  char jpegName[40];  // jpegName must be at least 5 chars longer than icon
  int windDegrees, windIconX, windIconY;

  reportChange |= CmpDictVal(report, "period", period); // Check to see if any of the values we care about have changed since last time
  reportChange |= CmpDictVal(report, "windDir", windDir);
  reportChange |= CmpDictVal(report, "windSpeed", windSpeed);
  reportChange |= CmpDictVal(report, "windText", windText);
  reportChange |= CmpDictVal(report, "cloudText", cloudText);
  if (!reportChange) return;  // Exit if nothing changed
  Serial.println("Weather Detail");
  // Parse the report as Python dict, as {<key>:<value>,...}
  if (!GetDictVal(report, "period", period)) return;
  if (!GetDictVal(report, "windDir", windDir)) return;
  if (!GetDictVal(report, "windSpeed", windSpeed)) return;
  if (!GetDictVal(report, "windText", windText)) return;
  if (!GetDictVal(report, "cloudText", cloudText)) return;
  // Convert windDir into icon name for wind arrows
  windDegrees = ((atoi(windDir)+22) / 45) % 8; // Get direction to nearest 45'
  switch (windDegrees) {
    case 0: strcpy(icon, "North"); break;
    case 1: strcpy(icon, "NorthEast"); break;
    case 2: strcpy(icon, "East"); break;
    case 3: strcpy(icon, "SouthEast"); break;
    case 4: strcpy(icon, "South"); break;
    case 5: strcpy(icon, "SouthWest"); break;
    case 6: strcpy(icon, "West"); break;
    case 7: strcpy(icon, "NorthWest"); break;
  }
  Serial.println(icon);
  // Display the results
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  jpegName[0] = '\0';
  strcpy(jpegName, "/");
  strcat(jpegName, icon);
  strcat(jpegName, ".jpg");
  Serial.print("Wind Jpeg:"); Serial.println(jpegName);
  tft.loadFont("Cambria-36");   // Name of font file (library adds leading / and .vlw)
  tft.setCursor(120, 20);
  tft.print(period);
  tft.unloadFont(); // To recover RAM
  tft.loadFont("Cambria-24");  //was NotoSansBold15");   // Name of font file (library adds leading / and .vlw)
  windIconX = 33;
  windIconY = 3;
  fex.drawJpeg(jpegName, windIconX,windIconY, nullptr);  // Draw JPEG directly to screen
  tft.setCursor(windIconX+19, windIconY+22);  // Middle of wind icon
  tft.print(windSpeed);
  PrettyLine(windText, 76);
  PrettyLine(cloudText, 98);
  tft.unloadFont(); // To recover RAM
}

void RenderWeather(char* report, bool reportChange)
{
  char jpegName[40];  // jpegName must be at least 5 chars longer than icon
  
  reportChange |= CmpDictVal(report, "period", period); // Check to see if any of the values we care about have changed since last time
  reportChange |= CmpDictVal(report, "icon", icon);
  reportChange |= CmpDictVal(report, "maxTemp", maxTemp);
  reportChange |= CmpDictVal(report, "minTemp", minTemp);
  if (!reportChange) return;  // Exit if nothing changed
  Serial.println("Weather Report");
  if (!GetDictVal(report, "period", period)) return;
  if (!GetDictVal(report, "icon", icon)) return;
  if (!GetDictVal(report, "maxTemp", maxTemp)) return;
  if (!GetDictVal(report, "minTemp", minTemp)) return;
  // Display the results
  tft.fillScreen(TFT_WHITE);
  tft.loadFont("Cambria-36");   // Name of font file (library adds leading / and .vlw)
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  jpegName[0] = '\0';
  strcpy(jpegName, "/");
  strcat(jpegName, icon);
  strcat(jpegName, ".jpg");
  Serial.print("Cloud Jpeg:"); Serial.println(jpegName);
  fex.drawJpeg(jpegName, 3,3, nullptr);  // Draw JPEG directly to screen
  tft.setCursor(150, 10);
  tft.print(period);
  tft.setCursor(150, 65);
  tft.print(maxTemp);
  tft.setCursor(150, 100);
  tft.print(minTemp);
  tft.unloadFont(); // To recover RAM
}

void DisplayWeather(char* report, int customBtn, bool forceRedraw)
{
  bool reportChange = (customBtn != oldCustomBtn) | forceRedraw;
  oldCustomBtn = customBtn;
  if (customBtn) {
    RenderWeatherDetail(report, reportChange);
  } else {  // less
    RenderWeather(report, reportChange);
  }
}