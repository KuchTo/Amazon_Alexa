/*  Alexa LED Desk LAMP
 *   
 *  Zweck:
 *  Steuerung einer LED Niederspannungsschreibtischlampe bis 12 Volt mit Hilfe von Alexa
 * 
 *  Tobias Kuch 2020 -- tobias.kuch@googlemail.com
 * 
 * Lizensiert under GPL 3.0
 * 
 *  
 */
 
#include <Espalexa.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h> //if you get an error here please update to ESP32 arduino core 1.0.0
#include <WiFiSettings.h>

const uint8_t LED_PIN = 23;  // Status LED
const uint8_t LIGHT_MANUAL_SW = 22; // Switch
const String Alexa_DeviceName = "Schreibtischlampe";

// PWM Configuration Stuff for ESP 32
const int freq = 5000; 
const int PWMChannel = 0; 
const int resolution = 8;
const int PWMOutputPin = 16;  // 16 corresponds to GPIO16


#define LED_ON  LOW
#define LED_OFF HIGH


//callback functions
void firstLightChanged(uint8_t brightness);

//boolean wifiConnected = false;

EspalexaDevice* lamp1;

Espalexa espalexa;
#ifdef ARDUINO_ARCH_ESP32
WebServer server(80);
#else
ESP8266WebServer server(80);
#endif

bool Sw_old_State = true;

// Bugfix: Every second time, no Success Connection is Made with Access Point
WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.print("WiFi lost connection. Reason: ");
    Serial.println(info.disconnected.reason);
    if (info.disconnected.reason == 202) {
    Serial.println("Connection failed, REBOOT/SLEEP!");
     esp_sleep_enable_timer_wakeup(10);
     esp_deep_sleep_start();
    delay(100);   
    }
  }, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);

void setup() {
Serial.begin(115200);
SPIFFS.begin(true);  // Will format on the first run after failing to mount
pinMode(LIGHT_MANUAL_SW, INPUT_PULLUP);
pinMode(LED_PIN, OUTPUT);
pinMode(PWMOutputPin, OUTPUT);
ledcSetup(PWMChannel, freq, resolution);
ledcAttachPin(PWMOutputPin, PWMChannel);
// Set custom callback functions
WiFiSettings.onSuccess  = []() {
digitalWrite(LED_PIN, LED_ON); // Turn LED on
};
WiFiSettings.onFailure  = []() {
digitalWrite(LED_PIN, LED_OFF); // Turn LED off
};
WiFiSettings.onWaitLoop = []() {
digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle LED
return 500; // Delay next function call by 500ms
};
// Callback functions do not have to be lambda's, e.g.
// WiFiSettings.onPortalWaitLoop = blink;
// Define custom settings saved by WifiSettings
// These will return the default if nothing was set before
String host = WiFiSettings.string( "server_host", "default.example.org");
int    port = WiFiSettings.integer("server_port", 80);
// Connect to WiFi with a timeout of 30 seconds
// Launches the portal if the connection failed
WiFiSettings.connect(true, 40);
Serial.println("Initializing Alexa API.."); 
server.on("/", HTTP_GET, []()
  {
  server.send(200, "text/plain", "Alexa Schreibtischlampe . Tobias Kuch 2020 tobias.kuch@googlemail.com");
  });
server.onNotFound([]()
  {
  if (!espalexa.handleAlexaApiCall(server.uri(),server.arg(0))) //if you don't know the URI, ask espalexa whether it is an Alexa control request
    {
    //whatever you want to do with 404s
    server.send(404, "text/plain", "404 - Not found");
    }
  });

// Define Alexa Devices
lamp1 = new EspalexaDevice(Alexa_DeviceName, firstLightChanged, EspalexaDeviceType::dimmable); //Dimmable device, optional 4th parameter is beginning state (here fully off)
espalexa.addDevice(lamp1);
espalexa.begin(&server); //give espalexa a pointer to your server object so it can use your server instead of creating its own
Serial.println("Done.");  
}


void Query_Manual_Controls()
{
bool Sw = digitalRead(LIGHT_MANUAL_SW);
if ((Sw == LOW) and (Sw_old_State == false))
  {
  Sw_old_State = true;
  delay(100);
  if (lamp1->getValue())
    {
    Serial.println("Manual OFF");
    lamp1->setValue(0);
    ledcWrite(PWMChannel, lamp1->getValue());
    }
    else 
    {
    Serial.println("Manual ON");
    lamp1->setValue(255);
    ledcWrite(PWMChannel, lamp1->getValue());
    }
  } else if (Sw == HIGH)
  {
    Sw_old_State = false;
  }
}

void loop() {
  espalexa.loop();
  Query_Manual_Controls();
  delay(1);
}

// Callback functions
void firstLightChanged(EspalexaDevice* d) {
    Serial.print("Lamp changed to ");
    if (d->getValue() == 255) {
      Serial.println("ON");
    }
    else if (d->getValue() == 0) {
      Serial.println("OFF");
    }
    else {
      Serial.print("DIM "); Serial.println(d->getValue());
    }
    ledcWrite(PWMChannel, lamp1->getValue());
}
