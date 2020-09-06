#include <WiFi.h>
#include <WiFiClient.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <BlynkSimpleEsp32.h>
#include<DHT.h>
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                  
#define AIO_USERNAME    "YOUR AFIO USERNAME"           
#define AIO_KEY         "YOUR AFIO API KEY" 
/*Pins Used 
*       Sensor           :   PIN   :   GPIO NO   :   Blynk Pin    :   Adafruit IO Feed   :
 * ------------------------------------------------------------------------------------
 * "DHT PIN Humidity"     :   D2    :     2       :   V0          :   
 * "DHT PIN Temperature"  :   D2    :     2       :   V1          :
 * "Light Relay"          :   D4    :     4       :   gp4         :   lights
 * "Fan Relay"            :   D13   :     13      :   gp13        :   relay2
 * "Sound Sensor"         :   D8    :     18      :               :
 * "Disco Relay"          :   D18   :      8      :               :
 */
DHT dht(2,DHT11);
float temperature;
float humidity;
int dht_pin=2;
int light_relay=4;
int fan_relay=13;
int sound_sns=8;
int disco_relay=18;
char auth[] = "BLYNK API KEY";
char ssid[] = "YOUR NETWORK SSID";
char pass[] = "YOUR NETWORK PASSWORD";
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe Light = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/light");
Adafruit_MQTT_Subscribe Fan = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/relay2");
void MQTT_connect();
void discoLight();
void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  pinMode(dht_pin,OUTPUT);
  pinMode(light_relay, OUTPUT);
  pinMode(disco_relay, OUTPUT);
  pinMode(sound_sns, INPUT);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  mqtt.subscribe(&Light);
}
void loop()
{
  MQTT_connect();
  discoLight();
  Adafruit_MQTT_Subscribe *subscription;
  temperature=dht.readTemperature();
  humidity=dht.readHumidity();
  Blynk.run();
  Blynk.virtualWrite(V0,humidity);
  Blynk.virtualWrite(V1,temperature);
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription == &Fan) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light.lastread);
      int Fan_State = atoi((char *)Light.lastread);
      Serial.println(Fan_State);
      digitalWrite(fan_relay,Fan_State);  
    }
    if (subscription == &Light) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light.lastread);
      int Light_State = atoi((char *)Light.lastread);
      Serial.println(Light_State);
      digitalWrite(light_relay, Light_State);  
    }
  }
}
void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { 
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
void discoLight(){
  digitalWrite(disco_relay,digitalRead(sound_sns));
}
