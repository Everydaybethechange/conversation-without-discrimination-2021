#include <SoftwareSerial.h>
// Define the data transmit/receive pins in Arduino

//if tx->2,rx->3, then mySerial(2,3)
//if tx->3,rx->2, then mySerial(3,2)
//EDIT WHAT IS BELOW THIS
SoftwareSerial mySerial(RxD, TxD); // (RX, TX) for Bluetooth
void setup() {
mySerial.begin(9600); // For Bluetooth
Serial.begin(9600); // For the IDE monitor Tools -> Serial Monitor
// Any code that you want to run once....
}
void loop() {
// put your main code here, to run repeatedly:
boolean isValidInput; do { byte c; // get the next character from the bluetooth serial port
while ( !mySerial.available() ) ; // LOOP...
c = mySerial.read(); // Execute the option based on the character recieved
Serial.print(c); // Print the character received to the IDE serial monitor
switch ( c ) {
case 'a': // You've entered a
// Do the code you need when 'a' is received.....
mySerial.println( "You've entered an 'a'" );
isValidInput = true;
break;
case 'b': // You've entered b
// Do the code you need when 'a' is received.....
mySerial.println( "You've entered an 'b'" );
isValidInput = true;
break;
default:
// Do the code you need when any other character is received.....
mySerial.println( "Please enter 'a' or 'b'" );
isValidInput = false;
break;
}
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          
                          --------------------------------
                            #include <SoftwareSerial.h>
// Define the data transmit/receive pins in Arduino
        #include <Arduino.h>
        #include <U8g2lib.h>

        #ifdef U8X8_HAVE_HW_SPI
        #include <SPI.h>
        #endif
        #ifdef U8X8_HAVE_HW_I2C
        #include <Wire.h>
        #endif
String msg;
        U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED

//if tx->2,rx->3, then mySerial(2,3)
//if tx->3,rx->2, then mySerial(3,2)
//EDIT WHAT IS BELOW THIS
        SoftwareSerial mySerial(RxD, TxD); // (RX, TX) for Bluetooth
        void setup() {
        u8g2.begin();
        mySerial.begin(9600); // For Bluetooth
        Serial.begin(9600); // For the IDE monitor Tools -> Serial Monitor
// Any code that you want to run once....
        }
        void loop() {
// put your main code here, to run repeatedly:
        if (Serial.available.0) {
        msg = Serial.readString();
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,msg);	// write something to the internal memory
        u8g2.sendBuffer();					// transfer internal memory to the display
        delay(1000);
        }
        boolean isValidInput; do { byte c; // get the next character from the bluetooth serial port
        while ( !mySerial.available() ) ; // LOOP...
        c = mySerial.read(); // Execute the option based on the character recieved
        Serial.print(c); // Print the character received to the IDE serial monitor
        switch ( c ) {
        case 'a': // You've entered a
// Do the code you need when 'a' is received.....
        mySerial.println( "You've entered an 'a'" );
        isValidInput = true;
        break;
        case 'b': // You've entered b
// Do the code you need when 'a' is received.....
        mySerial.println( "You've entered an 'b'" );
        isValidInput = true;
        break;
default:
// Do the code you need when any other character is received.....
        mySerial.println( "Not valid bro" );
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,"Not valid bro");	// write something to the internal memory
        u8g2.sendBuffer();					// transfer internal memory to the display
        delay(1000);
        isValidInput = false;
        break;
        }
        } while ( isValidInput == true ); // Repeat the loop
        }
} while ( isValidInput == true ); // Repeat the loop
}
