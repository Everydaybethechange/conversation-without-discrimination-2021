//
//
// ------------------------------------------------------------
//
//ONLY
//DISPLAY
//CODE
//?/?/
// --------------------------------------------------------




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
        #include<SoftwareSerial.h>
        SoftwareSerial Serial(2,3);


        /* -------------- Variables -------------- */

//pick your display below
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


        const char* ble_name = "Arduino Smartglass";
        bool isDeviceConnected = false;



// Variables changeable during BLE connect
        int _MarginX = 0;
        int _MarginY = 0;




        int _PowerSaveInterval = 8000;


        float _BatteryVoltageFlat       = 3.1f; // In my case 3.1V from ADC means ~3.6V from battery ( ADC in ESP is not linear! so we cant calculate exact value of battery just estimate!)

        /* --- Operating Mode --- */
        int iOperatingMode = 0;        // 0 - BLE, 1 - WEB_UPDATE, default = BLE

        int getOperatingMode() {
        iOperatingMode = 0;
        return 0; //hardcode always to BLE
        }

        void setOperatingMode(int om, bool restart) {
        if (iOperatingMode == om) return;       // if already set

        iOperatingMode = om;


        }

/* --- Button Class --- */
//[deleted]

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
        setFontSize(4); drawString(0, 22, "your msg");

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
        setFontSize(8 ); drawString(13, 9, "your msg");

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
//[deleted]



    /*
This part from originalinstructjava.ino
*/





        DisplayClass dc;



        void setup(){


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


        Serial.println("##[INFO] OperatingMode: BL");



        // Info OLED
        delay(100);
        dc.setFontSize(7); dc.drawString(10, 10, "Ready. Mode:" ); dc.drawString(10, 20,"BL" ); dc.sendBuffer(); powerSavePreviousMillis = millis();


        }

        void loop(){
        if (Serial.available() > 0){ // Check if there is data coming
        dc.drawString(10, 10, Serial.readString() );
        }




        }




 

//  testtest
//        int customPower(int 2, int 0){
//        int functionResult = 1;
//        for (int i = 0; i < 0; i++){
//        functionResult = functionResult * 2;
//        }
//        return functionResult;
//        }


          
