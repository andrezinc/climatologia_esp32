#include <PubSubClient.h> // Biblioteca mqtt
#include <DHT.h> //Biblioteca sensor umidade e temperatura
#include <WiFi.h> //Biblioteca de wifi
#define pino_dht 4
#define tip_dht DHT11
#define IO_USERNAME  "xxx"
#define IO_KEY       "xxx"
DHT dht(pino_dht, tip_dht);
const char* ssid = "TP-xx";
const char* password = "xxx";
const char* mqtt_server = "xxxx";
unsigned long tempo = 0;
String temp_str;
String umid_str;
char temp[50];
char umid[50];
WiFiClient Esp;
PubSubClient client(Esp);
void callback(char* topic, byte* payload, unsigned int length);
void setup_wifi();
void reconnect();
void send_m(float h, float t);
void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (millis() - tempo > 900000) {
    float h = 0;
    float t = 0;
    for (int i = 0; i < 10000; i++) {
      h += dht.readHumidity();
      t += dht.readTemperature();
    }
    t = t / 10000;
    h = h / 10000;
    send_m(h, t);
    tempo = millis();
  }
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("conetando...");
  }
  Serial.println("conectou");

}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem do topico [");
  Serial.print(topic);
  Serial.print("]:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão com mqtt");
    String clientId = "ESP32_Andre-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), IO_USERNAME, IO_KEY, "Climatologia/feeds/esta-on", 1, true, "Desligado")) {
      Serial.println("Conectado");
      client.publish("Climatologia/feeds/esta-on", "Ligado", true);
      client.subscribe("Climatologia/feeds/teste");
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}
void send_m(float h, float t) {
  umid_str = String((String)h );
  temp_str = String((String)t );

  umid_str.toCharArray(umid, umid_str.length() + 1);
  temp_str.toCharArray(temp, temp_str.length() + 1);
  client.publish("Climatologia/feeds/umid-andre", umid);
  client.publish("Climatologia/feeds/temp-andre", temp);
}
