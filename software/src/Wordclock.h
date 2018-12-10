#include "application.h"

#ifndef Wordclock_h
#define Wordclock_h

SYSTEM_THREAD(ENABLED);



#define WORDCLOCK_VERSION 20180917

//----------------- LED AND LDR Handling ------------------------
#define PIXEL_PIN A5
#define PIXEL_COUNT 97
#define PIXEL_TYPE WS2812B
#define LDR_PIN A1

// after CONNECTION_TIMEOUT ms the listening mode is started
#define CONNECTION_TIMEOUT 20000

//sync every 4 hours
#define SYNC_INTERVAL 4*60*60

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
    setClockStatus(String cmd),
    controlColor(String cmd),
    listen(String cmd),
    getVersion(String cmd);


  private:
    uint32_t
        lastSync = 0,
        hours_buffer = 0,
        minutes_buffer = 0,
        color,
        version = WORDCLOCK_VERSION;
    unsigned long startConnection = millis();
    void
        display_time(int hours, int minutes),
        adjustBrightness(void),
        adjustTime(void),
        allLedsOff(void);
    bool
        showClock = true,
        clockState = true,
        wlanOff = false,
        disableWiFiNow = false,
        autoBrightness = true,
        hasCredentials = false; // lets asume we don't have the credentials
    uint8_t
        wlanOffTimeH = 255, // bigger than 24 means not set
        wlanOffTimeM = 0,
        wlanOffTimeS = 0,
        wlanOnTimeH = 255,  // bigger than 24 means not set
        wlanOnTimeM = 0,
        wlanOnTimeS = 0,
        splitColor(char value);

};

#endif // Wordclock_h
