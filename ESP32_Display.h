// ST7789 135 x 240 display with no chip select line

#define ST7789_DRIVER     // Configure all registers

#define TFT_WIDTH  135
#define TFT_HEIGHT 240

#define CGRAM_OFFSET      // Library will add offsets required

#define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue [JPG] Guessed wrong - was commented out
//#define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red [JPG] was commented out

//#define TFT_INVERSION_ON
//#define TFT_INVERSION_OFF

// DSTIKE stepup
//#define TFT_DC    23
//#define TFT_RST   32
//#define TFT_MOSI  26
//#define TFT_SCLK  27


#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define ADC_EN          14
#define ADC_PIN         34
#define BUTTON_1        35
#define BUTTON_2        0

// Generic ESP32 setup
#define TFT_MISO -1
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS    5 // Not connected
#define TFT_DC    16
#define TFT_RST   23  // Connect reset to ensure display initialises
#define TFT_BL     4  // Display backlight control pin
#define TFT_BACKLIGHT_ON HIGH


//#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
//#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
//#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
//#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
//#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
//#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
//#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT


// #define SPI_FREQUENCY  27000000
#define SPI_FREQUENCY  40000000

#define SPI_READ_FREQUENCY  20000000

#define SPI_TOUCH_FREQUENCY  2500000

// #define SUPPORT_TRANSACTIONS

typedef enum {
  EVENT_INIT, // No arg
  EVENT_POSTINIT, // No arg
  EVENT_TICK, // Arg is ms since last EVENT_TICK
  EVENT_SEC,  // Arg is secs since last EVENT_SEC
  EVENT_SOCKET,  // Arg is SCKSTATE
  EVENT_BUTTON, // Arg is BUTTON
  EVENT_REPORT, // Arg is pointer to new report
  EVENT_DISPLAY, // Arg is DISPLAY_xxx
} EVENT;

typedef enum {
  BTN_FUNC_TAP,
  BTN_FUNC_LONG,
  BTN_FUNC_DOUBLE,
  BTN_CUSTOM_TAP,
  BTN_CUSTOM_LONG,
  BTN_CUSTOM_DOUBLE,
} BUTTON;

typedef enum {
  DISPLAYID_NULL, // Used to force a display redraw after server comes back up
  DISPLAYID_FACE, // For happy or sad faces
  DISPLAYID_WEATHER,
  DISPLAYID_TIME,
  DISPLAYID_POWER,
  DISPLAYID_HEATING,
} DISPLAYID;

typedef enum {
  SCKSTATE_GETCREDS,  // Read ssid and pass from file
  SCKSTATE_JOINING, // Keep trying to join network
  SCKSTATE_DISCONNECTING, // In case we can't join after a number of tries.  Disconnects, then restarts JOINING
  SCKSTATE_CONNECTING,  // Finding socket on server
  SCKSTATE_CONNECTED, // All ready to start getting reports from server
  SCKSTATE_RECONNECTING,  // Re-connecting to socket on server after getting a report
} SCKSTATE;

#define WIFI_JOINING_TIMEOUTS 10
#define WIFI_CONNECTION_TIMEOUTS 10

// For PrettyLine routine
#define JUSTIFY_LEFT -1
#define JUSTIFY_CENTRE 0
#define JUSTIFY_RIGHT 1
#define TFT_MARGIN 10 // To stop text starting or ending right at the edge

#define MAX_REPORT 1024 // Arbitrary maximum length of serverReport
#define REPORT_TIMEOUTS 10

#define OSIssueEvent(evId, evArg) _OSIssueEvent(evId, (long)evArg)

// Global function prototypes
void _OSIssueEvent(EVENT eventId, long eventArg);
void OSEventHandler(EVENT eventId, long eventArg);
void WiFiEventHandler(EVENT eventId, long eventArg);
void ServerCmdEventHandler(EVENT eventId, long eventArg);
void RendererEventHandler(EVENT eventId, long eventArg);

// Parser
bool GetDictVal(char* dict, char* item, char* val);
bool CmpDictVal(char* dict, char* item, char* oldVal); // Returns false if the oldVal is the same as the new value from the dict

// Renderer
int PrettyLine(char* text, int startY, int justify);
void PrettyPrint(char* textStart, int textY, char* font); // May modify the text
void RenderFace(char* face, char* reason);
