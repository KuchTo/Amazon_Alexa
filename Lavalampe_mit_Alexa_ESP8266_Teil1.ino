#include <Espalexa.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


#define LAMP_PIN 16 //D0

// prototypes
boolean connectWifi();

//callback functions
void firstLightChanged(uint8_t brightness);

// Change this!!
const char* ssid = "WLANGW1339";
const char* password = "N72832302598295C*#";

boolean wifiConnected = false;

Espalexa espalexa;
EspalexaDevice* device;
ESP8266WebServer server(80);

void setup()
{
  Serial.begin(115200);
  pinMode(LAMP_PIN,OUTPUT); 
  digitalWrite(LAMP_PIN,LOW);
  // Initialise wifi connection
  wifiConnected = connectWifi(); 
  if(wifiConnected){
    server.on("/", HTTP_GET, [](){
    server.send(200, "text/plain", "This is an example index page your server may send.");
    });
    server.on("/test", HTTP_GET, [](){
    server.send(200, "text/plain", "This is a second subpage you may have.");
    });
    server.onNotFound([](){
      if (!espalexa.handleAlexaApiCall(server.uri(),server.arg(0))) //if you don't know the URI, ask espalexa whether it is an Alexa control request
      {
        //whatever you want to do with 404s
        server.send(404, "text/plain", "Not found");
      }
    });
    // Define our LavaLamp 
    device = new EspalexaDevice("Lavalampe", LightChanged,0); // default state off
    espalexa.addDevice(device);    
    espalexa.begin(&server); //give espalexa a pointer to your server object so it can use your server instead of creating its own
  } else
  {
    while (1)
    {
      Serial.println("Cannot connect to WiFi. Please check data and reset the ESP.");
      delay(2500);
    }
  }
}
 
void loop()
{
   espalexa.loop();
   delay(1);
}

//our callback Function
void LightChanged(uint8_t brightness) {
    Serial.print("Lavalampe changed to ");
    if (brightness == 0) {
      Serial.println("OFF");
         digitalWrite(LAMP_PIN,LOW);
    }
    else  {
      Serial.println("ON");
         digitalWrite(LAMP_PIN,HIGH);
    }
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi(){
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false; break;
    }
    i++;
  }
  Serial.println("");
  if (state){
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("Connection failed.");
  }
  delay(100);
  return state;
}
