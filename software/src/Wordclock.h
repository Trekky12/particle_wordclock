#include "application.h"

#ifndef Wordclock_h
#define Wordclock_h

SYSTEM_THREAD(ENABLED);

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

#define WORDCLOCK_VERSION 20200530

//----------------- LED AND LDR Handling ------------------------
#define PIXEL_PIN A5
#define PIXEL_COUNT 97
#define PIXEL_TYPE WS2812B
#define LDR_PIN A1

// after CONNECTION_TIMEOUT ms the listening mode is started (5 min)
#define CONNECTION_TIMEOUT 5*60*1000

//sync every 4 hours
#define SYNC_INTERVAL 4*60*60

// reset Device
#define RESET_DELAY 5 * 1000

//------------- Wordclock --------------

class Wordclock {

 public:

  Wordclock();

  void
    begin(void),
    update(void),
    setWiFiOnTime(uint8_t hour, uint8_t minute, uint8_t second),
    setWiFiOffTime(uint8_t hour, uint8_t minute, uint8_t second),
    setWiFiState(bool),
    setAutoBrightness(bool),
    setBrightness(uint8_t),
    blinkLED(uint8_t);
  bool
    isAutoBrightnessOn(void),
    isWiFiOn(void);
  int
    disableWiFi(String cmd),
    getColor(String cmd),
    setTimeZone(String cmd),
    setClockLight(String cmd),
    controlColor(String cmd),
    listen(String cmd),
    getVersion(String cmd),
    getBrightness(String cmd),
    reset(String cmd);


  private:
    uint32_t
        lastSync = 0,
        hours_buffer = 0,
        minutes_buffer = 0,
        color,
        version = WORDCLOCK_VERSION;
    uint8_t brightness = 255;
    unsigned long startConnection = millis();
    unsigned long lastResetTriggered = 0;
    void
        display_time(int hours, int minutes),
        adjustBrightness(void),
        adjustTime(void),
        allLedsOff(void),
        handleLocalServer(void);
    bool
        wlanOff = false,
        disableWiFiNow = false,
        autoBrightness = true,
        hasCredentials = false, // lets asume we don't have the credentials
        serverStarted = false,
        networkReady(void);
    uint8_t
        wlanOffTimeH = 255, // bigger than 24 means not set
        wlanOffTimeM = 0,
        wlanOffTimeS = 0,
        wlanOnTimeH = 255,  // bigger than 24 means not set
        wlanOnTimeM = 0,
        wlanOnTimeS = 0,
        splitColor(char value);

    // https://community.particle.io/t/solved-retained-and-variable-of-class/20980/5
    // https://stackoverflow.com/a/5019896
    // http://www.cplusplus.com/forum/beginner/121441/
    static bool showClock;
};

#endif // Wordclock_h
