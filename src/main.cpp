#include "ESP32RotaryEncoder.h"
#include "Arduino.h"
 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// OLED
//const int8_t OLED_SCL  = 22;
//const int8_t OLED_SDA  = 21;

// Relay
const int8_t REPLAY_PIN  = 19;
 
//const int8_t  DO_ENCODER_VCC = D2; 
// Only needed if you're using a GPIO pin to supply the 3.3v reference
const int8_t DI_ENCODER_SW  = 27; // Pushbutton, if your rotary encoder has it
const uint8_t DI_ENCODER_A   = 25; // Might be labeled CLK
const uint8_t DI_ENCODER_B   = 26; // Might be labeled DT

RotaryEncoder rotaryEncoder( DI_ENCODER_A, DI_ENCODER_B, DI_ENCODER_SW );

bool RELAY_STATE = false;
int INITIAL_ACTIVE_DURATION_MIN = 30;
int INITIAL_MAX_DURATION_MIN = 120;
int RUN_MAX_DURATION_MIN = 1;
int RUN_ACTIVE_DURATION_MIN = 1;


void drawDisplay( int remaining, int total);
void drawDisplayText( String toDisplay, int total);

void knobCallback( long value )
{
    // This gets executed every time the knob is turned
  RUN_MAX_DURATION_MIN=value;
  drawDisplay(value,value);
}

void buttonCallback( unsigned long duration )
{
    // This gets executed every time the pushbutton is pressed
    RELAY_STATE = !RELAY_STATE;
    if ( RELAY_STATE == true) {
      RUN_ACTIVE_DURATION_MIN = RUN_MAX_DURATION_MIN;
      digitalWrite(REPLAY_PIN,HIGH);
      drawDisplay(RUN_ACTIVE_DURATION_MIN,RUN_ACTIVE_DURATION_MIN);
      drawDisplayText("Start",RUN_ACTIVE_DURATION_MIN);
      drawDisplay(RUN_ACTIVE_DURATION_MIN,RUN_ACTIVE_DURATION_MIN);

    } else {
      digitalWrite(REPLAY_PIN,LOW);
      drawDisplayText("Cancel",RUN_ACTIVE_DURATION_MIN);
      drawDisplayText("Reset",INITIAL_ACTIVE_DURATION_MIN);
      drawDisplay(INITIAL_ACTIVE_DURATION_MIN,INITIAL_ACTIVE_DURATION_MIN);
    }

}


void setup()
{
    Serial.begin( 115200 );

    // This tells the library that the encoder has its own pull-up resistors
    rotaryEncoder.setEncoderType( EncoderType::HAS_PULLUP );

    // Range of values to be returned by the encoder: minimum is 1, maximum is 10
    // The third argument specifies whether turning past the minimum/maximum will
    // wrap around to the other side:
    //  - true  = turn past 10, wrap to 1; turn past 1, wrap to 10
    //  - false = turn past 10, stay on 10; turn past 1, stay on 1
    rotaryEncoder.setBoundaries( INITIAL_ACTIVE_DURATION_MIN, INITIAL_MAX_DURATION_MIN, false );

    // The function specified here will be called every time the knob is turned
    // and the current value will be passed to it
    rotaryEncoder.onTurned( &knobCallback );

    // The function specified here will be called every time the button is pushed and
    // the duration (in milliseconds) that the button was down will be passed to it
    rotaryEncoder.onPressed( &buttonCallback );

    // This is where the inputs are configured and the interrupts get attached
    rotaryEncoder.begin();

    pinMode(REPLAY_PIN, OUTPUT);


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000);
  display.clearDisplay();
  drawDisplay(INITIAL_ACTIVE_DURATION_MIN,INITIAL_ACTIVE_DURATION_MIN);

}

 
void loop()
{
  if ( RELAY_STATE == true && RUN_ACTIVE_DURATION_MIN > 0) {
    drawDisplay(RUN_ACTIVE_DURATION_MIN,RUN_MAX_DURATION_MIN);
    delay(60000);
    RUN_ACTIVE_DURATION_MIN--;
  } else if (RELAY_STATE == true && RUN_ACTIVE_DURATION_MIN <= 0) {
    digitalWrite(REPLAY_PIN,LOW);
    drawDisplayText("Finished",RUN_MAX_DURATION_MIN);
    drawDisplayText("Resetting",INITIAL_ACTIVE_DURATION_MIN);
    drawDisplay(INITIAL_ACTIVE_DURATION_MIN,INITIAL_ACTIVE_DURATION_MIN);
  }

}



void drawDisplay( int remaining, int total) {
  display.clearDisplay();
  display.setTextSize(2.5); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  String textstate = (RELAY_STATE == true) ? "on" : "off";
  display.printf("%s\n %dm/%dm",textstate,remaining, total);
  display.display();      // Show initial text
}

void drawDisplayText( String toDisplay, int total) {
  display.clearDisplay();
  display.setTextSize(2.5); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.printf("%s\n %dm",toDisplay, total);
  display.display();      // Show initial text
  delay(3000); // 10 seconds
}