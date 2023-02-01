#include <Adafruit_NeoPixel.h>

#define PIN 6    // data pin for neopixels
#define Pixels 60  // number of neopixels in water effect
#define StripLength  62  // total number of lights, both spots and water effect
// TINYDUINO: set environment to board "Arduino Pro or Pro Mini" and 
//            processor to "ATmega328P (3.3V, 8 MHz)"
// Nano by Elegoo with micro USB set to UNO
// Nano by Makefun with micro USB set to Nano, ATMega328P (new bootloader not old)


// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(StripLength, PIN, NEO_GRBW + NEO_KHZ800);
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(Pixels, PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(4, 7, NEO_GRB + NEO_KHZ800);
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.


float redStates[Pixels];
float blueStates[Pixels];
float greenStates[Pixels];
float whiteStates[Pixels];
float red;
float green;
float blue;
float white;
bool flip;

//int myColors[] = {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//int myColors[] = {3,2,2,3,2,4,3,2,2,3,2,4,3,2,2,3,2,4,3,2};
//int myColors[] = {3,2,2,3,2,4,3,2,2,3,2,4,3,2,2,3,2,4,3,2,3,2,2,3,2,4,3,2,2,3,2,4,3,2,2,3,2,4,3,2};
//int myColors[] = {3,0,2,3,2,4,0,0,0,3,2,3,0,0,0,2,3,4,0,0,0,3,4,0,0,3,2,2,3,2,4,3,2,0,0,4,3,0,3,2};
int myColors[] = {0,0,2,3,2,0,2,3,4,0,
                  0,2,3,4,0,2,3,2,0,0,
                  0,2,3,2,0,0,2,3,4,0,
                  0,0,2,3,4,0,0,2,3,2,
                  0,0,2,3,2,0,0,2,3,4,
                  0,2,3,4,0,0,2,3,2,0};  // 60 lights (3 x 20)
  
void setup() {
  // Setup Serial Monitor
  Serial.begin(9600);
  Serial.println("initialize");      

  Serial.println("theaterChase");
  delay(500);
  uint16_t i;
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  theaterChase(strip, strip.Color(10,30,0,20), 50); // Pink
  strip.begin();
  strip.clear();
  strip.show(); // Initialize all pixels to 'off'
  
  Serial.println("zero state arrays");
  for(uint16_t k = 0; k < Pixels; k++) {
    setColors(i,0);
  }
  flip = false;
  initWater();

  // initialize spot lights, last two lights on string
  strip.setPixelColor(Pixels + 0, 20, 20, 0, 255);
  strip.setPixelColor(Pixels + 1, 20, 20, 0, 255);
  strip.show();
}


void loop () {
    waterEffect();
    //glint();
}

void initWater() {
  Serial.println("initWater starts");
  for (int i=0; i < Pixels; i++) {   
    setColors(i,myColors[i]);       
    strip.setPixelColor(i, redStates[i], greenStates[i], blueStates[i], whiteStates[i]);
  }
}

void glint() {
  //check if glint will happen
  int i = random(100);
  if (i > 2) return;
  
  //pick a random light
  i = random(Pixels);
  //flash white
  strip.setPixelColor(i, redStates[i], greenStates[i], blueStates[i], 100);         
  //show updated color, delay, and iterate to next color shift
  strip.show();
  delay(40);
  strip.setPixelColor(i, redStates[i], greenStates[i], blueStates[i], whiteStates[i]);
  strip.show();    
}

void getColors(int colorValue) {
  
  float scale = 1.0;
  switch (colorValue) {
    case 1:
      // gold
      red = 50;
      green = 50;
      blue = 00;
      white = 00;        
      break;
    case 2:    
      //blue
      red = 0;
      green = 0;
      blue = 100;
      white = 0;          
      break;
    case 3:    
      //blue green
      red = 00;
      green = 60;
      blue = 40;
      white = 0;          
      break;    
    case 4:    
      //blue red
      red = 10;
      green = 0;
      blue = 90;
      white = 0;          
      break;
    default:    
      //no color
      red = 0;
      green = 0;
      blue = 0;
      white = 0;          
      break;
  } 
  red = red * scale;
  green = green * scale;
  blue = blue * scale;
  white = white * scale; 
}

void setColors(int i, int colorValue) {
  getColors(colorValue);
  redStates[i] = red;
  greenStates[i] = green;
  blueStates[i] = blue;
  whiteStates[i] = white;  
}

void waterEffect() {
  int nextColors[Pixels];
  if (flip) {  //next is behind
    for (int i=1; i < Pixels; i++) {
      nextColors[i] = myColors[i-1];
    }
    nextColors[0] = myColors[Pixels-1];
  } else {  //next is ahead
    for (int i=0; i < Pixels-1; i++) {
      nextColors[i] = myColors[i+1];
    }
    nextColors[Pixels-1] = myColors[0];        
  }
  float r0;
  float g0;
  float b0;
  float w0;
  float r1;
  float g1;
  float b1;
  float w1;
  int steps = 20;
  Serial.println("waterEffect starts");
  Serial.print("flip = "); Serial.println(flip);    
  for (int j=0; j <= steps; j++) {  
    //shift color to next pixel by increments
    float fade = (float)j / (float)steps;
    Serial.print("j="); Serial.print(j); Serial.print("  fade="); Serial.println(fade);
    for (int i=0; i < Pixels; i++) {
      //iterate through each pixel and update color
      getColors(myColors[i]);
      r0 = red;
      g0 = green;
      b0 = blue;
      w0 = white;
      getColors(nextColors[i]);
      r1 = red;
      g1 = green;
      b1 = blue;
      w1 = white; 
      redStates[i] = (r0 * (1.0 - fade)) + (r1 * fade) ;
      greenStates[i] = (g0 * (1.0 - fade)) + (g1 * fade) ;
      blueStates[i] = (b0 * (1.0 - fade)) + (b1 * fade) ;
      whiteStates[i] = (w0 * (1.0 - fade)) + (w1 * fade) ;
      
//  Serial.print("*** myColors = ");
//  Serial.print(myColors[i]);
//  Serial.print(" nextColors = ");
//  Serial.print(nextColors[i]);   
//  Serial.print(" b0 = ");
//  Serial.print(b0);
//  Serial.print(" b1 = ");
//  Serial.print(b1);
//  Serial.print(" blueStates[");
//  Serial.print(i);  
//  Serial.print("] = ");
//  Serial.println(blueStates[i]);
      strip.setPixelColor(i, redStates[i], greenStates[i], blueStates[i], whiteStates[i]);         
    }
    //show updated color, delay, and iterate to next color shift
    strip.show();
    delay(10);  
  }

  // update to current color situation
  for (int j=0; j < Pixels; j++) { 
    myColors[j] = nextColors[j];
  }    

  delay(50);
  int slosh = random(10);
  if (slosh >8) {
    flip = !flip;  // change light switch direction
  }
   
}

void redWhiteBlueLights(uint16_t i) {
  if (redStates[i] < 1 && whiteStates[i] < 1 && greenStates[i] < 1 && blueStates[i] < 1) {
    int j = random(10);
    if (j >= 3) {
      redStates[i] = 150;
      greenStates[i] = 0;
      blueStates[i] = 0;
      whiteStates[i] = 0;          
    }
    else {
      redStates[i] = 0;
      greenStates[i] = 0;
      blueStates[i] = 150;          
      whiteStates[i] = 0;   
    }
    if (j >= 7) {
      redStates[i] = 0;
      greenStates[i] = 0;
      blueStates[i] = 0;
      whiteStates[i] = 250;               
    }
  }
}  


//Theatre-style crawling lights.
void theaterChase(Adafruit_NeoPixel myStrip, uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < myStrip.numPixels(); i=i+3) {
        myStrip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      myStrip.show();

      delay(wait);

      for (int i=0; i < myStrip.numPixels(); i=i+3) {
        myStrip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}
