//************************************************************
//   PixelStrip For Backlight - a sketch to backlight a 3D
//      print. A switch is included to move from one mode
//      to another.
//      Hardware: Arduino Nano, 330ohm resistor, momentary
//                contact switch, 8 light WS2812B Individually
//                Addressable 5050 RGB LED Strip Light
//************************************************************


#include <Adafruit_NeoPixel.h>
// Any unconnected pin, to try to generate a random seed
#define UNCONNECTED_PIN   2
#define PIN 5           // "D" data pin for candle strip
#define SWITCH_PIN 3    // data pin for switch, other side to ground
#define NUMPIXELS 8     // number of pixels in candle light strip

// The LED can be in only one of these states at any given time
#define BRIGHT                  0
#define UP                      1
#define DOWN                    2
#define DIM                     3
#define BRIGHT_HOLD             4
#define DIM_HOLD                5

// Percent chance the LED will suddenly fall to minimum brightness
#define INDEX_BOTTOM_PERCENT    10
// Absolute minimum red value (green value is a function of red's value)
#define INDEX_BOTTOM            128
// Minimum red value during "normal" flickering (not a dramatic change)
#define INDEX_MIN               192
// Maximum red value
#define INDEX_MAX               255

// Decreasing brightness will take place over a number of milliseconds in this range
#define DOWN_MIN_MSECS          20
#define DOWN_MAX_MSECS          250
// Increasing brightness will take place over a number of milliseconds in this range
#define UP_MIN_MSECS            20
#define UP_MAX_MSECS            250
// Percent chance the color will hold unchanged after brightening
#define BRIGHT_HOLD_PERCENT     20
// When holding after brightening, hold for a number of milliseconds in this range
#define BRIGHT_HOLD_MIN_MSECS   0
#define BRIGHT_HOLD_MAX_MSECS   100
// Percent chance the color will hold unchanged after dimming
#define DIM_HOLD_PERCENT        5
// When holding after dimming, hold for a number of milliseconds in this range
#define DIM_HOLD_MIN_MSECS      0
#define DIM_HOLD_MAX_MSECS      50

#define MINVAL(A,B)             (((A) < (B)) ? (A) : (B))
#define MAXVAL(A,B)             (((A) > (B)) ? (A) : (B))

byte state[NUMPIXELS];
unsigned long flicker_msecs[NUMPIXELS];
unsigned long flicker_start[NUMPIXELS];
byte index_start[NUMPIXELS];
byte index_end[NUMPIXELS];
byte color_shift[NUMPIXELS];
bool tint = false;

unsigned long time_pushed;
int light_style = 1;  // 0 = off, 1 = candle; 2 = color candle; 3 = solid color-blue;
          // 4 = solid color-purple; 5 = solid color-tea green; 5 = solid color-white

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(UNCONNECTED_PIN));
  // Set up switch pin
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  // Turn off the onboard red LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  strip.begin();
  strip.setBrightness(127);
  strip.show(); // Initialize all pixels to 'off'
  delay(50);
}

void SetStipColor(uint8_t red, uint8_t green, uint8_t blue) {
    for (int i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, red, green, blue);
    }  
}


void FlickerLights(unsigned long current_time) {
    for(int i=0;i<NUMPIXELS;i++){
      flicker (current_time, i);
    }  
}


void loop() {
  unsigned long current_time;
  current_time = millis();

  int j = 0;
  switch (getLightStyle(current_time)) {
    case 0:  {
    // all lights off
    SetStipColor(0,0,0);
    strip.show();
    break;
    }
    
    case 1: {
    // color candle flicker
    tint = false;
    FlickerLights(current_time);
    break;
    }
      
    case 2: {
    // candle flicker
    tint = true;
    FlickerLights(current_time);
    break;
    }

    case 3: {
    // solid pastels - purple
    SetStipColor(200,40,235);  //light purple      
    strip.show();
    break;
    }

    case 4: {
    // tea green
    SetStipColor(235,210,40);  //tea green
    strip.show();
    break;
    }

//    strip.setPixelColor(0,179,215,255);  //light blue
//    strip.setPixelColor(1,255,135,50);  //pastel yellow
//    strip.setPixelColor(2,255,128,102);  //warm pink
//    strip.setPixelColor(3,213,255,68);  //tea green
    // strip.setPixelColor(0,220,246,235);  //light blue
    // strip.setPixelColor(1,227,135,50);  //pastel yellow
    // strip.setPixelColor(2,255,128,102);  //warm pink
    // strip.setPixelColor(3,235,210,40);  //tea green
  }
}


int getLightStyle(unsigned long current_time) {
  // return the light style after trying
  // to read if a button push occurred
  if (digitalRead(SWITCH_PIN) == 0) {
    unsigned long delta = current_time - time_pushed;
    if(delta > 600) {
    time_pushed = millis();
    Serial.print(delta);
    Serial.print("  button pushed, light_style = ");
    light_style += 1;
    if (light_style>4) light_style = 0;
    Serial.println(light_style);
    }
  }
  return light_style;  
}


void flicker (unsigned long current_time, int iCandle) {
  // update iCandle brightness for flickering
  switch (state[iCandle])
    {
    case BRIGHT:
      flicker_msecs[iCandle] = random(DOWN_MAX_MSECS - DOWN_MIN_MSECS) + DOWN_MIN_MSECS;
      flicker_start[iCandle] = current_time;
      index_start[iCandle] = index_end[iCandle];
      if ((index_start[iCandle] > INDEX_BOTTOM) &&
          (random(100) < INDEX_BOTTOM_PERCENT))
        index_end[iCandle] = random(index_start[iCandle] - INDEX_BOTTOM) + INDEX_BOTTOM;
      else
        index_end[iCandle] = random(index_start[iCandle] - INDEX_MIN) + INDEX_MIN;

      state[iCandle] = DOWN;
      break;
    case DIM:
      flicker_msecs[iCandle] = random(UP_MAX_MSECS - UP_MIN_MSECS) + UP_MIN_MSECS;
      flicker_start[iCandle] = current_time;
      index_start[iCandle] = index_end[iCandle];
      index_end[iCandle] = random(INDEX_MAX - index_start[iCandle]) + INDEX_MIN;
      state[iCandle] = UP;
      break;
    case BRIGHT_HOLD:
    case DIM_HOLD:
      if (current_time >= (flicker_start[iCandle] + flicker_msecs[iCandle]))
        state[iCandle] = (state[iCandle] == BRIGHT_HOLD) ? BRIGHT : DIM;

      break;
    case UP:
    case DOWN:
      if (current_time < (flicker_start[iCandle] + flicker_msecs[iCandle]))
        set_color_type(index_start[iCandle] + ((index_end[iCandle] - index_start[iCandle]) * (((current_time - flicker_start[iCandle]) * 1.0) / flicker_msecs[iCandle])), iCandle);
      else
        {
        set_color_type(index_end[iCandle], iCandle);

        if (state[iCandle] == DOWN)
          {
          if (random(100) < DIM_HOLD_PERCENT)
            {
            flicker_start[iCandle] = current_time;
            flicker_msecs[iCandle] = random(DIM_HOLD_MAX_MSECS - DIM_HOLD_MIN_MSECS) + DIM_HOLD_MIN_MSECS;
            state[iCandle] = DIM_HOLD;
            }
          else
            state[iCandle] = DIM;
          }
        else
          {
          if (random(100) < BRIGHT_HOLD_PERCENT)
            {
            flicker_start[iCandle] = current_time;
            flicker_msecs[iCandle] = random(BRIGHT_HOLD_MAX_MSECS - BRIGHT_HOLD_MIN_MSECS) + BRIGHT_HOLD_MIN_MSECS;
            state[iCandle] = BRIGHT_HOLD;
            }
          else
            state[iCandle] = BRIGHT;
          }
        }

      break;
    }      
}

void set_color_type(byte index, int CandleValue)
  {
    index = MAXVAL(MINVAL(index, INDEX_MAX), INDEX_BOTTOM);
    float redTarget;
    float greenTarget;
    float blueTarget;
    switch (CandleValue)
    {
      case 0:  //blue
      case 7:
      case 1:
      case 6:
        redTarget = 1.0*153.0/255.0;
        greenTarget = 1.0*255.0/255.0;
        blueTarget = 255.0/255.0;
      break;
      
      // case 1:  //yellow
      // case 6:
      //   redTarget = 0.8*255.0/255.0;
      //   greenTarget = 255.0/255.0;
      //   blueTarget = 0.2*128.0/255.0;
      // break;    
      
      case 2:  //pink
      case 5:
        redTarget = 255.0/255.0;
        greenTarget = 1.0*153.0/255.0;
        blueTarget = 0.8*204.0/255.0;
      break;    
      
      case 3:  //green
      case 4:
        redTarget = 0.6*191.0/255.0;
        greenTarget = 255.0/255.0;
        blueTarget = 0.4*79.0/255.0;
      break;    
      
      default:  //normal
        redTarget = 1.0;
        greenTarget = 1.0;
        blueTarget = 1.0;
      break;
    }
    
    if (tint == false) {
        redTarget = 1.0;
        greenTarget = 1.0;
        blueTarget = 0.05;      
    }
    if (index >= INDEX_MIN)
      strip.setPixelColor(CandleValue, int(redTarget*index), int(greenTarget*(index * 3.0) / 8.0), int(blueTarget*200.0));
    else if (index < INDEX_MIN)
      strip.setPixelColor(CandleValue, int(redTarget*index), int(greenTarget*(index * 3.25) / 8.0), int(blueTarget*200.0));
    strip.show();
    return;
  }
