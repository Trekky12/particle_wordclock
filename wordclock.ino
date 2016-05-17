// This #include statement was automatically added by the Particle IDE.
#include "neopixel/neopixel.h"

#define PIXEL_PIN A5
#define PIXEL_COUNT 97
#define PIXEL_TYPE WS2812B
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

#define photoresistorPin A1


// Variables

// Time Sync
int lastSync = 0;
const int syncInterval = 60*60*4; //sync every  4 hours

// temporary variables
int hours_buffer;
int minutes_buffer;

// make mean of ldrr values
const int LDR_Messungen_Anzahl = 10;
int LDR_Messungen[LDR_Messungen_Anzahl];
int brightness = 0;

// LED control
uint32_t color = strip.Color(255, 0, 0);
bool showClock = true;

// This function gets called whenever there is a matching API request
// the command string format is
// <LED_Red>,<LED_Green>,<LED_Blue>
// for example: 000,255,000
int controlColor(String command) {

    int red = 0;
    int blue = 0;
    int green = 0;

    char * params = new char[command.length() + 1];

    strcpy(params, command.c_str());
    char * param1 = strtok(params, ",");
    char * param2 = strtok(NULL, ",");
    char * param3 = strtok(NULL, ",");


    if (param1 != NULL && param2 != NULL && param3 != NULL) {

        red = atoi(param1);
        green = atoi(param2);
        blue = atoi(param3);
        if (red < 0 || red > 255) return -1;
        if (green < 0 || green > 255) return -1;
        if (blue < 0 || blue > 255) return -1;

        color = strip.Color(red,green,blue);
        EEPROM.put(1, color);
        display_time(hours_buffer, minutes_buffer);
        return 0;
    }
    return -1;
}

// This function gets called whenever there is a matching API request
// the command string format is the number o the current timezone
// for example: 2
int setTimeZone(String zone){
  int timezone = zone.toInt();
  Time.zone(timezone);
  return 0;
}

// This function gets called whenever there is a matching API request
// the command string format is the status of the leds ("on"/"off")
int setClockStatus(String status){
  if (status == "off"){
    showClock = false;
  }else{
    showClock = true;
    display_time(hours_buffer, minutes_buffer);
  }
  return 0;
}

void adjustTime(){
    if(lastSync + syncInterval < Time.now()) {
        Spark.syncTime();
        lastSync = Time.now();
    }
}


void adjustBrightness() {

    // LDR_Messungen-Puffer Inhalte weiterruecken
    for (int i = LDR_Messungen_Anzahl-1; i > 0; i = i - 1) {
     LDR_Messungen[i] = LDR_Messungen[i - 1];
    }
    LDR_Messungen[0] = analogRead(photoresistorPin);   // meanLDR an erste Stelle des LDR_Messungen schreiben
    brightness = 0;                           // gemittelten meanLDR auf 0 setzen

    // LDR_Messungen-Puffer Inhalte addieren
    for (int i = 0; i < LDR_Messungen_Anzahl; i = i + 1) {
      brightness = brightness + LDR_Messungen[i];
    }
    brightness = brightness / LDR_Messungen_Anzahl; // gemittelten meanLDR errechnen

    //brightness = map(analogRead(photoresistorPin), 0, 4095, 0, 100);
    //brightness = map(meanLDR, 0, 4095, 0, 100);

    int strip_brightness = 255;
    if (brightness < 30){
        strip_brightness = 10;
    }else if (brightness < 100){
        strip_brightness = 20;
    }else if (brightness < 400){
        strip_brightness = 100;
    }else if (brightness < 1000){
        strip_brightness = 100;
    }else if (brightness < 2000){
        strip_brightness = 150;
    }else{
      strip_brightness = 255;
    }
    strip.setBrightness(strip_brightness);
    strip.show();
}

void display_time(int hours, int minutes){

    //alles resetten
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }

    //ES
    strip.setPixelColor(95, color);
    strip.setPixelColor(96, color);

    //IST
    strip.setPixelColor(92, color);
    strip.setPixelColor(93, color);
    strip.setPixelColor(94, color);

    // <5 Minutes
    switch(minutes%5){
        case 1:
            strip.setPixelColor(3, color);
            break;
        case 2:
            strip.setPixelColor(3, color);
            strip.setPixelColor(2, color);
            break;
        case 3:
            strip.setPixelColor(3, color);
            strip.setPixelColor(2, color);
            strip.setPixelColor(1, color);
            break;
        case 4:
            strip.setPixelColor(3, color);
            strip.setPixelColor(2, color);
            strip.setPixelColor(1, color);
            strip.setPixelColor(0, color);
            break;
    }


    //Minuten anzeige
    if(minutes < 5){

    } else if(minutes < 10){
        //Fünf
        strip.setPixelColor(88, color);
        strip.setPixelColor(89, color);
        strip.setPixelColor(90, color);
        strip.setPixelColor(91, color);
        //Nach
        strip.setPixelColor(59, color);
        strip.setPixelColor(60, color);
        strip.setPixelColor(61, color);
        strip.setPixelColor(62, color);
    } else if(minutes < 15){
        //Zehn
        strip.setPixelColor(77, color);
        strip.setPixelColor(78, color);
        strip.setPixelColor(79, color);
        strip.setPixelColor(80, color);
        //Nach
        strip.setPixelColor(59, color);
        strip.setPixelColor(60, color);
        strip.setPixelColor(61, color);
        strip.setPixelColor(62, color);
    } else if(minutes < 20){
        //Viertel
        strip.setPixelColor(66, color);
        strip.setPixelColor(67, color);
        strip.setPixelColor(68, color);
        strip.setPixelColor(69, color);
        strip.setPixelColor(70, color);
        strip.setPixelColor(71, color);
        strip.setPixelColor(72, color);
        //Nach
        strip.setPixelColor(59, color);
        strip.setPixelColor(60, color);
        strip.setPixelColor(61, color);
        strip.setPixelColor(62, color);
    } else if(minutes < 25) {
        //Zwanzig
        strip.setPixelColor(81, color);
        strip.setPixelColor(82, color);
        strip.setPixelColor(83, color);
        strip.setPixelColor(84, color);
        strip.setPixelColor(85, color);
        strip.setPixelColor(86, color);
        strip.setPixelColor(87, color);
        //Nach
        strip.setPixelColor(59, color);
        strip.setPixelColor(60, color);
        strip.setPixelColor(61, color);
        strip.setPixelColor(62, color);

    } else if(minutes < 30) {
        //Fünf
        strip.setPixelColor(88, color);
        strip.setPixelColor(89, color);
        strip.setPixelColor(90, color);
        strip.setPixelColor(91, color);
        //Vor
        strip.setPixelColor(63, color);
        strip.setPixelColor(64, color);
        strip.setPixelColor(65, color);
        //Halb
        strip.setPixelColor(55, color);
        strip.setPixelColor(56, color);
        strip.setPixelColor(57, color);
        strip.setPixelColor(58, color);
    } else if(minutes < 35) {
        //Halb
        strip.setPixelColor(55, color);
        strip.setPixelColor(56, color);
        strip.setPixelColor(57, color);
        strip.setPixelColor(58, color);
    } else if(minutes < 40) {
        //Fünf
        strip.setPixelColor(88, color);
        strip.setPixelColor(89, color);
        strip.setPixelColor(90, color);
        strip.setPixelColor(91, color);
        //Nach
        strip.setPixelColor(59, color);
        strip.setPixelColor(60, color);
        strip.setPixelColor(61, color);
        strip.setPixelColor(62, color);
        //Halb
        strip.setPixelColor(55, color);
        strip.setPixelColor(56, color);
        strip.setPixelColor(57, color);
        strip.setPixelColor(58, color);
    } else if(minutes < 45) {
        //Zwanzig
        strip.setPixelColor(81, color);
        strip.setPixelColor(82, color);
        strip.setPixelColor(83, color);
        strip.setPixelColor(84, color);
        strip.setPixelColor(85, color);
        strip.setPixelColor(86, color);
        strip.setPixelColor(87, color);
        //Vor
        strip.setPixelColor(63, color);
        strip.setPixelColor(64, color);
        strip.setPixelColor(65, color);
    } else if(minutes < 50) {
        //Viertel
        strip.setPixelColor(66, color);
        strip.setPixelColor(67, color);
        strip.setPixelColor(68, color);
        strip.setPixelColor(69, color);
        strip.setPixelColor(70, color);
        strip.setPixelColor(71, color);
        strip.setPixelColor(72, color);
        //Vor
        strip.setPixelColor(63, color);
        strip.setPixelColor(64, color);
        strip.setPixelColor(65, color);
    } else if(minutes < 55) {
        //Zehn
        strip.setPixelColor(77, color);
        strip.setPixelColor(78, color);
        strip.setPixelColor(79, color);
        strip.setPixelColor(80, color);
        //Vor
        strip.setPixelColor(63, color);
        strip.setPixelColor(64, color);
        strip.setPixelColor(65, color);

    } else {

        //Fünf
        strip.setPixelColor(88, color);
        strip.setPixelColor(89, color);
        strip.setPixelColor(90, color);
        strip.setPixelColor(91, color);
        //Vor
        strip.setPixelColor(63, color);
        strip.setPixelColor(64, color);
        strip.setPixelColor(65, color);
    }

    if(minutes >= 25) {
        hours = (hours + 1) % 12;
    }


    switch(hours){
        case 0:
            //Zwölf
            strip.setPixelColor(50, color);
            strip.setPixelColor(51, color);
            strip.setPixelColor(52, color);
            strip.setPixelColor(53, color);
            strip.setPixelColor(54, color);
            break;
        case 1:
            //Ein(s)
            strip.setPixelColor(41, color);
            strip.setPixelColor(42, color);
            strip.setPixelColor(43, color);
            if(minutes < 5)
            {
                strip.setPixelColor(44, color);
            };
            break;
        case 2:
            //Zwei
            strip.setPixelColor(39, color);
            strip.setPixelColor(40, color);
            strip.setPixelColor(41, color);
            strip.setPixelColor(42, color);
            break;
        case 3:
            //Drei
            strip.setPixelColor(35, color);
            strip.setPixelColor(36, color);
            strip.setPixelColor(37, color);
            strip.setPixelColor(38, color);
            break;
        case 4:
            //Vier
            strip.setPixelColor(27, color);
            strip.setPixelColor(28, color);
            strip.setPixelColor(29, color);
            strip.setPixelColor(30, color);
            break;
        case 5:
            //Fünf
            strip.setPixelColor(31, color);
            strip.setPixelColor(32, color);
            strip.setPixelColor(33, color);
            strip.setPixelColor(34, color);
            break;
        case 6:
            //Sechs
            strip.setPixelColor(4, color);
            strip.setPixelColor(5, color);
            strip.setPixelColor(6, color);
            strip.setPixelColor(7, color);
            strip.setPixelColor(8, color);
            break;
        case 7:
            //Sieben
            strip.setPixelColor(44, color);
            strip.setPixelColor(45, color);
            strip.setPixelColor(46, color);
            strip.setPixelColor(47, color);
            strip.setPixelColor(48, color);
            strip.setPixelColor(49, color);
            break;
        case 8:
            //Acht
            strip.setPixelColor(16, color);
            strip.setPixelColor(17, color);
            strip.setPixelColor(18, color);
            strip.setPixelColor(19, color);
            break;
        case 9:
            //Neun
            strip.setPixelColor(23, color);
            strip.setPixelColor(24, color);
            strip.setPixelColor(25, color);
            strip.setPixelColor(26, color);
            break;
        case 10:
            //Zehn
            strip.setPixelColor(12, color);
            strip.setPixelColor(13, color);
            strip.setPixelColor(14, color);
            strip.setPixelColor(15, color);
            break;
        case 11:
            //Elf
            strip.setPixelColor(20, color);
            strip.setPixelColor(21, color);
            strip.setPixelColor(22, color);
            break;
    }


    //Uhr
    strip.setPixelColor(9, color);
    strip.setPixelColor(10, color);
    strip.setPixelColor(11, color);


    strip.show();


}


void setup() {
    strip.begin();
    strip.show();
    Time.zone(2);
    
    // if color not saved, save it
    if(EEPROM.read(0) != 1){
      EEPROM.write(0, 1);
      EEPROM.put(1, color);

    }else{
      EEPROM.get(1, color);
    }

    //Spark.variable("hour", &hours_buffer, INT);
    //Spark.variable("minute", &minutes_buffer, INT);
    Spark.variable("brightness", &brightness, INT);
    Spark.function("setTimeZone", setTimeZone);
    Spark.function("setStatus", setClockStatus);
    Spark.function("controlColor", controlColor);


    pinMode(photoresistorPin, INPUT_PULLDOWN);

    RGB.control(true);
    RGB.brightness(0);
}

void loop() {

    if(showClock){
      int hours = Time.hour() % 12;
      int minutes = Time.minute();

      if((hours_buffer!=hours) || (minutes_buffer!=minutes)){
          display_time(hours, minutes);
          hours_buffer = hours;
          minutes_buffer=minutes;
          if (WiFi.ready()) {
            adjustTime();
          }
      }

      adjustBrightness();
    }else{
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
      }
      strip.show();
    }
}
