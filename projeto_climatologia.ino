#include <PubSubClient.h> // Biblioteca mqtt
#include <DHT.h> //Biblioteca sensor umidade e temperatura
#include <WiFi.h> //Biblioteca de wifi
#define pino_dht 4
#define tip_dht DHT11
#define USUARIO  "Climatologia"
#define SENHA       "XXXXXXX"
DHT dht(pino_dht, tip_dht);
const char* ssid_wifi = "XXXXX";
const char* senha_wifi = "XXXXXXX";
const char* mqtt_server = "52.54.163.195";
unsigned long tempo = 0;
int reboot = 0;
int leitura = 0;
String temp_str;
String umid_str;
char temp[50];
char umid[50];
WiFiClient Esp;
PubSubClient cliente(Esp);
void conexao_mqtt() { // Função responsavel pela conexão
    while (!cliente.connected()) {
        Serial.print("Tentando conexão com mqtt");
        String clienteId = "ESP32_";
        clienteId += String(random(0xffff), HEX);

        if (cliente.connect(clienteId.c_str(), USUARIO, SENHA, "Climatologia/feeds/esta-on", 1, true, "Desligado")) {
            Serial.println("Conectado");
            cliente.publish("Climatologia/feeds/esta-on", "Ligado", true);
            cliente.subscribe("Climatologia/feeds/reboot");
            cliente.subscribe("Climatologia/feeds/ativar-temp-umid");


        } else {
            Serial.print("falha, rc=");
            Serial.print(cliente.state());
            Serial.println(" tentando novamente em 5 segundos");
            delay(5000);
        }
    }
}

void retorno_mensagem(char* topic, byte* payload, unsigned int length) { //função responsável pela inscrição dos topicos
    Serial.print("Mensagem do topico [");
    Serial.print(topic);
    Serial.print("]:");
    if (strcmp(topic, "Climatologia/feeds/reboot") == 0) {
        reboot = 1;
    }
    if (strcmp(topic, "Climatologia/feeds/ativar-temp-umid") == 0) {
        leitura = 1;
    }
    Serial.println();
}
void configuracao_wifi() { //função para configurar o wifi
    WiFi.begin(ssid_wifi, senha_wifi);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("conetando...");
    }
    Serial.println("conectou");

}
void send_m(float h, float t) { //função para enviar a mensagem nos topico umid e temp 
    umid_str = String((String)h );
    temp_str = String((String)t );

    umid_str.toCharArray(umid, umid_str.length() + 1);
    temp_str.toCharArray(temp, temp_str.length() + 1);
    cliente.publish("Climatologia/feeds/umid", umid);
    cliente.publish("Climatologia/feeds/temp", temp);
}
void setup() { // ativação das funções padrões 
  Serial.begin(9600);
  configuracao_wifi();
  cliente.setServer(mqtt_server, 1883);
  cliente.setCallback(retorno_mensagem);
  dht.begin();
}
void loop() { //leitura e reconexão com mqtt sempre que desconectado 
  if (!cliente.connected()) {
    conexao_mqtt();
  }
  cliente.loop();
  if (leitura) {
    float h = 0;
    float t = 0;
    for (int i = 0; i < 10000; i++) {
      h += dht.readHumidity();
      t += dht.readTemperature();
    }
    t = t / 10000;
    h = h / 10000;
    send_m(h, t);
    leitura = 0;
  }
  if (reboot == 1) {
    ESP.restart();
  }
}
