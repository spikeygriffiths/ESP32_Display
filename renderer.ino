// renderer

static bool more;
static DISPLAYID displayId, oldDisplayId;
static bool redraw;
static bool serverAvailable;

int PrettyLine(char* text, int startY, int justify)
{
  int textW = tft.textWidth(text);
  int startX;
  switch (justify) {
  case JUSTIFY_LEFT:
    startX = TFT_MARGIN;
    break;
  case JUSTIFY_CENTRE:
    startX = (TFT_HEIGHT/2) - (textW / 2); // Screen centre is 120
    break;
  case JUSTIFY_RIGHT:
    startX = (TFT_HEIGHT-TFT_MARGIN) - textW; // Screen width is TFT_HEIGHT, but don't go right to edge
    break;
  }
  tft.setCursor(startX, startY);
  tft.print(text);
  return (tft.fontHeight() * 12) / 10;  // Return height of text line, with extra gap for readability
}

bool PrettyCheck(char** pText, int* pTextY, char** pTextEnd, char** pLastTextEnd)
{
  int startX = (TFT_HEIGHT/2) - (tft.textWidth(*pText)/ 2); // Screen width is TFT_HEIGHT
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
  Debug(face); Debug(", "); DebugLn(reason);
  tft.fillScreen(TFT_WHITE);
  fex.drawJpeg(face, (TFT_HEIGHT/2) - 48,3, nullptr);  // Draw JPEG directly to screen (JPEG is 96x96, hence 48 for middle)
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.loadFont("Cambria-24");   // Name of font file (library adds leading / and .vlw)
  PrettyLine(reason, 100, JUSTIFY_CENTRE);
  tft.unloadFont(); // To recover RAM
  oldDisplayId = DISPLAYID_FACE; // Will lose user-selected displayId if we change that
}

void RenderSadFace(char* reason)
{
  RenderFace("/SadFace.jpg", reason);
}

void RenderHappyFace(char* reason)
{
  RenderFace("/HappyFace.jpg", reason);
}

void RendererEventHandler(EVENT eventId, long eventArg)
{
  switch (eventId) {
  case EVENT_INIT:
    tft.init();
    tft.setSwapBytes(true);
    tft.setRotation(1);
    tft.fillScreen(TFT_WHITE);  // Blank screen at power on
    break;
  case EVENT_POSTINIT:
    oldDisplayId = DISPLAYID_NULL;
    displayId = DISPLAYID_TIME; // Default at power on
    serverAvailable = false;
    more = false;
    break;
  case EVENT_SOCKET:
    redraw = true;
    if (SCKSTATE_DISCONNECTING == eventArg) {
      serverAvailable = false;
      RenderSadFace("Network down!");
    }
    break;
  case EVENT_BUTTON:
    redraw = true;
    switch (eventArg) {
    case BTN_FUNC_TAP:
      DebugLn("Func button tap");
      if (displayId == DISPLAYID_WEATHER) displayId = DISPLAYID_TIME;
      else if (displayId == DISPLAYID_TIME) displayId = DISPLAYID_WEATHER;
      else displayId = DISPLAYID_TIME; // Default
      more = false; // Default
      break;
    case BTN_FUNC_LONG: // Taps often also look like long presses
      DebugLn("Func button long");
      //displayId = DISPLAYID_TIME; // Default
      //more = false; // Default
      break;
    //case BTN_FUNC_DOUBLE: break;
    case BTN_CUSTOM_TAP:
      DebugLn("Custom button tap");
      more ^= true;
      break;
    case BTN_CUSTOM_LONG: // Taps often also look like long presses
      DebugLn("Custom button long");
    //  more = false; // Default
      break;
    //case BTN_CUSTOM_DOUBLE: break;
    }
    break;
  case EVENT_REPORT:
    redraw = true;
    serverAvailable = (bool)eventArg;
    if (!eventArg) {
      RenderSadFace("Server down");
    }
    break;
  case EVENT_TICK:
    if ((redraw) & (serverAvailable)) {
      redraw = false;
      switch (displayId) {
      case DISPLAYID_WEATHER:
        DisplayWeather(serverReport, more, (displayId != oldDisplayId));
        break;
      case DISPLAYID_TIME:
        DisplayDateTime(serverReport, more, (displayId != oldDisplayId));
        break;
      } // end switch()
      oldDisplayId = displayId;
    }
    break;
  } // end switch (eventId)
}
