
#define BLE 0

        #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
        #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

        #define MAX_RECEIVED_ADATA_SIZE 10

        #define TOUCH_SENSOR_GPIO 13
//#define TOUCH_SENSOR_THRESHOLD 20

        #define BATTERY_GPIO 35

        #define BLE 0
        #define WEB_UPDATE 1
//#define POWER_SAVE_INTERVAL 3000

        #define EEPROM_SIZE 1
        #define EEPROM_PLACE 0

        #include <EEPROM.h>

// Prepare to deepsleep
        //   #include "driver/adc.h"
//            #include <esp_WiFi.h>
//        #include <esp_bt.h>

// Disable brownout detector
        //  #include "soc/soc.h"
        //    #include "soc/rtc_cntl_reg.h"

// OLED
        #include <Arduino.h>
        #include <U8g2lib.h>

        #ifdef U8X8_HAVE_HW_SPI
        #include <SPI.h>
        #endif
        #ifdef U8X8_HAVE_HW_I2C
        #include <Wire.h>
        #endif

// BLE
        //   #include <BLEDevice.h>
        //    #include <BLEUtils.h>
        //  #include <BLEServer.h>



        /* -------------- Variables -------------- */
        U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_MIRROR, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C
        U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8G2_MIRROR, /* reset=*/ U8X8_PIN_NONE);
        bool shouldDrawToOled = false;
        String *aOledSendData;
        unsigned long powerSavePreviousMillis = 0;
        unsigned long powerSaveCurrentMillis = 0;
        //theword comment

        String theword;

        //intVoltage
        int intVoltage;

        //pad stuff

        int pad4 = 0;
        int pad0 = 0;
        int pad2 = 0;
        int pad15 = 0;
        int pad27 = 0;

        //serial port



        const char* ble_name = "ESP32 (SmartGlasses)";
        bool isDeviceConnected = false;

        //   WebServer server(80);
        const char* ssid = "ESP32Update";
        const char* password = "12345678";

        //RTC_DATA_ATTR bool isDeepSleepBoot = false;

        // Variables changeable during BLE connect
        int _MarginX = 0;
        int _MarginY = 0;

        //D5 to D9 , or pins 8 to 12
        int _TouchSensorGPIO = 9;
        int _TouchSensorThreshold = 20;
        int _PowerSaveInterval = 8000;



        int _TouchSensorGPIO15 = 15;
        int _TouchSensorGPIO2 = 2;
        int _TouchSensorGPIO0 = 0;
        int _TouchSensorGPIO4 = 4;
        int _TouchSensorGPIO27=27;

        float _BatteryVoltageFlat       = 3.1f; // In my case 3.1V from ADC means ~3.6V from battery ( ADC in ESP is not linear! so we cant calculate exact value of battery just estimate!)

        /* --- Operating Mode --- */
        int iOperatingMode = -1;        // 0 - BLE, 1 - WEB_UPDATE, default = BLE

        int getOperatingMode() {
        if (iOperatingMode == -1) iOperatingMode = 0;
        return 0; //hardcode always to BLE
        }

        void setOperatingMode(int om, bool restart) {
        if (iOperatingMode == om) return;       // if already set

        iOperatingMode = om;

        if (restart) {
        Serial.print("[Warning] Writing to EEPROM. Mode= "); Serial.println(om);
        // EEPROM.write(EEPROM_PLACE, om); EEPROM.commit(); //Writing om (operating mode) to EEPROM_PLACE, which is 0 at init, as said by #define
        delay(100);
        //   ESP.restart();
        }
        }

/* --- Button Class --- */
class ButtonClass {
    int lastState=0; int lastState0=0; int lastState2=0; int lastState4=0; int lastState15=0;
    //int lastState=
    long clickBreakTime=50;      // \]\]\] STATIC
    long lastClickTime=0; //long lastClickTime0=0; long lastClickTime2=0; long lastClickTime4=0; long lastClickTime15=0;

    int multiClickCounter =0; //int multiClickCounter0=0; int multiClickCounter2=0; int multiClickCounter4=0; int multiClickCounter15=0;
    long multiClickMaxTime = 250;

    bool longClickDetected = false; // \]\]\] DYN, not req
    long longClickTime=500;

    bool shouldUpdate = false;


    int ReturningValue =-1;
    int btnState;
    int samplesCurrent = 0; //int samplesCurrent0=0; int samplesCurrent2=0; int samplesCurrent4=0; int samplesCurrent15=0;      // Positive reads until negative happens (prevents a situation where a microsecond long accidental touch occurs)
    int samplesLimit = 20;          // \]\]\] STATIC

    public:
    // -1 - no action, 0 - long click, 1 - click, x - multiple click
    int detect(int GPIO) {
        long now = millis();            // \]\]\] TIME IPPO
        // int btnState = (touchRead(GPIO) < _TouchSensorThreshold) ? 1 : 0; // \]\]\] IF GPIO's less than threshold (when you touching it), btnstate is yes, ille na no
        boolean booState = !digitalRead(GPIO);
        if (booState)
        {
            btnState = 1;
        } else {
            btnState = 0;
        }

        // Samples - prevent situation when accidentally device detect one touch for microsecond
//            if (btnState != lastState) {
//                samplesCurrent++;
//
//                if (samplesCurrent < samplesLimit) return -1;
//            }
//            samplesCurrent = 0;

//    Serial.println(touchRead(GPIO));
//    Serial.println(btnState);

        if (btnState != lastState && (now - lastClickTime) > clickBreakTime) {     // \]\]\] if btnstate isn't laststate (y/n), and (delay> 50 millis)
            if (btnState == 0 && !longClickDetected) {                             // Button click detected when button goes up
                multiClickCounter++;
            }

            lastState = btnState;
            lastClickTime=now;
            longClickDetected=false;
        }

        if ( (now - lastClickTime) > longClickTime) {
            if (btnState == 1 && !longClickDetected) {                         // \]\]\] if delay is greater than 500 millis
                longClickDetected=true;                                        // \]\]\] if btnstate yes, and there's no long click
                shouldUpdate=true;
                // \]\]\] make long click true, then update
            }
        }

        if ( ((now - lastClickTime) > multiClickMaxTime) && multiClickCounter > 0 ) shouldUpdate=true;


        if (shouldUpdate) {
            if      (longClickDetected    ) { ReturningValue = 0; }
            else if (multiClickCounter==1 ) { ReturningValue = 1; }
            else if (multiClickCounter >1 ) { ReturningValue = 1;  } //hardcode force all to #TS1
            shouldUpdate=false;
            multiClickCounter=0;
        }

        //ifshouldupdate

        return ReturningValue;
    }
};

/* --- Custom Display Functions --- */ //array lam ullana
class DisplayClass {
    public:
    int __tick = 150; // ms

    int __previousScreen=0;
    int __currentScreen=0;

    long __lastTickTime=0;



    // Offset for scroll/other simple animations
    int __Offset=0;

    void tickScreen(String *aDataStr) {
        __currentScreen = aDataStr[0].toInt();

        // Tick every 250 ms
        long tickTime = millis();

        if ( (tickTime - __lastTickTime) > __tick ) {
            if (__previousScreen != __currentScreen)  __Offset=0;        // When powersavemode cut a text for example from SMS u can just continue reading - just call this screen again it will continue from prev stop

            switch (__currentScreen) {
                case 0: default : screenMain   (aDataStr[1]        , aDataStr[2]        , aDataStr[3]        , aDataStr[4].toInt(), aDataStr[5] ); break;
                case 1          : screenMsgNoti(aDataStr[1].toInt(), aDataStr[2]        , aDataStr[3]                                           ); break;
                case 2          : screenCall   (aDataStr[1]                                                                                     ); break;// case 2          : screenCall   (aDataStr[1]                                                                                     ); break;
                case 3          : screenNav    (aDataStr[1]        , aDataStr[2]        , aDataStr[3]        , aDataStr[4].toInt()              ); break;
                case 4          : screenList   (aDataStr[1].toInt(), aDataStr[2]        , aDataStr[3].toInt(), aDataStr[4]                      ); break;
                case 5          : screenMusic  (aDataStr[1].toInt(), aDataStr[2]        , aDataStr[3].toInt(), aDataStr[4].toInt()              ); break;
            }

            __previousScreen = __currentScreen;
            __lastTickTime = tickTime;
        }
    }

    // --- Screens ---
    void screenMusic(int musicIcon, String title, int symbolPlayStop, int symbolNext) {
        int titleFontSize = 8;

        //setFontSize(titleFontSize);
        int d_width = u8g2.getDisplayWidth();
        //int t_width = getStringWidth(title);

        // Draw
        u8g2.clearBuffer();

        // Scroll text
        drawSymbol(0, 9, musicIcon, 1);

        setFontSize(titleFontSize);
        drawString(13, 9, title);

        // Symbols
        drawSymbol(0 , 30, symbolPlayStop, 2);
        drawSymbol(22, 30, symbolNext, 2);

        u8g2.sendBuffer();
    }

    void screenList(int symbolMain, String title, int symbolSub, String text) {                                    // List - Shopping List; 1 text than max
        // Max 2 lines
        String line1 = text.substring(0 , 10);
        String line2 = text.substring(11, 20);

        u8g2.clearBuffer();

        drawSymbol(0, 9, symbolMain, 1); setFontSize(8 ); drawString(13, 9, title);

        // Lines
        drawSymbol(0, 22, symbolSub, 1);

        setFontSize(8);
        drawString(10, 21, line1);
        drawString(10, 30, line2);
        u8g2.sendBuffer();
    }

    void screenNav(String maxSpeed, String distance, String distanceToDes, int symbol) {
        u8g2.clearBuffer();
        drawSymbol(0, 22, symbol, 2);

        setFontSize(7 ); drawString(18, 7, maxSpeed);
        setFontSize(12); drawString(21, 21, distance);
        setFontSize(7 ); drawString(18, 31, distanceToDes);
        u8g2.sendBuffer();
    }

    void screenCall(String from) {
        // Restart offset when 3 dots reached but give 2 more updates for larger delay beetween 3 dots and back to 1 dot
        if (__Offset >= 6) __Offset=0;

        u8g2.clearBuffer();
        drawSymbol(0, 9, 260, 1);

        setFontSize(4);
        if      (__Offset == 0) drawString(12, 9, "Typing.");
        else if (__Offset == 1) drawString(12, 9, "Typing..");
        else                    drawString(12, 9, "Typing...");
        setFontSize(4); drawString(0, 22, theword);

        u8g2.sendBuffer();

        // Increase dots offset
        __Offset++;
    }

    void screenMsgNoti(int symbol, String from, String text) {                                    // Msg/Notification
        int msgFontSize = 10;
        int d_width = u8g2.getDisplayWidth();
        String cut = text;

        // Restart scroll when reaches end
        if (__Offset >= text.length() ) __Offset=0;

        u8g2.clearBuffer();

        // Same text
        drawSymbol(0, 9, symbol, 1);
        setFontSize(8 ); drawString(13, 9, theword);

        // Scroll Text
        setFontSize(msgFontSize);

        cut = text.substring(0 + __Offset, d_width - 108 + __Offset);                       // For this font 108

        drawString(0, 25, cut);
        u8g2.sendBuffer();

        // Increase offset for scroll
        __Offset++;
    }

    void screenMain(String HH, String mm, String date, int symbol, String degrees) {
        // Serial.println(time + " OK");
        int addDePx = 0;
        if      (degrees .length() <  3) addDePx = 4;

        if (__Offset >= 10) __Offset=0;

        // Time always in format: HH:mm
        // Degrees format: DD'C' - when only one D then we have to move everything

        u8g2.clearBuffer();
        setFontSize(12);

        // Blinking ":" -> 15:40
        drawString(0, 17, HH);
        if      (__Offset < 4) drawString(20, 17, ":");
        drawString(25, 17, mm);

        setFontSize(8 ); drawString(4          , 29, date);

        drawSymbol(52         , 22, symbol, 2);
        setFontSize(7 ); drawString(50 + addDePx, 30, degrees);

        u8g2.sendBuffer();
        __Offset++;
    }

    // Utilities - Symbols
    void drawSymbol(int x, int y, int index, int size) {                                        // https://github.com/olikraus/u8g2/wiki/u8g2reference#drawglyph
        switch (size) {
            case 1  : u8g2.setFont(u8g2_font_open_iconic_all_1x_t ); break;                         // 8 px height
            case 2  : u8g2.setFont(u8g2_font_open_iconic_all_2x_t ); break;                         // 16 px height
            case 4  : u8g2.setFont(u8g2_font_open_iconic_all_4x_t ); break;
            case 6  : u8g2.setFont(u8g2_font_open_iconic_all_6x_t ); break;
            case 8  : u8g2.setFont(u8g2_font_open_iconic_all_8x_t ); break;
            default : u8g2.setFont(u8g2_font_open_iconic_all_1x_t ); break;
        }

        u8g2.drawGlyph(_MarginX+x, _MarginY+y, index);                                                         // podawanie w hex lub index (na stronie u8g2 w wybranej czcionce po lewej stronie pisze ktory to znak (dec/hex) tylko to index 1 znaku w linii, jak chcesz inny to dodaj sobie i masz :)
    }

    // Utilities - Text
    int getStringWidth(String text) {               // Estimated
        int t_width = 0;
        String s = "";

        for (int i = 0; i < text.length(); i++) {
            s = text.charAt(i);
            t_width += u8g2.getStrWidth(s.c_str()) + 1;        // 1 space between characters
        }

        return t_width;
    }

    void setFontSize(int size) {
        switch (size) {
            case 4 : u8g2.setFont(u8g2_font_u8glib_4_tr)      ; break;
            case 5 : u8g2.setFont(u8g2_font_micro_tr)         ; break;
            case 6 : u8g2.setFont(u8g2_font_5x8_tr)           ; break;                                              // 6 px height - default
            case 7 : u8g2.setFont(u8g2_font_profont11_tr)     ; break;
            case 8 : u8g2.setFont(u8g2_font_profont12_tr)     ; break;
            case 9 : u8g2.setFont(u8g2_font_t0_14_tr)         ; break;
            case 10: u8g2.setFont(u8g2_font_unifont_tr)       ; break;
            case 12: u8g2.setFont(u8g2_font_samim_16_t_all)   ; break;
            case 18: u8g2.setFont(u8g2_font_ncenR18_tr)       ; break;
            default: u8g2.setFont(u8g2_font_5x8_tr)           ; break;
        }
    }

    void drawString(int x, int y, String text) { u8g2.drawStr(_MarginX+x, _MarginY+y, text.c_str() ); }

    // --- Other ---
    void sendBuffer() { u8g2.sendBuffer(); }

    void clearBuffer() { u8g2.clearBuffer(); }
    void clear() { u8g2.clear(); }

    int lastPowerSaveMode = -1;
    bool setPowerSave(int i) {
        bool changed=false;

        if (lastPowerSaveMode != i) {
            if (i == 1) Serial.println("##[INFO - OLED] Power save mode");
            u8g2.setPowerSave(i);
            changed = true;
        }
        lastPowerSaveMode = i;

        return changed;
    }
};

/* --- BLE - Data receive --- */
class BLEReceive {
    public:

    // --- Connected/Disconnected ---
//        class BLEConnectState : public BLEServerCallbacks {
//            void onConnect(BLEServer* pServer) {
//                isDeviceConnected = true;
//                Serial.println("##[INFO - BLE] Device connected");
//                //BLEDevice::startAdvertising();
//            }
//
//            void onDisconnect(BLEServer* pServer) {
//                isDeviceConnected = false;
//                Serial.println("##[INFO - BLE] Device disconnected");
//                delay(100);
//                ESP.restart();    // Bug in BLE cant connect after disconnecting so just restart for now
//            }
//        };

    // --- Receive ---
    class BLEReceiveClass {    //class BLEReceiveClass : public BLECharacteristicCallbacks {
        String aReceivedData[MAX_RECEIVED_ADATA_SIZE];

        int    indexRD = 0;

        void addReceivedData(String s) {
            aReceivedData[indexRD++] = s;
        }
        String getReceivedData(int i) {
            return aReceivedData[i];
        }
        void clearReceivedData() {
            for (int i = 0; i < MAX_RECEIVED_ADATA_SIZE; i++) {
                aReceivedData[i] = "";
            };
            indexRD = 0;
        }

        void onWrite() {            //void onWrite(BLECharacteristic *pCharacteristic) {
            String sReceived = "";
            //std::string receivedValue = pCharacteristic->getValue();

            if (Serial.available() > 0){                                 //OLD>ESP it was  "     if (receivedValue.length() > 0) {   "

                sReceived = Serial.readString();
                // OLD>ESP Serial.print("[INFO - BLE] Received: "); Serial.println(sReceived);

                // --- Change Operating Mode - from ble to wu ---
                if      (sReceived == "#OM=0"   )  { setOperatingMode(BLE        , true); return; }
                else if (sReceived == "#OM=1"   )  { setOperatingMode(BLE , true); return; }
                //else if (sReceived == "#RESTART")  { ESP.restart()                      ; return; }

                else if (sReceived.startsWith("#MX="  ))  { _MarginX              =sReceived.substring(4, sReceived.length()).toInt(); Serial.println("##"+_MarginX)             ; return; }  // Margin X             Def: 0
                else if (sReceived.startsWith("#MY="  ))  { _MarginY              =sReceived.substring(4, sReceived.length()).toInt(); Serial.println("##"+_MarginY)             ; return; }  // Margin Y             Def: 0
                else if (sReceived.startsWith("#PSI=" ))  { _PowerSaveInterval    =sReceived.substring(5, sReceived.length()).toInt(); Serial.println("##"+_PowerSaveInterval)   ; return; }  // PowerSaveInterval    Def: 8000
                else if (sReceived.startsWith("#TSG=" ))  { _TouchSensorGPIO      =sReceived.substring(5, sReceived.length()).toInt(); Serial.println("##"+_TouchSensorThreshold); return; }  // TouchSensorGPIO      Def: 13
                else if (sReceived.startsWith("#TST=" ))  { _TouchSensorThreshold =sReceived.substring(5, sReceived.length()).toInt(); Serial.println("##"+_TouchSensorThreshold); return; }  // TouchSensorThreshold Def: 20

                //else if (sReceived.startsWith("#BF="  ))  { _BatteryVoltageFlat   =sReceived.substring(4, sReceived.length()).toFloat(); Serial.println("##"+_BatteryVoltageFlat); return; }  // Battery Flat Voltage Def: 3.1f // In my case 3.1V from ADC means ~3.6V from battery ( ADC in ESP is not linear! so we cant calculate exact value of battery just estimate!)

                // #0,Text,saf....  - #0 - screen select - 0=main
                if (sReceived.startsWith("#")) {
                    sReceived = sReceived.substring(1, sReceived.length());                        // /]/]/]/] IF Phone command recieved, getting rid of #

                    //clearReceivedData();

                    // Split String by delimeter - '|'
                    for (int i = 0; i < sReceived.length(); i++) {
                        if (indexRD >= MAX_RECEIVED_ADATA_SIZE) break;                                                        // Over the array param limit - something gone wrong

                        if (sReceived.charAt(i) == '|') {
                            indexRD++;
                            continue;
                        }
                        aReceivedData[indexRD] += sReceived.charAt(i);
                    }

                    aOledSendData = aReceivedData;
                    shouldDrawToOled = true;
                    powerSavePreviousMillis = millis();                                                // Measure time from now (ble send)
                    return;
                }
            }
        }
    };

    // --- Send ---
    //BLECharacteristic *pCharacteristic;
//        void sendValue(String value) {
//            pCharacteristic->setValue(value.c_str());
//            pCharacteristic->notify();
//            delay(30);
//        }

    // --- Init ---
    void init() {
//            BLEDevice::init(ble_name);
//
//            BLEServer *pServer = BLEDevice::createServer();
//            pServer->setCallbacks(new BLEConnectState());
//
//            BLEService *pService = pServer->createService(SERVICE_UUID);
//
//            pCharacteristic = pService->createCharacteristic(
//                    CHARACTERISTIC_UUID,
//                    BLECharacteristic::PROPERTY_READ |
//                            BLECharacteristic::PROPERTY_WRITE
//            );
//
//            pCharacteristic->setCallbacks(new BLEReceiveClass());
//            pCharacteristic->setValue("1");
//
//            pService->start();
//
//            BLEAdvertising *pAdvertising = pServer->getAdvertising();
//            pAdvertising->start();
//
//            String sBleMacAddress = BLEDevice::getAddress().toString().c_str();
//
        Serial.println("[INFO - BLE] Starting with MAC address: ");
//            Serial.println(sBleMacAddress);
    }
};


//      //     //      ////     //
//
//      //     //      ////     //   N E W           N E W              N E W
//




    /*
Programmed by Brian McEvoy. 24 Hour Engineer
24hourengineer.com
Program is distributable for personal use.
*/

// Inputs. Buttons may be addressed by name but the program expects all buttons after the pinky
// to be numbered sequentially.

        int fifthbutton = 5;
                int fourthbuttonSIX = 6;
                int middleButton = 7;
                int secondButtonEIGHT = 8;
                int firstbuttonNINEnearT = 9;
                //  int centerTButton = 9;
                // int farTButton = 10;

                // Program integers
                int prefixChord = 0;            // 1 = shift (F). 2 = numlock (N). 3 = special (CN). 4 = function keys
                int chordValue = 0;
                int randomNumber01;
                int randomNumber02;
                int randomNumber03;
                int randomNumber04;
                int debounceDelay = 20;

                int sign1 = 0;
                int sign2 = 0;

                // Booleans
                boolean buttons[5];     // Pinky is [0] and far thumb is [6]
                boolean latchingButtons[5];
                boolean acquiringPresses = LOW;
                boolean calculateKey = LOW;
                boolean stickyCapsLock = LOW;
                boolean stickyNumlock = LOW;
                boolean stickySpecialLock = LOW;



                DisplayClass dc;
                BLEReceive ble;
                ButtonClass button;


                void setup(){
//        Serial1.begin(9600);

                Serial.begin(9600);  //communication rate of the HC05 Module
                Serial.println("##Up and runnning");


                randomSeed(analogRead(0));

                pinMode(fifthbutton, INPUT_PULLUP);
                pinMode(fourthbuttonSIX, INPUT_PULLUP);
                pinMode(middleButton, INPUT_PULLUP);
                pinMode(secondButtonEIGHT, INPUT_PULLUP);
                pinMode(firstbuttonNINEnearT, INPUT_PULLUP);



                delay(3000);
                u8g2.begin();


                Serial.println("##[INFO] OperatingMode: BLE");
                ble.init();


                // Info OLED
                delay(100);
                dc.setFontSize(7); dc.drawString(10, 10, "Ready. Mode:" ); dc.drawString(10, 20, (getOperatingMode() == WEB_UPDATE) ? "WU" : "BLE" ); dc.sendBuffer(); powerSavePreviousMillis = millis();


//        pinMode(centerTButton, INPUT_PULLUP);
//        pinMode(farTButton, INPUT_PULLUP);
                }

                void loop(){
                acquiringPresses = checkButtonArray();

//        if (acquiringPresses && onlyFarThumbPressed(farTButton)){
//            doMouseSTUFF();//fifthbutton
//        }

                int action = button.detect(_TouchSensorGPIO);

                if ( action > -1 ){
                if(action==0){Serial.println("#TS0");} // 0 - long click
                else if(action>0){Serial.println("#TS"+String(action));}
                }

                if (acquiringPresses){
                delay(debounceDelay);                           // Instead of a true software debounce this will wait a moment until the first button press has settled.
                typingChord();                      // Wait and see which keys are touched. When they are all released print the correct letter.
//            updateShiftKeys();          // Change the prefixChord value if any of the 'locks' are set. Example, Num Lock or Caps Lock.
                //   sendKeyPress();                     // Using the buttons pressed during the typingChord function determine how to handle the chord.
                delay(debounceDelay);                           // The other half of the software "debounce"
                for (int i = 0; i < 5; i++){                                        //         {}_}_{}// Once a keypress has been sent, make all latching buttons zero.
        latchingButtons[i] = LOW;
        }
        chordValue = 0;
        }

        }





        boolean checkButtonArray(){
        // Update the buttons[] array with each scan. Set the acquiringPresses bit HIGH if any switch is pressed.
        for (int i = 0; i < 5; i++){
        boolean buttonState = !digitalRead(fifthbutton + i);
        if (buttonState){
        buttons[i] = HIGH;
        }else{
        buttons[i] = LOW;
        }
        }
        for (int i = 0; i < 5; i++){
        if (buttons[i]){
        return HIGH;
        }
        }
        return LOW;
        }

        void typingChord(){
        while (acquiringPresses){







        for (int i = 0; i < 5; i++){
        if (buttons[i] == HIGH){
        latchingButtons[i] = HIGH;
        }
        }
        acquiringPresses = checkButtonArray();
        }
        }

        void sendKeyPress(){
        for (int i = 0; i < 5; i++){
        if (latchingButtons[i] == HIGH){
        chordValue = chordValue + customPower(2, i);

        }
        }
        Serial.println(keySwitch(chordValue));
        // Serial.write(keySwitch(chordValue));



        }

        int customPower(int functionBase, int functionExponent){
        int functionResult = 1;
        for (int i = 0; i < functionExponent; i++){
        functionResult = functionResult * functionBase;
        }
        return functionResult;
        }

        int keySwitch(int functionChordValue){
        switch (functionChordValue){
        case 0:
        prefixChord = 0;
        //  Keyboard.releaseAll();
        stickyNumlock = LOW;
        stickyCapsLock = LOW;
        stickySpecialLock = LOW;
        return 0;     // error
        case 1:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('w');
        return 119;          // 119 is equivalent to the letter 'w'

        }
        case 2:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('y');
        return 121;          // 121 is equivalent to the letter 'y'

        case 3:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('u');
        return 117;          // 117 is equivalent to the letter 'u'
        }
        case 4:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('r');
        return 114;          // 114 → r
        }
        case 5:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println("Oho");
        //  Serial1.print("Oho");
        return 0;

        }
        case 6:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('h');
        return 104;          // 104 → h

        }
        case 7:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('s');
        return 115;          // 115 → s

        }
        case 8:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('i');
        return 105;          // 105 → i

        }
        case 9:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('b');
        return 98;          // 98 → b

        }
        case 10:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('k');
        return 107;          // 107 → k

        }
        case 11:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('z');
        return 122;          // 122 → z

        }
        case 12:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('d');
        return 100;          // 100 → d

        }
//            case 13:                  // This chord is open
//                randomNumber01 = random(0, 255);
//                randomNumber02 = random(0, 1000);
//                randomNumber03 = random(0, 4000);
//                switch (prefixChord){
//                    case 0:
//                        prefixChord = 0;
//                        sign1 = random(0,4);
//                        sign2 = random(0,4);
//                        Serial.println(randomNumber01);
//                       // Serial.print(randomNumber01);
//                        if (sign1 == 0){
//                            Serial.println('/');
//                            Serial1.print('/');
//                        }
//                        if (sign1 == 1){
//                            Serial.println('*');
//                            Serial1.print('*');
//                        }
//                        if (sign1 == 2){
//                            Serial.println('-');
//                            Serial1.print('-');
//                        }
//                        if (sign1 == 3){
//                            Serial.println('+');
//                            Serial1.print('+');
//                        }
//                        Serial.println(randomNumber02);
//                        Serial1.print(randomNumber02);
//                        if (sign2 == 0){
//                            Serial.println('/');
//                            Serial1.print('/');
//                        }
//                        if (sign2 == 1){
//                            Serial.println('*');
//                            Serial1.print('*');
//                        }
//                        if (sign2 == 2){
//                            Serial.println('-');
//                            Serial1.print('-');
//                        }
//                        if (sign2 == 3){
//                            Serial.println('+');
//                            Serial1.print('+');
//                        }
//                        Serial.println(randomNumber03);
//                        Serial1.print(randomNumber03);
//                        delay(5);
//                        Serial.println('\n');
//                        return 10;
//                    case 1:
//                        prefixChord = 0;
//                        Serial.println("0.");
//                        Serial1.print("0.");
//                        randomNumber04 = random(0, 10);
//                        Serial.println(randomNumber04);
//                        Serial1.print(randomNumber04);
//                        randomNumber04 = random(0, 10);
//                        Serial.println(randomNumber04);
//                        Serial1.print(randomNumber04);
//                        randomNumber04 = random(0, 10);
//                        Serial.println(randomNumber04);
//                        Serial1.print(randomNumber04);
//                        randomNumber04 = random(0, 10);
//                        Serial.println(randomNumber04);
//                        Serial1.print(randomNumber04);
//                        return 0;
//                    case 2:
//                        prefixChord = 0;
//                        return 0;
//                    case 3:
//                        prefixChord = 0;
//                        return 0;
//                    case 4:
//                        prefixChord = 0;
//                        Serial.println(randomNumber01);
//                        Serial1.print(randomNumber01);
//                        return 0;
//                }
        case 14:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('e');
        return 101;          // 101 → e

        }
        case 15:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('t');
        return 116;          // 116 → t

        }
//            case 16:
//                stickyCapsLock = LOW;
//                stickyNumlock = LOW;
//                switch (prefixChord){
//                    case 0:
//                        prefixChord = 2;
//                        return 0;       // Set the 'numlock' when only the near thumb key is pressed.
//
//                }
//            case 17:
//                stickyCapsLock = LOW;
//                stickyNumlock = LOW;
//                stickySpecialLock = LOW;
//                switch (prefixChord){
//                    case 0:
//                        prefixChord = 4;
//                        return 0;
//                    case 1:
//                        prefixChord = 4;
//                        return 0;
//                    case 2:
//                        prefixChord = 4;
//                        return 0;
//                    case 3:
//                        prefixChord = 4;
//                        return 0;
//                    case 4:
//                        prefixChord = 0;
//                        return 0;
        //             }
//            case 18:
//                stickyCapsLock = LOW;
//                stickyNumlock = LOW;
//                stickySpecialLock = LOW;
//                switch (prefixChord){
//                    case 0:
//                        prefixChord = 0;
//                        Serial.println(27);
//                        return 0x1B;
//                    case 1:
//                        prefixChord = 0;
//                        Serial.println(27);
//                        return 0x1B;
//                    case 2:
//                        prefixChord = 0;
//                        Serial.println(27);
//                        return 0x1B;
//                    case 3:
//                        prefixChord = 0;
//                        Serial.println(27);
//                        return 0x1B;
//                    case 4:
//                        prefixChord = 0;
//                        Serial.println(27);
//                        return 0x1B;
        }
//            case 19:
//                switch (prefixChord){
//                    case 0:
//                        prefixChord = 0;
//                        Serial.println(';');
//                        return 59;
//                    case 1:
//                        prefixChord = 0;
//                        Serial.println(':');
//                        return 58;
//                    case 2:
//                        prefixChord = 0;
//                        return 0;
//                    case 3:
//                        prefixChord = 0;
//                        return 0;
//                    case 4:
//                        prefixChord = 0;
//                        return 0;
//                }
//            case 20:
//                switch (prefixChord){
//                    case 0:
//                        prefixChord = 0;
//                        Serial.println(',');
//                        return 44;
//                    case 1:
//                        prefixChord = 0;
//                        Serial.println('<');
//                        return 60;
//                    case 2:
//                        prefixChord = 0;
//                        return 0;
//                    case 3:
//                        prefixChord = 0;
//                        return 0;
//                    case 4:
//                        prefixChord = 0;
//                        return 0;
//                }
//            case 21:
//                switch (prefixChord){
//                    case 0:
//                        prefixChord = 0;
//                        return 0;
//                    case 1:
//                        prefixChord = 0;
//                        return 0;
//                    case 2:
//                        prefixChord = 0;
//                        return 0;
//                    case 3:
//                        prefixChord = 0;
//                        return 0;
//                    case 4:
//                        prefixChord = 0;
//                        return 0;
//                }
//            case 22:
//                switch (prefixChord){
//                    case 0:
//                        prefixChord = 0;
//                        Serial.println('.');
//                        return 46;
//                    case 1:
//                        prefixChord = 0;
//                        Serial.println('>');
//                        return 62;
//                    case 2:
//                        prefixChord = 0;
//                        return 0;
//                    case 3:
//                        prefixChord = 0;
//                        return 0;
//                    case 4:
//                        prefixChord = 0;
//                        return 0;
//                }

        case 33:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('f');
        return 102;          // 102 → f

        }
        case 34:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('g');
        return 103;          // 103 → g

        }
        case 35:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('v');
        return 118;          // 118 → v

        }
        case 36:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('c');
        return 99;          // 99 → c

        }

        case 38:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('p');
        return 112;          // 112 → p

        }
        case 39:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('n');
        return 110;          // 110 → n

        }
        case 40:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('l');
        return 108;          // 108 → l (lowercase L)

        }
        case 41:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('x');
        return 120;          // 120 → x

        }
        case 42:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('j');
        return 106;          // 106 → j

        }
        case 43:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('q');
        return 113;          // 113 → q

        }
        case 44:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('m');
        return 109;          // 109 → m

        }
        case 45:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println(']');
        return 91;          // 91 → ]
        }
        case 46:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('a');
        return 97;          // 97 → a

        }
        case 47:
        switch (prefixChord){
        case 0:
        prefixChord = 0;
        Serial.println('o');
        return 111;          // 111 → o

        }
default:

        return 0;     // error
        }
        }
        void oledPowerSave() {
        powerSaveCurrentMillis = millis();
        if ( (powerSaveCurrentMillis - powerSavePreviousMillis) >= _PowerSaveInterval ) {
        bool changed = dc.setPowerSave(1); delay(3);
        shouldDrawToOled = false;
        if ( changed && getOperatingMode() == BLE ) Serial.println("[INFO] OLED PowerSave");       // Screen is off
        powerSavePreviousMillis = powerSaveCurrentMillis;
        }
        }
