/*
 * Project Wordclock
 */
#include "Wordclock.h"
#include "mySoftAP.h"

Wordclock wc = Wordclock();

void setup() {
    wc.begin();
}

void loop() {
    wc.update();
}
