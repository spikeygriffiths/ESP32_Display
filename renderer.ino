// renderer

void RenderTimeDetail(char* report)
{
  char timeText[32];
  char monthText[20], dayOfMonthText[5], dayOfWeekText[12];
  char dayText[32];
  int pix;
  
  Serial.println("Time text");
  // Parse the report as Python dict, as {<key>:<value>,...}
  GetDictVal(report, "timeText", timeText);
  GetDictVal(report, "dayOfWeekText", dayOfWeekText);
  GetDictVal(report, "dayOfMonthText", dayOfMonthText);
  GetDictVal(report, "monthText", monthText);
  strncpy(dayText, dayOfWeekText, 3); // First three characters of day of week...
  strcat(dayText, ", ");
  strcat(dayText, dayOfMonthText);
  switch (atoi(dayOfMonthText) % 10) {
    case 1: strcat(dayText, "st"); break;
    case 2: strcat(dayText, "nd"); break;
    case 3: strcat(dayText, "rd"); break;
    default: strcat(dayText, "th"); break;    
  }
  strcat(dayText, " ");
  strncat(dayText, monthText, 3); // First three characters of month
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.loadFont("Cambria-24");   // Name of font file (library adds leading / and .vlw)
  pix = tft.textWidth(timeText);
  if (pix < 0) pix = 0; // Need to split text up across two lines
  tft.setCursor((120 - pix/ 2), 20);
  tft.print(timeText);
  tft.unloadFont(); // To recover RAM 
  tft.loadFont("Cambria-36");   // Name of font file (library adds leading / and .vlw)
  pix = tft.textWidth(dayText);
  if (pix < 0) pix = 0; // Need to split text up across two lines
  tft.setCursor((120 - pix/ 2), 90);
  tft.print(dayText);
  tft.unloadFont(); // To recover RAM 
}

void RenderTimeDigits(char* report)
{
  char timeDigits[6];
  char monthText[20], dayOfMonthText[5], dayOfWeekText[12];
  
  Serial.println("Time digits");
  // Parse the report as Python dict, as {<key>:<value>,...}
  GetDictVal(report, "dayOfWeekText", dayOfWeekText);
  GetDictVal(report, "dayOfMonthText", dayOfMonthText);
  GetDictVal(report, "monthText", monthText);
  GetDictVal(report, "timeDigits", timeDigits);
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.loadFont("Cambria-Bold-72");   // Name of font file (library adds leading / and .vlw)
  //tft.setTextSize(8); // Very blocky
  tft.setCursor(20, 20);
  tft.print(timeDigits);
  tft.unloadFont(); // To recover RAM
  tft.loadFont("Cambria-24");   // Name of font file (library adds leading / and .vlw)
  tft.setCursor(10, 110);
  tft.print(dayOfWeekText);
  tft.print(", ");
  tft.print(dayOfMonthText);
  tft.print(" ");
  tft.print(monthText);
  tft.unloadFont(); // To recover RAM
}

void RenderWeatherDetail(char* report)
{
  char icon[32], jpegName[40];  // jpegName must be at least 5 chars longer than icon
  char period[6], cloudText[64];
  char windDir[6], windSpeed[6], windText[64];
  int windDegrees;
  
  Serial.println("Weather Detail");
  // Parse the report as Python dict, as {<key>:<value>,...}
  GetDictVal(report, "period", period);
  GetDictVal(report, "windDir", windDir);
  GetDictVal(report, "windSpeed", windSpeed);
  GetDictVal(report, "windText", windText);
  GetDictVal(report, "cloudText", cloudText);
  if (!strcmp(cloudText, "N/A")) return;  // Bail if no weather!
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
  fex.drawJpeg(jpegName, 0,3, nullptr);  // Draw JPEG directly to screen
  tft.loadFont("Cambria-36");   // Name of font file (library adds leading / and .vlw)
  tft.setCursor(150, 10);
  tft.print(period);
  tft.unloadFont(); // To recover RAM
  tft.loadFont("Cambria-24");  //was NotoSansBold15");   // Name of font file (library adds leading / and .vlw)
  tft.setCursor(20, 25);  // Middle of wind icon
  tft.print(windSpeed);
  tft.setCursor(10, 72);
  tft.print(windText);
  tft.setCursor(10, 94);
  tft.print(cloudText);
  tft.unloadFont(); // To recover RAM
}

void RenderWeather(char* report)
{
  char icon[32], jpegName[40];  // jpegName must be at least 5 chars longer than icon
  char period[6];
  char minTemp[5], maxTemp[5];
  
  Serial.println("Weather Report");
  // Parse the report as Python dict, as {<key>:<value>,...}
  GetDictVal(report, "period", period);
  GetDictVal(report, "icon", icon);
  GetDictVal(report, "maxTemp", maxTemp);
  GetDictVal(report, "minTemp", minTemp);
  if (!strcmp(icon, "N/A")) return;
  // Display the results
  tft.fillScreen(TFT_WHITE);
  tft.loadFont("NotoSansBold36");   // Name of font file (library adds leading / and .vlw)
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  jpegName[0] = '\0';
  strcpy(jpegName, "/");
  strcat(jpegName, icon);
  strcat(jpegName, ".jpg");
  Serial.print("Cloud Jpeg:"); Serial.println(jpegName);
  fex.drawJpeg(jpegName, 0,3, nullptr);  // Draw JPEG directly to screen
  tft.setCursor(150, 10);
  tft.print(period);
  tft.setCursor(150, 65);
  tft.print(maxTemp);
  tft.setCursor(150, 100);
  tft.print(minTemp);
  tft.unloadFont(); // To recover RAM
}
