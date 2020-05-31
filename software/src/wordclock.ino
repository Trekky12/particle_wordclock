/*
 * Project Wordclock
 */
#include "Wordclock.h"
#if Wiring_WiFi
    #include "mySoftAP.h"
#endif

Wordclock wc = Wordclock();

void setup() {
    wc.begin();
}

void loop() {
    wc.update();
}
