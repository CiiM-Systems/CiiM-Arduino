#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configurações da rede WiFi
const char* ssid = "Guilherme4G";
const char* password = "gui12345";

// Configurações do servidor MQTT
const char* mqttServer = "192.168.20.85";
const int mqttPort = 1883;

// Pino de leitura analógica
const int sensorHumidity = 32;
const int relePin = 2;

const int maxHumidity = 40;
const int minHumidity = 20;

// Intervalo de envio (30 segundos = 30000 milissegundos)
unsigned long previousMillis = 0;
const long interval = 10000;  // 30 segundos

bool currentRelayState = LOW; // Estado atual do relé

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("Erro ao fazer parsing do JSON: ");
    Serial.println(error.c_str());
    return;
  }

  if (strcmp(topic, "switch") == 0) {
    bool power = doc["power"];
    Serial.print("Valor de power: ");
    Serial.println(power ? "ON" : "OFF");
    controlRelay(power);
  }
  
}


void setup() {
  // Inicia a comunicação serial
  Serial.begin(9600);
  delay(2000);

  // Configura o pino como entrada analógica
  pinMode(sensorHumidity, INPUT);
  pinMode(relePin, OUTPUT);

  // Conecta ao Wi-Fi
  connectToWiFi();

  // Configura o servidor MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  connectToMQTT();
  client.subscribe("power");
}

void loop() {
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();

  // Lê o valor do pino e envia a cada 30 segundos
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  
    Serial.println("Obtendo umidade...");

    int percentage = getHumidityPercentage();
    Serial.println("Enviando umidade...");
    sendHumidityData(percentage);

    if(percentage >= maxHumidity) {
      controlRelay(false);
    } else if (percentage <= minHumidity) {
      controlRelay(true);
    }
  }
}

void sendHumidityData(int value) {
  String payload = "{\"value\": " + String(value) + "}";
  Serial.print("Enviando mensagem: ");
  Serial.println(payload);
  client.publish("data", payload.c_str());
}

void controlRelay(bool powerState) {
  if (currentRelayState != powerState) {
    digitalWrite(relePin, powerState ? HIGH : LOW);
    currentRelayState = powerState;

    // Monta o payload e publica no MQTT
    String payload = "{\"power\": " + String(powerState ? "true" : "false") + "}";
    client.publish("power", payload.c_str());
    Serial.print("Publicando: ");
    Serial.println(payload);
  } else {
    Serial.println("Estado do relé já está correto. Sem alteração.");
  }
}

int getHumidityPercentage() {
  int analogValue = analogRead(sensorHumidity);
  Serial.print("Valor anal´ogico lido: ");
  Serial.println(analogValue);
  int percentage = 100 - (((analogValue - 1) * 100) / (4095 - 1));
  return percentage;
}

void connectToWiFi() {
  Serial.print("Conectando à rede WiFi: ");
  Serial.println(ssid);
  Serial.println(WiFi.status());
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
  }

  WiFi.begin(ssid, password);
  Serial.println(WiFi.status());
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  Serial.print("Conectando ao servidor MQTT: ");
  Serial.println(mqttServer);

  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect("ArduinoClient")) {
      Serial.println("Conectado ao MQTT!");
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}
