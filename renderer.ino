// renderer

int PrettyLine(char* text, int startY, int justify)
{
  int textW = tft.textWidth(text);
  int startX;
  switch (justify) {
  case JUSTIFY_LEFT:
    startX = 10;
    break;
  case JUSTIFY_CENTRE:
    startX = 120 - (textW / 2); // Screen centre is 120
    break;
  case JUSTIFY_RIGHT:
    startX = 230 - textW; // Screen width is 240, but don't go right to edge
    break;
  }
  tft.setCursor(startX, startY);
  tft.print(text);
  return (tft.fontHeight() * 12) / 10;  // Return height of text line, with extra gap for readability
}

bool PrettyCheck(char** pText, int* pTextY, char** pTextEnd, char** pLastTextEnd)
{
  int startX = 120 - (tft.textWidth(*pText)/ 2); // Screen centre is 120
  if (startX > 10) {  // If the line will nicely fit on the display
    *pLastTextEnd = *pTextEnd;  // and make a note of where it was so we can go back there
    **pTextEnd = ' ';  // Restore the space
    return false; // We didn't print
  } else {  // Line is too long, so go back to last good line and print that
    **pLastTextEnd = '\0';  // Terminate the earlier string that did fit
    *pTextY += PrettyLine(*pText, *pTextY, JUSTIFY_CENTRE);
    *pText = *pLastTextEnd+1;  // Point just beyond the old space (now a terminator), 
    *pTextEnd = *pText-1;
    *pLastTextEnd = *pTextEnd;  // Update last end to be
    return true;  // We did print
  }
}

void PrettyPrint(char* textStart, int textY, char* font) // May modify the text
{
  char* textEnd = textStart;  // Step along string looking for spaces and seeing if line will fit
  char* lastTextEnd = textEnd;  // To keep track of line that did fit
  tft.loadFont(font);   // Name of font file (library adds leading / and .vlw)
  while (*textEnd) {  // Keep scanning text until '\0' terminator
    if (' ' == *textEnd) { // Found space, so check if text will fit
      *textEnd = '\0';  // Temporarily terminate string to see if string so far will fit on display
      if (!PrettyCheck(&textStart, &textY, &textEnd, &lastTextEnd)) {  // True if it printed the string
      } else {  // We did print
      }
    }
    textEnd++;  // On to next character in line...
  }
  if (textEnd != textStart) {
    if (!PrettyCheck(&textStart, &textY, &textEnd, &lastTextEnd)) {  // True if it printed the string
      PrettyCheck(&textStart, &textY, &textEnd, &lastTextEnd);  // Print final fragment
    }
  }
  tft.unloadFont(); // To recover RAM 
}

void RenderFace(char* face, char* reason)
{
  Debug(face); Debug(", "); Debug(reason); DebugLn();
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  fex.drawJpeg("/SadFace.jpg", 120 - 48,3, nullptr);  // Draw JPEG directly to screen
  tft.loadFont("Cambria-24");   // Name of font file (library adds leading / and .vlw)
  PrettyLine(reason, 100, JUSTIFY_CENTRE);
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
