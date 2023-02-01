// Lighting controller for front wall lights
//  Customized by WiFi controller with persistant settings and OTA updates
//  Control hardware: HiLetgo ESP-WROOM-32 ESP-32S Development Board
//  Lights: Strip of BTF-LIGHTING RGBWW Warm White SK6812 60 Pixels/m
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

#define Pixels 162  // number of neopixels
const uint8_t PixelPin = 14;  // data pin for neopixels IO14
// update as needed
#define WIFI_SSID "E6"
#define WIFI_PW "hightime9955B"
unsigned long previousMillis = 0;
unsigned long interval = 30000;

Preferences preferences;  //flash storage for retaining values through reboot

NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> strip(Pixels, PixelPin);
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
AsyncWebSocket ws("/ws");

// Variables to store the most recent color sent to us by the client (i.e a web browser)
RgbwColor color[] = {RgbwColor(0,0,0,0),RgbwColor(0,0,0,0),RgbwColor(0,0,0,0),RgbwColor(0,0,0,0),RgbwColor(0,0,0,0)};
int speedType = 1;
int styleType = 1;
uint16_t animationTime = 100;  // duration of the animation, centiseconds
int wheelOffset = 0;
unsigned long myTime;
int activeColors = 0;
int startingColor = 0;
int rowShift = 0;
int offSet = 0;
int currentStyleType = styleType;
int currentSpeed = speedType;
RgbwColor nextColors[Pixels];
RgbwColor myColors[Pixels];
RgbwColor rainbow[Pixels];
RgbwColor pixelColors[Pixels];
String strColor0;
String strColor1;
String strColor2;
String strColor3;
String strColor4;
struct MyAnimationState
{
    RgbwColor StartingColor;  // the color the animation starts at
    RgbwColor EndingColor; // the color the animation will end at
    AnimEaseFunction Easeing; // the acceleration curve it will use 
};

MyAnimationState animationState[Pixels];   // one entry per pixel to match the animation timing manager

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.print("got message on WS: ");
    String str = (char*)data;
    Serial.println(str);
    // notify client
    String statusStr = "IP address:" + WiFi.localIP().toString() + ", RSSI: " + String(WiFi.RSSI());
    Serial.println(statusStr);
    ws.textAll(statusStr);
    // update control parameters
    ApplyUpdatesFromWeb(str);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void InitializeColorWheel() {
  //Initialize color wheel
  float hue=0;
  float sat=1.0;
  float bright=0.3;
  HsbColor hsb;
  for (int i=0; i < Pixels; i++) {
    hue=float(i)/float(Pixels);
    rainbow[i]=RgbwColor(HsbColor(hue,sat,bright));
  }

}

void VerticalShift() {
  // Fill colors from one 18 pixel tier to the next
  int rowColor = 0;
  int nextColor = 0;
  int pixNumber = 0;
  int k = rowShift;
  moveEase = NeoEase::CubicInOut;
  //set light colors by rows (9 rows of 18)
  for (int row=8; row >= 0; row--) {
    rowColor = k;
    //increment for next color
    k+=1;
    if(k >= activeColors) k = 0; 
    nextColor = k;
    // active colors, start from top
    for (int i=0; i < 18; i++) {
      pixNumber = (row*18) + i;
      myColors[pixNumber] = color[rowColor];
      nextColors[pixNumber] = color[nextColor];      
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

int isActive(RgbwColor c) {
  // check if color is black
  if((c.R + c.G + c.B + c.W)<=1) {
    return 0;
  }
  return 1;
}

int GetPriorColor(int i) {
  int j = i-1;
  if(j<0) j=activeColors-1;
  return j;
}

int GetNextColor(int i) {
  int j = i+1;
  if(j>=activeColors) j=0;
  return j;  
}

void ChasingColors(){
  moveEase = NeoEase::CubicInOut;
  int a[Pixels];
  int b[Pixels];
  int k;
  k = startingColor;
  // fill arrays
  for (int i=0; i < Pixels; i+=2) {
    a[i] = k;
    a[i+1] = k;
    b[i] = GetPriorColor(k);
    b[i+1] = k;     
    if(offSet==1) {
      a[i+1] = GetNextColor(k);
      b[i] = k;
    }
    k=GetNextColor(k);    
  }

  for (int i=0; i < Pixels; i++) { 
      myColors[i] = color[a[i]];
      nextColors[i] = color[b[i]];  
  }

}

void RainbowEffect() {
  int pix = wheelOffset;
  for (int i=0; i < Pixels; i++) {
    myColors[i] = rainbow[pix];
    pix +=1;
    if(pix >= Pixels) pix = 0;
    nextColors[i] = rainbow[pix];
  }
}

void LoopAnimUpdate(const AnimationParam& param) {
    // wait for this animation to complete,
    // we are using it as a timer of sorts for the entire set of pixels
    if (param.state == AnimationState_Completed)
    {
      // update the animations for each pixel
      SetupAnimationSet();   
      // done, time to update all of the pixel colors
      animations.RestartAnimation(param.index);
   }
}

void SetupAnimationSet() {
  //count active colors
  activeColors = 0;
  activeColors = isActive(color[0]) + isActive(color[1]) + isActive(color[2]) + isActive(color[3]) + isActive(color[4]);  
  moveEase = NeoEase::Linear;  //default  
  // Set lights and increment annimation to next position
  switch (styleType) {
    case 0:  //off
      for (int i=0; i < Pixels; i++) {
        myColors[i] = RgbwColor(0,0,0,0);
        nextColors[i] = RgbwColor(0,0,0,0);
      }
      break;
    case 1:  //solid color, use color[0]
      for (int i=0; i < Pixels; i++) {
        myColors[i] = color[0];
        nextColors[i] = color[0];
      }
      break;
    case 2:  //chasing
      ChasingColors();
      // increment starting color
      offSet=offSet+1;
      if(offSet>=2)offSet = 0; 
      if(offSet==1) {
        startingColor=startingColor-1;
        if(startingColor<0)startingColor = activeColors-1;
      }
      break;
    case 3:  //waterfall
      VerticalShift();
      // increment vertical shift
      rowShift +=1;
      if(rowShift>=activeColors) rowShift = 0;  
      break;    
    case 4:  //rainbow
      RainbowEffect();
      wheelOffset -=1;
      if (wheelOffset < 0) wheelOffset = Pixels - 1;
      break;
    default:
      break; 
  }

  //SetAnimationTime(speedType);
  for (uint16_t pix = 0; pix < Pixels; pix++)
  {
    // starting state is the starting color 
    animationState[pix].StartingColor = myColors[pix];
    animationState[pix].EndingColor = nextColors[pix];
    animationState[pix].Easeing = moveEase;
    animations.StartAnimation(pix, animationTime, AnimationUpdate); 
  }
  //Serial.println("speedType="+String(speedType)+"  animationTime="+String(animationTime)) ;
}

void SetAnimationTime(int speed) {
  // set animationTime
  switch (speed) {
    case 0: //slow
      animationTime = 300;
      break;    
    case 1: // medium
      animationTime = 100;
      break;
    case 2: // fast
      animationTime = 30;
      break;
    case 3: // maximum     
      animationTime = 10;
      break;
  }
  // setup the master loop timing object, last object
  animations.StartAnimation(Pixels, animationTime, LoopAnimUpdate);
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

String getParsedValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void ApplyUpdatesFromWeb(String message) {
  //parse message for parameters
  // Example: style=1&speed=1&color1=#000000&color2=#000000&color3=#000000&color4=#000000&color5=#000000
  String result;
  String key;
  String value;
  int x;
  for(int i=0; i<7; i++){
    result = getParsedValue(message,'&', i);
    Serial.println(String(i)+" "+result);
    // parse result for key and value
    x = result.indexOf('=');
    key = result.substring(0,x);
    value = result.substring(x+1,result.length());
    Serial.println("Key:" + key + "  Value:" + value);
    // update local variables
    if(key == "style") {
      styleType = value.toInt();
    } 
    else if(key == "speed") {
      speedType = value.toInt();
      SetAnimationTime(speedType);
    }
    else if(key == "color0"){
      strColor0 = value;
      color[0] = SetColorFromString(value);
    }
    else if(key == "color1"){
      strColor1 = value;
      color[1] = SetColorFromString(value);
    }
    else if(key == "color2"){
      strColor2 = value;
      color[2] = SetColorFromString(value);
    }
    else if(key == "color3"){
      strColor3 = value;
      color[3] = SetColorFromString(value);
    }
    else if(key == "color4"){
      strColor4 = value;
      color[4] = SetColorFromString(value);
    }

  }
  Serial.println();
  SavePreferences();
  SetupAnimationSet();  // setup the animation objects for pixels 
}

RgbwColor SetColorFromString(String val){
  int r = 0;
  int g = 0;
  int b = 0;
  int w = 0;
  int Ro = 0;
  int Go = 0;
  int Bo = 0;
  int Wo = 0;
  if(val.length()==7) {
    r = strtol(val.substring(1,3).c_str(),0,16);
    g = strtol(val.substring(3,5).c_str(),0,16);
    b = strtol(val.substring(5,7).c_str(),0,16);
    Serial.print("r="+String(r));
    Serial.print("  g="+String(g));
    Serial.println("  b="+String(b));
    //convert to RGBW
    Wo = min(r, min(g, b));
    Ro = r - Wo; 
    Go = g - Wo; 
    Bo = b - Wo;
  }
  // check if essentially black
  int colorSum = (Wo+Ro+Go+Bo);
  if(colorSum<2) {
    Wo=0;
    Ro=0;
    Go=0;
    Bo=0;
  }
  // limit brightness to prevent overheating
  if(colorSum>120) {
    float factor = 120.0/(float)colorSum;
    Wo=Wo * factor;
    Ro=Ro * factor;
    Go=Go * factor;
    Bo=Bo * factor;;  
  }
  Serial.print("Ro="+String(Ro));
  Serial.print("  Go="+String(Go));
  Serial.print("  Bo="+String(Bo));
  Serial.println("  Wo="+String(Wo));
  return RgbwColor(Ro,Go,Bo,Wo);
} 

void PrepareWiFiServer() {
  ConnectToWifi();

  // On a GET request, respond with the HTML string found in control_page.h
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", HTML);
  });

  // Start the server
  server.begin();
}

void SavePreferences() {
  // save current state to flash memory
  preferences.putString("color0", strColor0);
  preferences.putString("color1", strColor1);
  preferences.putString("color2", strColor2);
  preferences.putString("color3", strColor3);
  preferences.putString("color4", strColor4);
  preferences.putInt("style", styleType);
  preferences.putInt("speed", speedType);

  Serial.println("State saved to flash memory");
}

void RecallPreferences() {
  // get preferences from flash memory to restore last state
  color[0] = SetColorFromString(preferences.getString("color0", "#000000"));
  color[1] = SetColorFromString(preferences.getString("color1", "#000000"));
  color[2] = SetColorFromString(preferences.getString("color2", "#000000"));
  color[3] = SetColorFromString(preferences.getString("color3", "#000000"));
  color[4] = SetColorFromString(preferences.getString("color4", "#000000"));
  styleType = preferences.getInt("style", 1);
  speedType = preferences.getInt("speed", 1);
  SetAnimationTime(speedType);
  Serial.println("State recalled from flash memory");
}

void FillStripColor(RgbwColor color) {
  // fill strip with one color
  for (int i=0; i < Pixels; i++) {
      strip.SetPixelColor(i, color);
    }  
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
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

  // initialize color wheel for rainbow
  InitializeColorWheel();

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

  // start web socket 
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);
  // set up the annimations based on user input
  SetupAnimationSet();
  // setup the master loop timing object, last object
  animations.StartAnimation(Pixels, animationTime, LoopAnimUpdate);

  myTime = millis(); // initialize loop timer
}


void loop () {
  // Check if it is time to test WiFi connection
  if((millis()-myTime)>5000) { 
    myTime = millis();
    ws.cleanupClients();
    if(WiFi.status() != WL_CONNECTED) {
      PrepareWiFiServer();
    }
    else {
      // Serial.print("WiFi connection RSSI: ");
      // Serial.println(WiFi.RSSI());
    }
  }
  animations.UpdateAnimations();
  strip.Show();
  //delay(5);
}
