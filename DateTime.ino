// DateTime

char monthText[20], dayOfMonthText[5], dayOfWeekText[12];
char timeText[32];
char timeDigits[6];
char dayText[32];
int oldDateTimeDetail;

bool GetDayText(char* report, char* dayText)
{  
  if (!GetDictVal(report, "dayOfWeekText", dayOfWeekText)) return false;
  if (!GetDictVal(report, "dayOfMonthText", dayOfMonthText)) return false;
  if (!GetDictVal(report, "monthText", monthText)) return false;
  strncpy(dayText, dayOfWeekText, 3); // First three characters of day of week...
  strcpy(dayText+3, ", ");
  strcat(dayText, dayOfMonthText);
  switch (atoi(dayOfMonthText)) {
    case 1:
    case 21:
    case 31:
      strcat(dayText, "st"); break;
    case 2:
    case 22:
      strcat(dayText, "nd"); break;
    case 3:
    case 23:
      strcat(dayText, "rd"); break;
    default:
      strcat(dayText, "th"); break;    
  }
  strcat(dayText, " ");
  strncat(dayText, monthText, 3); // First three characters of month
  return true;  // Good dayText
}

#define NUM_TIMELINES 3
#define MAX_TIMELINELEN 20

void RenderTimeDetail(char* report, bool reportChange)
{
  char lines[NUM_TIMELINES][MAX_TIMELINELEN];
  int index, endLine, lineIndex, startY, justify;
  reportChange |= CmpDictVal(report, "timeText", timeText); // Check to see if any of the values we care about have changed since last time
  if (!reportChange) return;  // Exit if nothing changed
  DebugLn("Time text");
  if (!GetDictVal(report, "timeText", timeText)) return;
  for (lineIndex = 0; lineIndex < NUM_TIMELINES; lineIndex++) {
    memset(lines[lineIndex], 0, MAX_TIMELINELEN); // Fill all lines with zeros so that any strings copied in will be auto-terminated, and so we can see how many lines are made up at the end
  }
  index = strlen(timeText); // Go backwards through timeText, looking for spaces and copy fragments into line1,2,3
  endLine = index;
  lineIndex = NUM_TIMELINES-1;  // Start at last line
  while ((--index) && (lineIndex)) {
    if (timeText[index] == ' ') {
      memcpy(lines[lineIndex--], &timeText[index+1], endLine - index);
      endLine = index-1;  // Note where new end of line will be, ignoring space
    }
  }
  memcpy(lines[lineIndex], &timeText, endLine+1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  startY = (lines[0][0]) ? 15 : 35; // Start lower down if first line is empty (eg for "One o'clock")
  tft.loadFont("Cambria-36");   // Name of font file (library adds leading / and .vlw)
  justify = (lines[0][0]) ? JUSTIFY_LEFT : JUSTIFY_CENTRE;
  for (lineIndex = 0; lineIndex < NUM_TIMELINES; lineIndex++) {
    if (lines[lineIndex][0]) {  // Don't print empty lines
      startY += PrettyLine(lines[lineIndex], startY, justify++);  // Rely on justification going from left, to centre to right
    }
  }
  tft.unloadFont(); // To recover RAM 
}

void RenderTimeDigits(char* report, bool reportChange)
{
  reportChange |= CmpDictVal(report, "timeDigits", timeDigits); // Check to see if any of the values we care about have changed since last time
  if (!reportChange) return;  // Exit if nothing changed
  DebugLn("Time update");
  // Parse the report as Python dict, as {<key>:<value>,...}
  if (!GetDictVal(report, "timeDigits", timeDigits)) return;
  if (!GetDayText(report, dayText)) return;
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.loadFont("Cambria-Bold-72");   // Name of font file (library adds leading / and .vlw)
  PrettyLine(timeDigits, 20, JUSTIFY_CENTRE);
  tft.unloadFont(); // To recover RAM
  tft.loadFont("Cambria-36");   // Name of font file (library adds leading / and .vlw)
  PrettyLine(dayText, 90, JUSTIFY_CENTRE);
  tft.unloadFont(); // To recover RAM
}

void DisplayDateTime(char* report, int dateTimeDetail, bool forceRedraw)
{
  bool reportChange = (dateTimeDetail != oldDateTimeDetail) | forceRedraw;
  oldDateTimeDetail = dateTimeDetail;
  if (dateTimeDetail) {
    RenderTimeDetail(report, reportChange);
  } else { // less
    RenderTimeDigits(report, reportChange);
  }
}
