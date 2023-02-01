// Lighting controller for front wall lights
//  Customized by WiFi controller with persistant settings
//  Control hardware: HiLetgo ESP-WROOM-32 ESP-32S Development Board
//  Lights: Strip of BTF-LIGHTING RGBWW Warm White SK6812 60 Pixels/m
//  Web page HTML code from control_page.h as const char HTML[] PROGMEM
//  Web page conversion: http://tomeko.net/online_tools/cpp_text_escape.php?lang=en 
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <Preferences.h>
#include "control_page.h"

// ESP32 Modules: Set environment to board "Nano32" for HiLetGo and 
//                "Adafruit ESP32 Feather" for Adafruit feather
//                 Add boards from URL: https://dl.espressif.com/dl/package_esp32_index.json
//
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define Pixels 162  // number of neopixels
const uint8_t PixelPin = 14;  // data pin for neopixels IO14

#define WIFI_SSID "E6"
#define WIFI_PW "hightime9955B"
unsigned long previousMillis = 0;
unsigned long interval = 30000;

Preferences preferences;  //flash storage for retaining values through reboot

NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> strip(Pixels, PixelPin);
// NeoPixel animation time management objects,
// one for each water pixel and one to control the loop
NeoPixelAnimator animations(Pixels + 1, NEO_CENTISECONDS);

// create with enough animations to have one per pixel, depending on the animation
// effect, you may need more or less.
//
// since the normal animation time range is only about 65 seconds, by passing timescale value
// to the NeoPixelAnimator constructor we can increase the time range, but we also increase
// the time between the animation updates.   
// NEO_CENTISECONDS will update the animations every 100th of a second rather than the default
// of a 1000th of a second, but the time range will now extend from about 65 seconds to about
// 10.9 minutes.  But you must remember that the values passed to StartAnimations are now 
// in centiseconds.
//
// Possible values from 1 to 32768, and there some helpful constants defined as...
// NEO_MILLISECONDS        1    // ~65 seconds max duration, ms updates
// NEO_CENTISECONDS       10    // ~10.9 minutes max duration, centisecond updates
// NEO_DECISECONDS       100    // ~1.8 hours max duration, decisecond updates
// NEO_SECONDS          1000    // ~18.2 hours max duration, second updates
// NEO_DECASECONDS     10000    // ~7.5 days, 10 second updates
//

AnimEaseFunction moveEase =
      NeoEase::Linear;
//      NeoEase::QuadraticInOut;
//      NeoEase::CubicInOut;
//      NeoEase::QuarticInOut;
//      NeoEase::QuinticInOut;
//      NeoEase::SinusoidalInOut;
//      NeoEase::ExponentialInOut;
//      NeoEase::CircularInOut;

RgbwColor colorStates[Pixels];

// The server object that will listen on port 80 and respond to requests from web browsers
AsyncWebServer server(80);

// Variables to store the most recent color sent to us by the client (i.e a web browser)
int red = 0;
int green = 0;
int blue = 0;
int white = 50;
int onOff = 1;
int ease = 1;
int bright = 100;
int styleType = 1;
uint16_t animationTime = 150;  // duration of the animation, centiseconds
int wheelOffset = 0;
unsigned long myTime;

// Variables to store the current color of the pixels (we will compare with above to see if we need to update the color)
int currentRed = red;
int currentGreen = green;
int currentBlue = blue;
int currentWhite = white;
int currentOnOff = onOff;
int currentEase = ease;
int currentBright = bright;
int currentStyleType = styleType;
int currentAnimationTime = animationTime;
int nextColors[Pixels];
int myColors[Pixels];
int xmasColors[Pixels];
int fallColors[Pixels];
int july4Colors[Pixels];
int r_rainbow[Pixels];
int g_rainbow[Pixels];
int b_rainbow[Pixels];
RgbwColor rainbow[Pixels];
RgbwColor pixelColors[Pixels];
int colorPixels = 24;
int verticalShift = 0;
struct MyAnimationState
{
    RgbwColor StartingColor;  // the color the animation starts at
    RgbwColor EndingColor; // the color the animation will end at
    AnimEaseFunction Easeing; // the acceleration curve it will use 
};

MyAnimationState animationState[Pixels];   // one entry per pixel to match the animation timing manager


void InitializeColorArrays() {
  // initialize all one color
  for (int i=0; i < Pixels-1; i++) {
    myColors[i] = 1;
    xmasColors[i] = 1;
    fallColors[i] = 3;
    july4Colors[i] = 1;
  }
  int index = 0;
  for (int i=3; i < Pixels-1; i++) {
    if(i%3 == 0) index += 1;
    if(index > 3) index = 1;
    switch (index) {
      case 1:
        xmasColors[i] = 3;
        fallColors[i] = 8;
        july4Colors[i] = 2;
        break;
      case 2:
        xmasColors[i] = 4;
        fallColors[i] = 7;
        july4Colors[i] = 4; 
        break;
      case 3:
        xmasColors[i] = 1;
        fallColors[i] = 3;
        july4Colors[i] = 1; 
        break;                  
    }
  }
  // print out arrays
  Serial.print("xmasColors = ");
  for (int i=0; i < Pixels-1; i++) {
    Serial.print(xmasColors[i]);
    Serial.print(" ");
  }
  Serial.println("**");

  Serial.print("fallColors = ");
  for (int i=0; i < Pixels-1; i++) {
    Serial.print(fallColors[i]);
    Serial.print(" ");
  }
  Serial.println("**");

    Serial.print("july4Colors = ");
  for (int i=0; i < Pixels-1; i++) {
    Serial.print(july4Colors[i]);
    Serial.print(" ");
  }
  Serial.println("**");
  
  //Initialize color wheel
  float hue=0;
  float sat=1.0;
  float bright=0.3;
  HsbColor hsb;
  for (int i=0; i < Pixels; i++) {
    hue=float(i)/float(Pixels);
    rainbow[i]=RgbwColor(HsbColor(hue,sat,bright));
    r_rainbow[i]=rainbow[i].R;
    g_rainbow[i]=rainbow[i].G;
    b_rainbow[i]=rainbow[i].B;
    Serial.print("hue = ");
    Serial.print(hue);
    Serial.print("  r_rainbow = ");
    Serial.print(r_rainbow[i]);
    Serial.print(" ");    
    Serial.print(" b_rainbow = ");
    Serial.print(b_rainbow[i]);
    Serial.print(" ");    
    Serial.print(" g_rainbow = ");
    Serial.print(g_rainbow[i]); 
    Serial.println(" **");   
  }

}


int GetWheelPosition() {
  // wheelOffset +=1;
  // if (wheelOffset >= Pixels) wheelOffset = 0;
  wheelOffset -=1;
  if (wheelOffset < 0) wheelOffset = Pixels - 1;
  return wheelOffset;
}

RgbwColor GetColors(int colorValueCode, int pixel=0, int offset=0) {
  float brightFactor;
  float red;
  float green;
  float blue;
  float white;
  RgbwColor wheelColor;
  // offset = 1 for next color value
  pixel = pixel + offset;
  if (pixel >= Pixels) pixel = 0;

  switch (colorValueCode) {
    case 1:
      // white
      red = 00;
      green = 00;
      blue = 00;
      white = 100;        
      break;
    case 2:    
      //blue
      red = 0;
      green = 0;
      blue = 100;
      white = 0;          
      break;
    case 3:    
      //green
      red = 00;
      green = 100;
      blue = 00;
      white = 0;          
      break;    
    case 4:    
      //red
      red = 100;
      green = 0;
      blue = 0;
      white = 0;          
      break;
    case 5:    
      //pink
      red = 100;
      green = 20;
      blue = 25;
      white = 0;          
      break;
    case 6:
      // return color wheel value based on position
      wheelColor = rainbow[pixel];
      red = wheelColor.R;
      green = wheelColor.G;
      blue = wheelColor.B;
      white = wheelColor.W;
      break;
    case 7:
      //Orange
      red = 90;
      green = 15;
      blue = 0;
      white = 0;          
      break;   
    case 8:
      //maroon
      red = 72;
      green = 12;
      blue = 10;
      white = 0;          
      break;  
    case 9:
      //custom
      red = currentRed;
      green = currentGreen;
      blue = currentBlue;
      white = currentWhite;          
      break;
    default:    
      //no color
      red = 0;
      green = 0;
      blue = 0;
      white = 0;          
      break;
  } 
  brightFactor = 1.0 * (float)bright/100.0; // limit max brightness
  red = red * brightFactor;
  green = green * brightFactor;
  blue = blue * brightFactor;
  white = white * brightFactor;
  return RgbwColor(red, green, blue, white); 
}


void SpecialChristmas() {
  // special Christmas style, add random red and green
  int randomWhite = 0;
  int randomRed = 0;
  int randomFlash = 0;  
  // add random red and green
  randomFlash = random(0,999);
  if(randomFlash > 500) {
    for (int i=0; i < colorPixels; i++) {
      randomRed = random(0,Pixels-1);  //4
      randomWhite = random(0,Pixels-1);  //3    
      animationState[randomWhite].StartingColor = GetColors(1);
      animationState[randomRed].StartingColor = GetColors(4);      
    }         
  }  
}

void VerticalShift(int color1, int color2, int color3) {
  // Fill 3 colors from one 18 pixel tier to the next
  int rowColor = 0;
  int nextColor = 0;
  int pixNumber = 0;
  int c1;
  int c2;
  int c3;
  switch (verticalShift) {
    case 1:
      c1 = color2;
      c2 = color3;
      c3 = color1;
      break;
    case 2:
      c1 = color3;
      c2 = color1;
      c3 = color2;
      break;
    default:
      c1 = color1;
      c2 = color2;
      c3 = color3;
      break;
  }
  //increment verticalShift
  verticalShift+=1;
  if(verticalShift > 2) verticalShift = 0;

  //set light colors by rows
  for (int j=0; j < 3; j++) {
    for (int k=0; k < 3; k++) {
      rowColor = c1;
      nextColor = c2;
      if (k == 1) {
        rowColor = c2;
        nextColor = c3;
      } 
      if (k == 2) {
        rowColor = c3;
        nextColor = c1;
      } 
      for (int i=0; i < 18; i++) {
        pixNumber = (8 - (j*3 + k))*18 + i;
        animationState[pixNumber].StartingColor = GetColors(rowColor);
        animationState[pixNumber].EndingColor = GetColors(nextColor);      
      } 
    }        
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

void SetUpAnimation(int styleTypeCode) {
    switch (styleTypeCode) {
    case 1:  //Standard Landscape
      for (int i=0; i < Pixels; i++) {
        nextColors[i] = 1;
        myColors[i] = 1;
        pixelColors[i] = GetColors(1,0,0);
      }
      break;
    case 2:  //Christmas
      for (int i=0; i < Pixels; i++) {
        myColors[i] = xmasColors[i];
        pixelColors[i] = GetColors(xmasColors[i],0,0);
        if(i==Pixels-1){
          nextColors[i] = xmasColors[0];
        }
        else{ 
          nextColors[i] = xmasColors[i+1];
        }
      } 
      break;
    case 3:  //Autumn
      for (int i=0; i < Pixels; i++) {
        myColors[i] = fallColors[i];
        pixelColors[i] = GetColors(fallColors[i],0,0);
        if(i==Pixels-1){
          nextColors[i] = fallColors[0];
        }        
        else{ 
          nextColors[i] = fallColors[i+1];
        } 
      }   
      break;    
    case 4:  //Valentines
      for (int i=0; i < Pixels; i++) {
        pixelColors[i] = GetColors(5,0,0);
        nextColors[i] = 5;
        myColors[i] = 5;
      }
      break;  
    case 5:  //St Patricks
      for (int i=0; i < Pixels; i++) {
        pixelColors[i] = GetColors(3,0,0);
        nextColors[i] = 3;
        myColors[i] = 3;
      }
      break;    
    case 6:  //Independance Day
      for (int i=0; i < Pixels; i++) {
        myColors[i] = july4Colors[i];
        pixelColors[i] = GetColors(july4Colors[i],0,0);
        if(i==Pixels-1){
          nextColors[i] = july4Colors[0];
        }        
        else{ 
          nextColors[i] = july4Colors[i+1];
        } 
      }   
      break;     
    case 7:  //Rainbow
      for (int i=0; i < Pixels; i++) {
        pixelColors[i] = GetColors(6,i,0);
        nextColors[i] = 6;
        myColors[i] = 6;
      }
      break;    
    case 8:  //Halloween
      for (int i=0; i < Pixels; i++) {
        pixelColors[i] = GetColors(7,0,0);
        nextColors[i] = 7;
        myColors[i] = 7;
      }
      break;  
    case 9:  //Custom
      for (int i=0; i < Pixels; i++) {
        pixelColors[i] = GetColors(9,0,0);
        nextColors[i] = 9;
        myColors[i] = 9;
      }
      break;
    case 10:  //Special Christmas   
      // all white with random flashes of red and green
      // set all white
      for (int i=0; i < Pixels; i++) {
        pixelColors[i] = RgbwColor(0,50,0,20);
        nextColors[i] = 1;
        myColors[i] = 1;
      }
      break;    

    default:
      break; 
  }   
}


void LoopAnimUpdate(const AnimationParam& param)
{
    // wait for this animation to complete,
    // we are using it as a timer of sorts for the entire set of pixels
    if (param.state == AnimationState_Completed)
    {
      // advance the pixel colors before updating the animation set
      //FindNextColors();

      // update the animations for each pixel
      SetupAnimationSet();  
      
      // done, time to update all of the pixel colors
      animations.RestartAnimation(param.index);

   }
}

void SetupAnimationSet()
{
  int offset = GetWheelPosition();

  for (uint16_t pix = 0; pix < Pixels; pix++)
  {
    // starting state is the starting color 
    animationState[pix].StartingColor = pixelColors[offset];
    // end state is color of the next pixel
    offset -= 1;
    if (offset < 0) offset = Pixels - 1;
    //animationState[pix].EndingColor = GetColors(nextColors[pix], pix, 1); 
    animationState[pix].EndingColor = pixelColors[offset];
    // use the specific curve
    animationState[pix].Easeing = moveEase;
    // now use the animation states we just created and start the animation
    // which will continue to run and call the update function until it completes
    animations.StartAnimation(pix, animationTime, AnimationUpdate); 
  }
  // Special Christmas is an update to add random white and red flashes to green background
  //if (styleType==10) SpecialChristmas();
  if (styleType==10) VerticalShift(1,3,4);
}

void ConnectToWifi() {
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PW);

  Serial.println("Connecting to wifi...");
  int count = 0;
  while ((WiFi.status() != WL_CONNECTED) || (count >=20))  {
    count++;
    delay(1000);
    Serial.print("Attempt ");
    Serial.println(count);
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.print(WIFI_SSID);
    Serial.print(", IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("WiFi connection RSSI: ");
    Serial.println(WiFi.RSSI());      
  }
  else {
    Serial.println("Failed to connect to wifi!");
  }
  // Makes it so you can find the esp32 on your network by visiting WallLightControl.local/ in 
  //  your browser rather than an ip address
  MDNS.begin("WallLightControl");
  
  Serial.println(WiFi.localIP());

}

void CheckForUpdatesFromWeb() {
  bool updateFlag = false; 
  if (currentRed != red || currentGreen != green || currentBlue != blue || currentWhite != white) {
    updateFlag = true;
    currentRed = red;
    currentGreen = green;
    currentBlue = blue;
    currentWhite = white;
    Serial.print("Colors updated.  Red="); Serial.print(currentRed);Serial.print("  Green="); Serial.print(currentGreen);
    Serial.print("   Blue="); Serial.print(currentBlue); Serial.print("   White="); Serial.println(currentWhite);
    SetUpAnimation(styleType);    
  }

  if (currentStyleType != styleType) {
    updateFlag = true;
    currentStyleType = styleType;
    Serial.print("styleType upated, styleType ="); Serial.println(currentStyleType);
    // setup the animations for each pixel
    SetUpAnimation(styleType); 
  }

  if (currentEase != ease) {
    updateFlag = true;
    currentEase = ease;
    Serial.print("Ease upated, ease ="); Serial.println(currentEase);
    moveEase = EaseCode(currentEase);
  }

  if (currentBright != bright) {
    updateFlag = true;
    currentBright = bright;
    Serial.print("bright upated, bright ="); Serial.println(currentBright);    
  }

  if (currentAnimationTime != animationTime) {
    updateFlag = true;
    currentAnimationTime = animationTime;
    Serial.print("animationTime upated, animationTime ="); Serial.println(currentAnimationTime);
    // reset the master loop timing object, last object
    animations.ChangeAnimationDuration(Pixels, animationTime);    
  }

  if (currentOnOff != onOff) {
    updateFlag = true;
    currentOnOff = onOff;
    switch (onOff) {
      case 0:
        Serial.println("*** turn off ***");
        break;
      case 1:
        Serial.println("*** turn on ***");
        break;     
    }
  }

  if (updateFlag) {
    SavePreferences();
    SetUpAnimation(styleType);  //Set the colors for myColors and nextColors
    SetupAnimationSet();  // setup the animation objects for pixels 
  } 
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
    AsyncWebParameter *styleTypeParam = request->getParam("styleType", true);
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
    styleType = styleTypeParam->value().toInt();
        Serial.print("styleType="); Serial.println(styleType);
    onOff = onoffParam->value().toInt();
        Serial.print("onOff="); Serial.println(onOff);
    ease = neoEaseParam->value().toInt();
        Serial.print("ease="); Serial.println(ease); 
    bright = brightParam->value().toInt();
        Serial.print("bright="); Serial.println(bright);  
    animationTime = timeParam->value().toInt();
        Serial.print("animationTime="); Serial.println(animationTime); 
  });
  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);
  // Start the server
  server.begin();
}

void SavePreferences() {
  // save current state to flash memory
  preferences.putInt("red", red);
  preferences.putInt("green", green);
  preferences.putInt("blue", blue);
  preferences.putInt("white", white);
  preferences.putInt("ease", ease);
  preferences.putInt("style", styleType);
  preferences.putInt("animationTime", animationTime);
  preferences.putInt("bright", bright);
  preferences.putInt("onOff", onOff);  

  Serial.println("State saved to flash memory");
}


void RecallPreferences() {
  // get preferences from flash memory to restore last state
  red = preferences.getInt("red", 0);
  green = preferences.getInt("green", 0);
  blue = preferences.getInt("blue", 0);
  white = preferences.getInt("white", 50);
  ease = preferences.getInt("ease", 1);
  styleType = preferences.getInt("style", 1);
  animationTime = preferences.getInt("animationTime", 100);
  bright = preferences.getInt("bright", 25);
  onOff = preferences.getInt("onOff", 1);

  Serial.println("State recalled from flash memory");
}

void FillStripColor(RgbwColor color) {
  // fill strip with one color
  for (int i=0; i < Pixels; i++) {
      strip.SetPixelColor(i, color);
    }  
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  // Serial.println("Trying to Reconnect");
  // ConnectToWifi();
}

void setup() {
  // Setup Serial Monitor
  Serial.begin(115200);
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

  // initialize color arrays
  InitializeColorArrays();

  uint16_t i;
  strip.Begin();
  strip.Show(); // Initialize all pixels to 'off'
  FillStripColor(RgbwColor(0, 0, 0, 50));
  strip.Show();
  
  // connect to WiFi and start server, indicate status
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  PrepareWiFiServer();
  if(WiFi.status() != WL_CONNECTED) {
    FillStripColor(RgbwColor(100, 0, 0, 0)); //Red
  }
  else {
    FillStripColor(RgbwColor(0, 100, 0, 0));  //Green  
  }
  strip.Show();
  delay(3000);
  //
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  //
  SetUpAnimation(styleType);  //Set the colors for myColors and nextColors
  SetupAnimationSet();  // setup the animation objects for pixels
  // setup the master loop timing object, last object
  animations.StartAnimation(Pixels, animationTime, LoopAnimUpdate);

  myTime = millis(); // initialize loop timer
}


void loop () {
  // Check if it is time to test WiFi connection
  if((millis()-myTime)>5000) { 
    myTime = millis();
    if(WiFi.status() != WL_CONNECTED) {
      PrepareWiFiServer();
    }
    else {
      Serial.print("WiFi connection RSSI: ");
      Serial.println(WiFi.RSSI());
    }
  }

  // Check if the web interface has new parameters
  CheckForUpdatesFromWeb();

  if(onOff == 0) {  
    strip.ClearTo(RgbwColor(0,0,0,0));
  }
  else {
    animations.UpdateAnimations();
  }
  strip.Show();
  delay(5);
}
