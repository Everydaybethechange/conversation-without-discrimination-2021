
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

// Inputs. Buttons may be addressed by name but the program expects all buttons after the pinky
// to be numbered sequentially.

//D5 to D9 , or pins 8 to 12    REAL BUTTONS - those are the working typing buttons
int fifthbutton = 5;
int fourthbuttonSIX = 6;
int middleButton = 7;
int secondButtonEIGHT = 8;
int firstbuttonNINEnearT = 9;


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



                                                acquiringPresses = checkButtonArray();
                                                    if (acquiringPresses){     // [pers] if buttons pressed
                                                        delay(debounceDelay);                           // Instead of a true software debounce this will wait a moment until the first button press has settled.
                                                        typingChord();                      // Wait and see which keys are touched. When they are all released, copy them to latching buttons
                                                        //   updateShiftKeys();          // Change the prefixChord value if any of the 'locks' are set. Example, Num Lock or Caps Lock.
                                                        sendKeyPress();                     // Using the buttons pressed during the typingChord function determine how to handle the chord.
                                                        delay(debounceDelay);                           // The other half of the software "debounce"
                                                            for (int i = 0; i < 5; i++){                                        //         {}_}_{}// Once a keypress has been sent, make all latching buttons zero.
                                                            latchingButtons[i] = LOW;
                                                            }
                                                        chordValue = 0;
                                                    }
                                                  //print to OLED



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
            Serial.println(keySwitch(chordValue));   //tHIS IS WHERE CHARACTER IS SENT TO THE BL
            //^ VERY IMPORTANT
            // Serial.write(keySwitch(chordValue));
        }

        int customPower(int functionBase, int functionExponent){
                int functionResult = 1;
                for (int i = 0; i < functionExponent; i++){
                functionResult = functionResult * functionBase;
                }
                return functionResult;
        }


//  testtest
//        int customPower(int 2, int 0){
//        int functionResult = 1;
//        for (int i = 0; i < 0; i++){
//        functionResult = functionResult * 2;
//        }
//        return functionResult;
//        }


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
                prefixChord=0;
                Serial.println('y');
                return 121;          // 121 is equivalent to the letter 'y'
                }
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
                //       }
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

