#include "application.h"

#ifndef Wordclock_h
#define Wordclock_h

SYSTEM_THREAD(ENABLED);

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
    controlColor(String cmd);


  private:
    uint32_t color;
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
        autoBrightness = true,
        hasCredentials = false; // lets asume we don't have the credentials
    uint8_t
        hours_buffer,
        minutes_buffer,
        lastSync = 0,
        wlanOffTimeH = 255, // bigger than 24 means not set
        wlanOffTimeM = 0,
        wlanOffTimeS = 0,
        wlanOnTimeH = 255,  // bigger than 24 means not set
        wlanOnTimeM = 0,
        wlanOnTimeS = 0,
        splitColor(char value);

};


//----------------- LED AND LDR Handling ------------------------
#define PIXEL_PIN A5
#define PIXEL_COUNT 97
#define PIXEL_TYPE WS2812B
#define LDR_PIN A1

#define CONNECTION_TIMEOUT 20000


#endif // Wordclock_h
