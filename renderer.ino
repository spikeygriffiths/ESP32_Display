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
