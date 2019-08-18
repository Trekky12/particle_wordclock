#include "Wordclock.h"
#include "neopixel.h"


Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// make mean of LDR values
const int LDR_Messungen_Anzahl = 50;
int LDR_Messungen[LDR_Messungen_Anzahl];

// Local TCP Server
TCPServer server = TCPServer(23);
TCPClient server_client;

Wordclock::Wordclock(){

}

void Wordclock::begin(){

    color = strip.Color(255,0,0);

    strip.begin();
    setBrightness(255);

    // EEPROM:
    // Byte 0   => Timezone value
    // Byte 1   => Color saved flag
    // Byte 2-4 => Color value

    // if timezone not saved, save it
    uint8_t timezone = 1;
    uint8_t timezone_value;
    EEPROM.get(0, timezone_value);
    if(timezone_value == 255) {
        EEPROM.put(0, timezone);
    }else{
        timezone = timezone_value;
    }
    Time.zone(timezone);

    // if color not saved, save it
    if(EEPROM.read(1) != 1) {
        EEPROM.write(1, 1);
        EEPROM.put(2, color);
    }else{
        EEPROM.get(2, color);
    }

    pinMode(LDR_PIN, INPUT_PULLDOWN);

    WiFi.setListenTimeout(60);
    System.set(SYSTEM_CONFIG_SOFTAP_PREFIX, "Wordclock");

    // Output LED
    pinMode(D7, OUTPUT);
    digitalWrite(D7, LOW);


    //Particle.variable("hour", &hours_buffer, INT);
    //Particle.variable("minute", &minutes_buffer, INT);

    Serial.println("Startup");

}


void Wordclock::update(){

    // Display Clock
    if(showClock && hasCredentials){
        int hours = Time.hour() % 12;
        int minutes = Time.minute();
        //int hours = 4;
        //int minutes = 12;

        if((hours_buffer!=hours) || (minutes_buffer!=minutes)){
           display_time(hours, minutes);
           hours_buffer = hours;
           minutes_buffer=minutes;
        }
        if (WiFi.ready()) {
            adjustTime();
        }
        if(autoBrightness){
            adjustBrightness();
        }
    }else{
        allLedsOff();
    }


    // Disable WiFi
    if(wlanOffTimeH < 24){
        if( ( Time.hour() == wlanOffTimeH) && (Time.minute() == wlanOffTimeM) && (Time.second() == wlanOffTimeS) && !wlanOff){
            disableWiFiNow = true;
        }
    }
    if(disableWiFiNow){
        disableWiFiNow =  false;
        Serial.println(Time.now());
        Serial.println("Disable WiFi");
        delay(1000);
        setWiFiState(false);
    }

     // Enable WiFi
    if(wlanOnTimeH < 24){
        if( ( Time.hour() == wlanOnTimeH) && (Time.minute() == wlanOnTimeM) && (Time.second() == wlanOnTimeS) && wlanOff){
            Serial.println(Time.now());
            Serial.println("Enable WiFi");
            setWiFiState(true);
        }
    }


    // Apparently we already have the WiFi credentials
    if(!hasCredentials && WiFi.ready()){
        hasCredentials = true;

        RGB.control(true);
        RGB.brightness(0);

        Particle.function("setTimeZone", &Wordclock::setTimeZone, this);
        Particle.function("setLight", &Wordclock::setClockLight, this);
        Particle.function("controlColor", &Wordclock::controlColor, this);
        Particle.function("getColor", &Wordclock::getColor, this);
        Particle.function("disablewifi", &Wordclock::disableWiFi, this);
        Particle.function("listen", &Wordclock::listen, this);
        Particle.function("version", &Wordclock::getVersion, this);

        //Particle.variable("h", hours_buffer);
        //Particle.variable("m", minutes_buffer);

        Particle.publish("register", Time.timeStr());

    }

    // No connection? => Go into listening mode
    if(!wlanOff && !WiFi.ready() && !hasCredentials && (millis() - startConnection >= CONNECTION_TIMEOUT) ) {

        RGB.brightness(255);
        RGB.control(false);

        WiFi.listen();
        //WiFi.setListenTimeout(0);
    }

    // Handle Local server
    handleLocalServer();
}


void Wordclock::setWiFiOffTime(uint8_t hour, uint8_t minute, uint8_t second){
    wlanOffTimeH = hour;
    wlanOffTimeM = minute;
    wlanOffTimeS = second;
}

void Wordclock::setWiFiOnTime(uint8_t hour, uint8_t minute, uint8_t second){
    wlanOnTimeH = hour;
    wlanOnTimeM = minute;
    wlanOnTimeS = second;
}

void Wordclock::setAutoBrightness(bool state){
    autoBrightness = state;
}

void Wordclock::setBrightness(uint8_t value){
    strip.setBrightness(value);
    strip.show();
}

void Wordclock::blinkLED(uint8_t times){
    for(uint8_t i=0; i < times; i++) {
        digitalWrite(D7, HIGH);
        delay(200);
        digitalWrite(D7, LOW);
        delay(200);
    }
}


bool Wordclock::isAutoBrightnessOn(){
    return autoBrightness;
}

bool Wordclock::isWiFiOn(){
    return !wlanOff;
}




/*
=====================================================================================
            Cloud functions
=====================================================================================
*/

// This function gets called whenever there is a matching API request
// the command string format is
// <LED_Red>,<LED_Green>,<LED_Blue>
// for example: 000,255,000
int Wordclock::controlColor(String command) {
    uint8_t red = 0, green = 0, blue = 0;

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

        color = strip.Color(red, green, blue);
        EEPROM.put(2, color);
        display_time(hours_buffer, minutes_buffer);
        return 0;
    }
    return -1;
}

// This function gets called whenever there is a matching API request
// the command string format is the number o the current timezone
// for example: 2
int Wordclock::setTimeZone(String zone){
    uint8_t timezone = zone.toInt();
    EEPROM.put(0, timezone);
    Time.zone(timezone);
    update();
    return 0;
}


// This function gets called whenever there is a matching API request
// the command string format is the status of the leds ("on"/"off")
int Wordclock::setClockLight(String status){
    if (status == "off"){
        showClock = false;
    }else{
        showClock = true;
        display_time(hours_buffer, minutes_buffer);
    }
    return 0;
}


int Wordclock::getColor(String command){
    return splitColor(command.charAt(0));
}

int Wordclock::disableWiFi(String wlan){
    disableWiFiNow = true;
    return 0;
}

int Wordclock::listen(String command){
    WiFi.listen();
    //WiFi.setListenTimeout(0);
    return 0;
}

int Wordclock::getVersion(String command){
    return version;
}


/*
=====================================================================================
            Private functions
=====================================================================================
*/

void Wordclock::allLedsOff(){
    for(int i = 0; i<PIXEL_COUNT; i++){
            strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}



uint8_t Wordclock::splitColor ( char value ){
    switch ( value ) {
        case 'r': return (uint8_t)(color >> 16);
        case 'g': return (uint8_t)(color >>  8);
        case 'b': return (uint8_t)(color >>  0);
        default:  return 0;
    }
}


void Wordclock::setWiFiState(bool state){
    if(state){
        WiFi.on();
        Particle.connect();
        wlanOff = false;
        startConnection = millis();
    }else{
        WiFi.off();
        wlanOff = true;
    }
}

void Wordclock::adjustBrightness() {

    uint8_t stripbrightness = 255;
    int brightness = 0;


    // LDR_Messungen-Puffer Inhalte weiterruecken
    for (int i = LDR_Messungen_Anzahl-1; i > 0; i = i - 1) {
        LDR_Messungen[i] = LDR_Messungen[i - 1];
    }
    LDR_Messungen[0] = analogRead(LDR_PIN);   // meanLDR an erste Stelle des LDR_Messungen schreiben


    // LDR_Messungen-Puffer Inhalte addieren
    for (int i = 0; i < LDR_Messungen_Anzahl; i = i + 1) {
        brightness = brightness + LDR_Messungen[i];
    }
    brightness = brightness / LDR_Messungen_Anzahl; // gemittelten meanLDR errechnen

     //brightness = map(analogRead(LDR_PIN), 0, 4095, 0, 100);
     //brightness = map(meanLDR, 0, 4095, 0, 100);

     if (brightness < 20){
         stripbrightness = 5;
     }else if (brightness < 30){
         stripbrightness = 10;
     }else if (brightness < 100){
         stripbrightness = 20;
     }else if (brightness < 400){
         stripbrightness = 100;
     }else if (brightness < 1000){
         stripbrightness = 100;
     }else if (brightness < 1500){
         stripbrightness = 150;
     }else{
       stripbrightness = 255;
     }

     strip.setBrightness(stripbrightness);
     strip.show();
 }


 void Wordclock::adjustTime(){
     if(WiFi.ready() && (lastSync + SYNC_INTERVAL < Time.now())) {
         Particle.publish("sync", Time.timeStr());
         Particle.syncTime();
         lastSync = Time.now();
     }
 }



void Wordclock::display_time(int hours, int minutes){

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
         //strip.setPixelColor(59, color);
         //strip.setPixelColor(60, color);
         //strip.setPixelColor(61, color);
         //strip.setPixelColor(62, color);
     } else if(minutes < 25) {
         //Zwanzig
         /*strip.setPixelColor(81, color);
         strip.setPixelColor(82, color);
         strip.setPixelColor(83, color);
         strip.setPixelColor(84, color);
         strip.setPixelColor(85, color);
         strip.setPixelColor(86, color);
         strip.setPixelColor(87, color);*/
         //Nach
         /*strip.setPixelColor(59, color);
         strip.setPixelColor(60, color);
         strip.setPixelColor(61, color);
         strip.setPixelColor(62, color);*/
         //Zehn
         strip.setPixelColor(77, color);
         strip.setPixelColor(78, color);
         strip.setPixelColor(79, color);
         strip.setPixelColor(80, color);
         //Vor
         strip.setPixelColor(63, color);
         strip.setPixelColor(64, color);
         strip.setPixelColor(65, color);
         //Halb
         strip.setPixelColor(55, color);
         strip.setPixelColor(56, color);
         strip.setPixelColor(57, color);
         strip.setPixelColor(58, color);

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
         /*strip.setPixelColor(81, color);
         strip.setPixelColor(82, color);
         strip.setPixelColor(83, color);
         strip.setPixelColor(84, color);
         strip.setPixelColor(85, color);
         strip.setPixelColor(86, color);
         strip.setPixelColor(87, color);*/
         //Vor
         /*strip.setPixelColor(63, color);
         strip.setPixelColor(64, color);
         strip.setPixelColor(65, color);*/
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
         //Halb
         strip.setPixelColor(55, color);
         strip.setPixelColor(56, color);
         strip.setPixelColor(57, color);
         strip.setPixelColor(58, color);
     } else if(minutes < 50) {
         //Drei
         strip.setPixelColor(73, color);
         strip.setPixelColor(74, color);
         strip.setPixelColor(75, color);
         strip.setPixelColor(76, color);
         //Viertel
         strip.setPixelColor(66, color);
         strip.setPixelColor(67, color);
         strip.setPixelColor(68, color);
         strip.setPixelColor(69, color);
         strip.setPixelColor(70, color);
         strip.setPixelColor(71, color);
         strip.setPixelColor(72, color);
         //Vor
         /*strip.setPixelColor(63, color);
         strip.setPixelColor(64, color);
         strip.setPixelColor(65, color);*/
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

     /*if(minutes >= 25) {
         hours = (hours + 1) % 12;
     }*/
     if(minutes >= 15) {
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

             if(minutes >= 5)
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
     if(minutes < 5){
       strip.setPixelColor(9, color);
       strip.setPixelColor(10, color);
       strip.setPixelColor(11, color);
     }


     strip.show();


 }

void Wordclock::handleLocalServer(){
     if (WiFi.ready()) {
         if(!serverStarted){
 			server.begin();
 			serverStarted = true;
         }

         if (server_client.connected()) {
 			while(server_client.available()){
 			    char incoming = server_client.read();
 			    Serial.print(incoming);

                // setLight on
			    if(incoming == 'B'){
                    showClock = true;
                    display_time(hours_buffer, minutes_buffer);
			    }
                // setLight off
			    if(incoming == 'C'){
                    showClock = false;
			    }
                // disableWiFi
                if(incoming == 'D'){
			        disableWiFiNow = true;
			    }
                // Summer time
                if(incoming == 'E'){
                    uint8_t timezone = 1;
                    EEPROM.put(0, timezone);
                    Time.zone(timezone);
                    update();
			    }
                // Winter time
                if(incoming == 'F'){
                    uint8_t timezone = 2;
                    EEPROM.put(0, timezone);
                    Time.zone(timezone);
                    update();
			    }

                // Debug
                if(incoming == 'X'){
			        digitalWrite(D7, HIGH);
			    }
			    if(incoming == 'Y'){
			        digitalWrite(D7, LOW);
			    }
 			    server_client.write(incoming);
 			}
 			server_client.stop();
           } else {
             server_client = server.available();
           }
     }else{
         if(serverStarted){
            Serial.println("server stop");
            server_client.stop();
 			server.stop();
 			serverStarted = false;
         }
     }
 }
