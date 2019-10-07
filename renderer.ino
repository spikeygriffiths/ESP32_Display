// renderer

int PrettyLine(char* text, int startY)
{
  int startX = 120 - (tft.textWidth(text)/ 2); // Screen centre is 120
  tft.setCursor(startX, startY);  // Screen centre is 120
  tft.print(text);
}

void PrettyPrint(char* textStart, int startY, char* font)
{
  char* textEnd = textStart;  // Step along string looking for spaces and seeing if line will fit
  char* lastTextEnd = textEnd;  // To keep track of line that did fit
  int startX;
  tft.loadFont(font);   // Name of font file (library adds leading / and .vlw)
  while (*textEnd) {  // Keep scanning text until '\0' terminator
    if (' ' == *textEnd) { // Found space, so check if text will fit
      *textEnd = '\0';  // Temporarily terminate string to see if string so far will fit on display
      startX = 120 - (tft.textWidth(textStart)/ 2); // Screen centre is 120
      *textEnd = ' ';  // Restore the space
      if (startX > 10) {  // If the line will nicely fit on the display
        lastTextEnd = textEnd;  // and make a note of where it was so we can go back there
      } else {  // Line is too long, so go back to last good line and print that
        *lastTextEnd = '\0';  // Terminate the earlier string that did fit
        PrettyLine(textStart, startY);
        startY += 30; // Font Depth - should be calculated or passed in
        textStart = lastTextEnd+1;  // Point just beyond the old space (now a terminator), 
        textEnd = textStart-1;  // Rely on pointer incrementing at end of loop
        lastTextEnd = textEnd;
      }
    }
    textEnd++;  // On to next character in line...
  }
  if (textEnd != textStart) {
    PrettyLine(textStart, startY);  // Print remainder of line
  }
  tft.unloadFont(); // To recover RAM 
}

bool GetDayText(char* report, char* dayText)
{
  char monthText[20], dayOfMonthText[5], dayOfWeekText[12];
  
  if (!GetDictVal(report, "dayOfWeekText", dayOfWeekText)) return false;
  if (!GetDictVal(report, "dayOfMonthText", dayOfMonthText)) return false;
  if (!GetDictVal(report, "monthText", monthText)) return false;
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
  return true;  // Good dayText
}

void RenderTimeDetail(char* report)
{
  char timeText[32];
  char dayText[32];
  
  Serial.println("Time text");
  // Parse the report as Python dict, as {<key>:<value>,...}
  if (!GetDictVal(report, "timeText", timeText)) return;
  if (!GetDayText(report, dayText)) return;
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  PrettyPrint(timeText, 20, "Cambria-24");
  tft.loadFont("Cambria-36");   // Name of font file (library adds leading / and .vlw)
  PrettyLine(dayText, 90);
  tft.unloadFont(); // To recover RAM 
}

void RenderTimeDigits(char* report)
{
  char timeDigits[6];
  char dayText[32];
  
  Serial.println("Time digits");
  // Parse the report as Python dict, as {<key>:<value>,...}
  if (!GetDictVal(report, "timeDigits", timeDigits)) return;
  if (!GetDayText(report, dayText)) return;
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.loadFont("Cambria-Bold-72");   // Name of font file (library adds leading / and .vlw)
  PrettyLine(timeDigits, 20);
  tft.unloadFont(); // To recover RAM
  tft.loadFont("Cambria-36");   // Name of font file (library adds leading / and .vlw)
  PrettyLine(dayText, 90);
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
  fex.drawJpeg(jpegName, 0,3, nullptr);  // Draw JPEG directly to screen
  tft.setCursor(150, 10);
  tft.print(period);
  tft.setCursor(150, 65);
  tft.print(maxTemp);
  tft.setCursor(150, 100);
  tft.print(minTemp);
  tft.unloadFont(); // To recover RAM
}

void RenderFace(char* face, char* reason)
{
  Serial.print("SadFace, "); Serial.println(reason);
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  fex.drawJpeg("/SadFace.jpg", 120 - 48,3, nullptr);  // Draw JPEG directly to screen
  tft.loadFont("Cambria-24");   // Name of font file (library adds leading / and .vlw)
  PrettyLine(reason, 100);
  tft.unloadFont(); // To recover RAM
}

void RenderSadFace(char* reason)
{
  RenderFace("/SadFace.jpg", reason);
}

void RenderHappyFace(char* reason)
{
  RenderFace("/HappyFace.jpg", reason);
}
