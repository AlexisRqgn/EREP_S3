#include <SI114X.h>
#include "Adafruit_SI1145.h"
#include <WiFi.h>
#include <Wire.h>
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT22
#include "Arduino.h"
#include "SI114X.h"
#include "PubSubClient.h"

DHT dht(DHTPIN, DHTTYPE);
SI114X SI1145 = SI114X();

//WIFI
const char* ssid = "fuckshavox";
const char* password = "soissons02";

//MQTT
const char* mqtt_server = "192.168.4.1";
const char* humidity_topic = "home/pi/humidity";
const char* temperature_topic = "home/pi/temperature";
const char* light_topic = "home/pi/light";
const char* mqtt_username = "cdavid";
const char* mqtt_password = "cdavid";
const char* clientID = "client_esp";

WiFiClient wifiClient;
//1883 port for broker
PubSubClient client(mqtt_server, 1883, wifiClient);

void connect_MQTT(){
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if(client.connect(clientID, mqtt_username, mqtt_password)){
    Serial.println("Connected to MQTT Broker !");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}

void setup(){
    Serial.begin(115200);
    delay(1000);
    Serial.println("Si1145: démarrage...");
    Serial.println(F("Connexion DHT22 réussie"));
    dht.begin();

 while (!SI1145.Begin()) {
    Serial.println("SI1145: pas prêt");
    delay(1000);
  }
  Serial.println("Si1145: prêt");

    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }
    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

void loop(){
    connect_MQTT();
    Serial.setTimeout(2000);

 float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

   if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
   }

    float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F("\nHumidity: "));
  Serial.print(h);
  Serial.print("%");
  Serial.print(F("\nTemperature: "));
  Serial.print(t);
  Serial.println(F("°C "));

  Serial.print("//--------------------------------------//\r\n");
  Serial.print("Vis: "); Serial.println(SI1145.ReadVisible());
  Serial.print("IR: "); Serial.println(SI1145.ReadIR());
  //the real UV value must be div 100 from the reg value , datasheet for more information.
  Serial.print("UV: ");  Serial.println((float)SI1145.ReadUV()/100);
 
  
  //Serial.print(f);
  //Serial.print(F("°F  Heat index: "));
  //Serial.print(hic);
  // Serial.print(F("°C "));
  //Serial.print(hif);
  //Serial.println(F("°F"));

  String hs="Hum: "+String((float)h)+" %";
  String ts="Temp: "+String((float)t)+"°C";
  String vs="Vis: "+String((float)SI1145.ReadVisible());

  if(client.publish(temperature_topic, String(t).c_str())){
    Serial.println("Temperature envoyee");
  }
  else{
    Serial.println("Impossible de send la temperature");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(temperature_topic, String(t).c_str());
  }

 if(client.publish(humidity_topic, String(h).c_str())){
    Serial.println("Humidite envoyee");
  }
  else{
    Serial.println("Impossible de send l humidite");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(humidity_topic, String(t).c_str());
  }

if(client.publish(light_topic, String(SI1145.ReadVisible()).c_str())){
    Serial.println("Visibilite envoyee");
  }
  else{
    Serial.println("Impossible de send la luminosite");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(light_topic, String(SI1145.ReadVisible()).c_str());
  }
  
  delay(1000);
  }
