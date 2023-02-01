// Lighting controller for dolphin art light, it generates the water
//  effect as well as controls the spot lights. The lights are 
//  controlled by a ESP32 controller with persistant settings and OTA updates.
//  Control hardware: HiLetgo ESP-WROOM-32 ESP-32S Development Board
//  Lights: BTF-LIGHTING SK6812RGBW(WS2812B RGBW) LED Chips
//  Web page HTML code from control_page.h as const char HTML[] PROGMEM
//  Web page conversion: http://tomeko.net/online_tools/cpp_text_escape.php?lang=en 
//  OTA update page: server IP/update, select precompiled BIN file created with: 
//  (Arduino IDE/Sketch/Export compiled binary)

// ESP32 Modules: Set environment to board "Nano32" for HiLetGo and 
//                "Adafruit ESP32 Feather" for Adafruit feather
//                 Add boards from URL: https://dl.espressif.com/dl/package_esp32_index.json
//
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
//   pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
//   and minimize distance between Arduino and first pixel.  Avoid connecting
//   on a live circuit...if you must, connect GND first.#include <Arduino.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <Preferences.h>
#include "control_page.h"

#define Pixels 60  // number of neopixels in water effect
const uint16_t StripLength = 62; // The number of pixels in your strip
const uint8_t PixelPin = 14;  // data pin for neopixels IO14

#define WIFI_SSID "E6"
#define WIFI_PW "hightime9955B"
unsigned long previousMillis = 0;
unsigned long interval = 30000;
Preferences preferences;  //flash storage for retaining values through reboot
NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> strip(StripLength, PixelPin);
// NeoPixel animation time management objects,
// one for each water pixel and one to control the loop
NeoPixelAnimator animations(Pixels + 1, NEO_CENTISECONDS);
// NOTES ON ANIMATION 
// Create with enough animations to have one per pixel, depending on the animation
// effect, you may need more or less. Since the normal animation time range is only 
// about 65 seconds, by passing timescale value to the NeoPixelAnimator constructor 
// we can increase the time range, but this increases the time between animation updates.   
// NEO_CENTISECONDS will update the animations every 100th of a second rather than the default
// of a 1000th of a second, but the time range will now extend from about 65 seconds to about
// 10.9 minutes.  The values passed to StartAnimations are now in centiseconds.
//
// Possible values from 1 to 32768, and there some helpful constants defined as...
// NEO_MILLISECONDS        1    // ~65 seconds max duration, ms updates
// NEO_CENTISECONDS       10    // ~10.9 minutes max duration, centisecond updates
// NEO_DECISECONDS       100    // ~1.8 hours max duration, decisecond updates
// NEO_SECONDS          1000    // ~18.2 hours max duration, second updates
// NEO_DECASECONDS     10000    // ~7.5 days, 10 second updates
//

AnimEaseFunction moveEase = NeoEase::Linear;

RgbwColor colorStates[Pixels];

// The server object that will listen on port 80 and respond to requests from web browsers
AsyncWebServer server(80);

// Variables to store the most recent color sent to us by the client (i.e a web browser)
int red = 0;
int green = 0;
int blue = 20;
int white = 50;
int onOff = 1;
int ease = 1;
int bright = 100;
uint16_t animationTime = 75;  // duration of the animation, centiseconds
  
bool flip;

int nextColors[Pixels];

int myColors[] = {0,0,2,0,0,0,0,3,0,0,
                  0,3,0,0,0,0,2,0,0,0,
                  0,0,0,2,0,0,0,3,0,0,
                  0,0,0,0,3,0,0,0,0,2,
                  0,0,0,3,0,0,0,2,0,0,
                  0,2,0,0,0,0,3,0,0,0};  // 60 lights sparse

struct MyAnimationState
{
    RgbwColor StartingColor;  // the color the animation starts at
    RgbwColor EndingColor; // the color the animation will end at
    AnimEaseFunction Easeing; // the acceleration curve it will use 
};

MyAnimationState animationState[StripLength];
// one entry per pixel to match the animation timing manager

RgbwColor GetColors(int colorValue) {
  float brightFactor;
  float red;
  float green;
  float blue;
  float white;
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
      green = 20;
      blue = 80;
      white = 0;          
      break;    
    case 4:    
      //blue red
      red = 05;
      green = 0;
      blue = 95;
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
  brightFactor = 2.5 * (float)bright/100.0;
  red = red * brightFactor;
  green = green * brightFactor;
  blue = blue * brightFactor;
  white = white * brightFactor;
  return RgbwColor(red, green, blue, white); 
}

void FindNextColors() {
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
}

void AnimationUpdate(const AnimationParam& param) {
    // apply the movement animation curve
    float progress = moveEase(param.progress);

    // this gets called for each animation on every time step
    // progress will start at 0.0 and end at 1.0
    // we use the blend function on the RgbColor to mix
    // color based on the progress given to us in the animation
    RgbwColor updatedColor = RgbwColor::LinearBlend(
        animationState[param.index].StartingColor,
        animationState[param.index].EndingColor,
        progress);
    // apply the color to the strip
    strip.SetPixelColor(param.index, updatedColor);
    if(param.index==2 && param.progress==1.0 ) {
      //Serial.print("Progress =");
      //Serial.println(param.progress);
    }
}

AnimEaseFunction EaseCode(int code) {
  switch (code) {
    case 2:
      return NeoEase::QuadraticInOut;
      break;
    case 3:
      return NeoEase::CubicInOut;
      break;    
    case 4:
      return NeoEase::QuarticInOut;
      break;    
    case 5:
      return NeoEase::QuinticInOut;
      break;    
    case 6:
      return NeoEase::SinusoidalInOut;
      break;    
    case 7:
      return NeoEase::ExponentialInOut;
      break;    
    case 8:
      return NeoEase::CircularInOut;
      break;    
    default:
      return NeoEase::Linear;
      break; 
  }   
}

void LoopAnimUpdate(const AnimationParam& param) {
    // wait for this animation to complete,
    // we are using it as a timer of sorts for the entire set of pixels
    if (param.state == AnimationState_Completed)
    {
      // set up the pixel colors
      FindNextColors();
      // slosh the water direction       
      int slosh = random(19);
      //Serial.print( "slosh = ");
      //Serial.println(slosh);      
      if (slosh >= 15) {
        flip = !flip;  // change light flow direction
        //Serial.print( "*********flip just changed = ");
        //Serial.println(flip);
      }
      // update the animations for each water pixel
      SetupAnimationSet();  
      
      // done, time to update all of the pixel colors
      animations.RestartAnimation(param.index);

      // shift the water pixel colors
      for (int i=0; i < Pixels; i++) {
        myColors[i] = nextColors[i];
      }

   }
}

void InitWater() {
  Serial.println("InitWater starts");
  for (int i=0; i < Pixels; i++) {   
    colorStates[i] = GetColors(myColors[i]);       
    strip.SetPixelColor(i, colorStates[i]);
  }
}

void SetupAnimationSet() {
    //Serial.println("SetupAnimationSet starts");
    // setup the animations for each pixel
    for (uint16_t pix = 0; pix < Pixels; pix++)
    {
      // starting state is the starting color set myColors
      animationState[pix].StartingColor = GetColors(myColors[pix]);
      // end state is color of the next pixel
      animationState[pix].EndingColor = GetColors(nextColors[pix]);       
      // use the specific curve
      animationState[pix].Easeing = moveEase;
      // now use the animation states we just created and start the animation
      // which will continue to run and call the update function until it completes
      animations.StartAnimation(pix, animationTime, AnimationUpdate); 
    }
}

void ConnectToWifi() {
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PW);

  Serial.println("Connecting to wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  
  Serial.println("Connected to wifi!");

  // Makes it so you can find the esp32 on your network by visiting DolphinLight.local/ in your browser rather than an ip address
  MDNS.begin("DolphinLight");
  
  Serial.println(WiFi.localIP());
}

void PrepareWiFiServer() {
  ConnectToWifi();

  // On a GET request, respond with the HTML string found in control_page.h
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", HTML);
  });

  // On a POST request, grab the values that were sent and save them
  server.on("/", HTTP_POST, [](AsyncWebServerRequest * request) {
    AsyncWebParameter *onoffParam = request->getParam("onoff", true);
    AsyncWebParameter *redParam = request->getParam("red", true);
    AsyncWebParameter *greenParam = request->getParam("green", true);
    AsyncWebParameter *blueParam = request->getParam("blue", true);
    AsyncWebParameter *whiteParam = request->getParam("white", true);
    AsyncWebParameter *neoEaseParam = request->getParam("NeoEase", true); 
    AsyncWebParameter *brightParam = request->getParam("bright", true);
    AsyncWebParameter *timeParam = request->getParam("aniTime", true);   
    red = redParam->value().toInt();
        Serial.print("red="); Serial.println(red);
    green = greenParam->value().toInt();
        Serial.print("green="); Serial.println(green);
    blue = blueParam->value().toInt();
        Serial.print("blue="); Serial.println(blue);
    white = whiteParam->value().toInt();
        Serial.print("white="); Serial.println(white);
    onOff = onoffParam->value().toInt();
        Serial.print("onOff="); Serial.println(onOff);
    ease = neoEaseParam->value().toInt();
        Serial.print("ease="); Serial.println(ease); 
    bright = brightParam->value().toInt();
        Serial.print("bright="); Serial.println(bright);  
    animationTime = timeParam->value().toInt();
        Serial.print("animationTime="); Serial.println(animationTime); 

    Serial.println("Update from web:");
    Serial.print("Spot colors:  Red="); Serial.print(red);Serial.print("  Green="); Serial.print(green);
    Serial.print("   Blue="); Serial.print(blue); Serial.print("   White="); Serial.println(white);
    Serial.print("OnOff code ="); Serial.println(onOff);    
    Serial.print("Ease code ="); Serial.println(ease);
    Serial.print("Bright ="); Serial.println(bright);
    Serial.print("Animation time ="); Serial.println(animationTime);

    // save new parameters
    SavePreferences();
    // update lights
    SetUpLighting();
  });

  // Start the server
  server.begin();
}

void SetUpLighting() {
  // set up water effect lights
  SetupAnimationSet();  
  // initialize spot lights, last two lights on string 
  strip.SetPixelColor(Pixels + 0, RgbwColor(red, green, blue, white));
  strip.SetPixelColor(Pixels + 1, RgbwColor(red, green, blue, white)); 
 
  if(onOff == 0) {  
    strip.ClearTo(RgbwColor(0,0,0,0));
  }
  else {
    animations.UpdateAnimations();
  }
  strip.Show();  
}

void SavePreferences() {
  // save current state to flash memory
  preferences.putInt("red", red);
  preferences.putInt("green", green);
  preferences.putInt("blue", blue);
  preferences.putInt("white", white);
  preferences.putInt("onOff", onOff);
  preferences.putInt("ease", ease);
  preferences.putInt("bright", bright);
  preferences.putInt("time", animationTime);
  Serial.println("State saved to flash memory");
}

void RecallPreferences() {
  // get preferences from flash memory to restore last state
  red = preferences.getInt("red", 0);
  green = preferences.getInt("green", 0);
  blue = preferences.getInt("blue", 20);
  white = preferences.getInt("white", 50);
  onOff = preferences.getInt("onOff", 1);
  ease = preferences.getInt("ease", 1);
  bright = preferences.getInt("bright", 100);
  animationTime = preferences.getInt("time", 75);  
  Serial.println("State recalled from flash memory");
}


void setup() {
  // Setup Serial Monitor
  Serial.begin(9600);
  Serial.println("initialize");

  // Open Preferences with PrefStore namespace. We will open storage in
  // RW-mode (second parameter has to be false).
  // Note: Namespace name is limited to 15 chars.
  preferences.begin("PrefStore", false);
  // Get the counter value, if the key does not exist, return a default value of 0
  // Note: Key name is limited to 15 chars.
  unsigned int counter = preferences.getUInt("counter", 0);
  // Increase counter by 1
  counter++;
  // Print the counter to Serial Monitor
  Serial.printf("Current counter value: %u\n", counter);
  // Store the counter to the Preferences
  preferences.putUInt("counter", counter);
  // get last state
  RecallPreferences();

  uint16_t i;
  strip.Begin();
  strip.Show(); // Initialize all pixels to 'off'
  flip = false;
  InitWater();  //initialize water effect pixel color state arrays
  SetupAnimationSet();  // setup the animation objects for water pixels
  // setup the master loop timing object, last object
  animations.StartAnimation(Pixels, animationTime, LoopAnimUpdate);
  
  // initialize spot lights, last two lights on string 
  strip.SetPixelColor(Pixels + 0, RgbwColor(red, green, blue, white));
  strip.SetPixelColor(Pixels + 1, RgbwColor(red, green, blue, white));

  // connect to WiFi and start server
  PrepareWiFiServer();

  // Start ElegantOTA to support OTA updates
  AsyncElegantOTA.begin(&server);
}



void loop () {

  delay(10);
}
