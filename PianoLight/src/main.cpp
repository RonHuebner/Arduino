//************************************************************
//   PianoMusicLightDimmer - a sketch to make a dimmer for
//      a COB strip that lights up my piano music holder.
//      
//      Hardware: Adafruit Trinket M0, 10Kohm resistor, momentary
//                contact switch, COB light strip (5v),
//                IRL540NPBF Infineon MOSFET
//************************************************************

// #include <Adafruit_DotStar.h>
// #include <Adafruit_I2CDevice.h>

#include <Arduino.h>

#define PIN_FOR_LED 2   // data pin for LED
#define SWITCH_PIN 3    // data pin for switch, other side to ground
#define NUMPIXELS 1 // Number of LEDs in strip

int brightness = 255;
int fadeSpeed = 5;
bool brighten = true;
unsigned long time_pushed = 0;

void blinkLight() {
  analogWrite(PIN_FOR_LED,0);
  delay(500);
  analogWrite(PIN_FOR_LED,brightness);
}

// put function definitions here:
bool buttonPush(unsigned long current_time) {
  bool buttonState = false;
  // read if a button push occurred
  if (digitalRead(SWITCH_PIN) == LOW) {
    unsigned long delta = current_time - time_pushed;
    if(delta > 10) {
      buttonState=true;
      time_pushed = millis();
      Serial.print(delta);
      Serial.print(" button pushed, brightness = ");
      if (brighten==true) {
        brightness += fadeSpeed;
        if (brightness>255) {
          brightness = 255;
          brighten = false;
          blinkLight();
        } 
      }
      else {
        brightness -= fadeSpeed;
        if (brightness<0) {
          brightness = 0;
          brighten = true;
        }
      }
      Serial.println(brightness);
    }
  }
  return buttonState;  
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PIN_FOR_LED, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(13, OUTPUT);  //red led on board
  Serial.println("starting program");
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long current_time;
  current_time = millis();
  if (buttonPush(current_time))  {
    /* adjust light brightness */
    analogWrite(PIN_FOR_LED,brightness);
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  else {
    digitalWrite(13, LOW);   // turn the LED off
  }
  delay(100);
}
