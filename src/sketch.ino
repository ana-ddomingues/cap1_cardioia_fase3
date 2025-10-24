#include <Arduino.h>
#include <DHT.h>
#include "SPIFFS.h"

// Parte 2: Conectividade, MQTT e JSON
#include <WiFi.h>           // Conectividade Wi‑Fi (ESP32)
#include <PubSubClient.h>   // Cliente MQTT leve
#include <ArduinoJson.h>    // Criação/serialização de JSON

// Força o uso imediato da rede simulada do Wokwi (sem senha, canal 6)
#ifndef WOKWI
#define WOKWI 1
#endif

// --- Configurações Wi-Fi (Parte 2) ---
const char* WIFI_SSID = "Florescer";
const char* WIFI_PASSWORD = "48996589513";

// --- Configurações MQTT (Parte 2) ---
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "cardioia/paciente/dados";

// --- Clientes Wi-Fi e MQTT (Parte 2) ---
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Conecta ao Wi‑Fi exibindo progresso no Serial Monitor
void setup_wifi() {
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.mode(WIFI_STA);
  // No Wokwi, a rede simulada é "Wokwi-GUEST" (sem senha, canal 6). No hardware real, usa as credenciais definidas.
#ifdef WOKWI
  WiFi.begin("Wokwi-GUEST", "", 6);
#else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

  const unsigned long timeoutMs = 15000; // 15s de timeout para não travar o setup
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
    delay(500);
    Serial.print('.');
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("Wi-Fi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Wi-Fi não conectou dentro do tempo limite.");
    // Tentativa extra: fallback para rede simulada do Wokwi (caso esteja rodando no simulador)
    Serial.println("Tentando rede 'Wokwi-GUEST' por 5s...");
    WiFi.disconnect(true);
    WiFi.begin("Wokwi-GUEST", "", 6);
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - t0) < 5000) {
      delay(500);
      Serial.print('.');
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.println("Conectado ao Wokwi-GUEST!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println();
      Serial.println("Sem Wi‑Fi. Seguindo em modo offline.");
    }
  }
}

// Conecta/reconecta ao broker MQTT com tentativas periódicas
void reconnect_mqtt() {
  // Só tenta se o Wi‑Fi estiver OK
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  static unsigned long lastAttempt = 0;
  const unsigned long retryEveryMs = 5000; // tenta a cada 5s sem travar o loop
  if (mqttClient.connected()) return;
  if (millis() - lastAttempt < retryEveryMs) return;
  lastAttempt = millis();

  Serial.print("Conectando ao MQTT...");
  String clientId = String("cardioia-esp32-") + String((uint32_t)ESP.getEfuseMac(), HEX);
  if (mqttClient.connect(clientId.c_str())) {
    Serial.println("conectado!");
  } else {
    Serial.print("falhou, rc=");
    Serial.println(mqttClient.state());
  }
}

// DEFINIÇÃO DE PINOS
#define RED 25     
#define GREEN 26   
#define BLUE 27   

#define DHTPIN 15        
#define DHTTYPE DHT22    
#define POTPIN 34        

// Criação do objeto DHT (sensor de temperatura e umidade)
DHT dht(DHTPIN, DHTTYPE);

// Variável de simulação da conexão Wi-Fi (true = online, false = offline)
bool conectado = false;

// Contador de batimentos (para futura integração com interrupções)
volatile int bpmCount = 0;

// FUNÇÃO PARA DEFINIR A COR DO LED RGB
void setColor(int red, int green, int blue) {
  digitalWrite(RED, red);
  digitalWrite(GREEN, green);
  digitalWrite(BLUE, blue);
}

// FUNÇÃO PARA ATUALIZAR O LED CONFORME OS SINAIS VITAIS 
void atualizarLED(float temperatura, float umidade, int bpm) {
// Define faixas normais para temperatura, batimentos e umidade
  bool tempNormal = (temperatura >= 36.5 && temperatura <= 37.5);
  bool bpmNormal = (bpm >= 60 && bpm <= 100);
  bool umidadeNormal = (umidade >= 30 && umidade <= 60);

// Se todos os parâmetros estiverem normais → LED verde
  if (tempNormal && bpmNormal && umidadeNormal) {
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, HIGH);
    Serial.println("🟢 Estado normal");
  }
// Se algum parâmetro estiver fora da faixa → LED amarelo (atenção)
  else if (
    (temperatura > 37.5 && temperatura <= 38.5) ||
    (bpm > 100 && bpm <= 120) ||
    (umidade < 30 || umidade > 60)
  ) {
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, HIGH);
    Serial.println("🟡 Estado de atenção");
  }
// Se algum parâmetro estiver crítico → LED vermelho
  else {
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);
    digitalWrite(BLUE, HIGH);
    Serial.println("🔴 Estado crítico");
  }
}

// FUNÇÃO DE ARMAZENAMENTO LOCAL (SPIFFS)
// Salva as leituras no arquivo "dados.txt" em formato JSON (uma linha por registro)
void salvarDados(float temperatura, float umidade, int bpm) {
  File arquivo = SPIFFS.open("/dados.txt", FILE_APPEND);
  if (!arquivo) {
    Serial.println("Erro ao abrir o arquivo para gravar!");
    return;
  }

  // Monta JSON idêntico ao usado em publishVitals
  StaticJsonDocument<256> doc;
  doc["temp"] = temperatura;
  doc["hum"] = umidade;
  doc["bpm"] = bpm;

  // Serializa para uma string e grava a linha no arquivo
  String payload;
  serializeJson(doc, payload);
  arquivo.println(payload);
  arquivo.close();
  Serial.println("Dados salvos localmente (JSON)!");
}

// FUNÇÃO PARA ENVIAR OS DADOS (Simulado)
// Lê o arquivo, imprime os dados no monitor serial e depois apaga o arquivo (simulação de envio à nuvem)
void enviarDados() {
  File arquivo = SPIFFS.open("/dados.txt");
  if (!arquivo) {
    Serial.println("Nenhum dado para enviar.");
    return;
  }
  if (arquivo.size() == 0) {
    arquivo.close();
    Serial.println("Arquivo vazio.");
    return;
  }

  Serial.println("\n=== ENVIANDO DADOS PARA A NUVEM (Simulado) ===");
  while (arquivo.available()) {
    Serial.write(arquivo.read());
  }
  arquivo.close();
  SPIFFS.remove("/dados.txt"); // Limpa o arquivo após o envio
  Serial.println("Dados enviados e arquivo local limpo.\n");
}

// Envia backlog offline do SPIFFS (cada linha é um JSON) via MQTT
void send_offline_data() {
  if (!mqttClient.connected()) return; // garante conexão MQTT

  File arquivo = SPIFFS.open("/dados.txt");
  if (!arquivo) {
    // Nada a enviar
    return;
  }
  if (arquivo.size() == 0) {
    arquivo.close();
    return;
  }

  Serial.println("\n=== Enviando backlog do SPIFFS via MQTT ===");
  bool allSent = true;
  while (arquivo.available()) {
    String line = arquivo.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      continue;
    }
    // Publica backlog com retain para que o último valor permaneça disponível
    if (!mqttClient.publish(MQTT_TOPIC, line.c_str(), true)) {
      Serial.println("Falha ao publicar uma linha do backlog. Tentará novamente depois.");
      allSent = false;
      break; // interrompe para não apagar o arquivo
    }
  }
  arquivo.close();

  if (allSent) {
    SPIFFS.remove("/dados.txt");
    Serial.println("Backlog enviado com sucesso e arquivo removido.\n");
  }
}

// Publica os sinais vitais no MQTT em formato JSON
void publishVitals(float temperatura, float umidade, int bpm) {
  // Captura e zera o contador de BPM (se estiver sendo usado em outra parte)
  int capturedBpmCount;
  noInterrupts();
  capturedBpmCount = bpmCount;
  bpmCount = 0;
  interrupts();

  StaticJsonDocument<256> doc;
  doc["temp"] = temperatura;
  doc["hum"] = umidade;
  doc["bpm"] = bpm;

  char payload[128];
  size_t n = serializeJson(doc, payload, sizeof(payload));
  bool ok = mqttClient.publish(MQTT_TOPIC, (uint8_t*)payload, n, true);
  Serial.println(ok ? " | MQTT: publicado" : " | MQTT: falha ao publicar");
}

// CONFIGURAÇÃO INICIAL
void setup() {
  Serial.begin(115200); 
  setup_wifi();
  // Configura o endereço do broker e porta do cliente MQTT
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  // Aumenta o buffer para garantir espaço nas mensagens JSON
  mqttClient.setBufferSize(256);
  dht.begin();          

// Define os pinos do LED RGB como saída
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

// Desliga todas as cores no início (por ser LED de anodo comum)
  setColor(HIGH, HIGH, HIGH);

// Inicializa o SPIFFS (sistema de arquivos interno do ESP32)
  if (!SPIFFS.begin(true)) {
    Serial.println("⚠️ SPIFFS não iniciado (esperado no Wokwi). Continuando em modo simulado.");
  } else {
    Serial.println("SPIFFS iniciado.");
  }

  Serial.println("Sistema CardioIA iniciado com sucesso!");
  Serial.println("Monitorando paciente cardiológico...\n");
}

// LOOP PRINCIPAL DO SISTEMA 
void loop() {
// Mantém e verifica conectividade Wi‑Fi/MQTT
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      reconnect_mqtt();
    }
    mqttClient.loop();
  }

// 1. Leitura dos sensores
  static float lastTemp = 36.8; // valores padrão para fallback
  static float lastHum  = 50.0;

  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  int potValue = analogRead(POTPIN);
  int bpm = map(potValue, 0, 4095, 40, 160); // Converte valor do potenciômetro em BPM (40–160)

// 2. Verifica se as leituras são válidas (usa último valor se der NaN)
  if (isnan(temperatura)) {
    Serial.print("DHT22: temp NaN, usando anterior -> ");
    Serial.println(lastTemp);
    temperatura = lastTemp;
  } else {
    lastTemp = temperatura;
  }

  if (isnan(umidade)) {
    Serial.print("DHT22: umid NaN, usando anterior -> ");
    Serial.println(lastHum);
    umidade = lastHum;
  } else {
    lastHum = umidade;
  }

// 3. Exibe os valores no monitor serial
  Serial.printf("Temp: %.2f°C | Umid: %.2f%% | BPM: %d", temperatura, umidade, bpm);

// 4. Atualiza o LED conforme o estado do paciente
  atualizarLED(temperatura, umidade, bpm);

// 5. Decisão de envio baseada no estado real do MQTT
  if (mqttClient.connected()) {
    // Primeiro, envia backlog offline
    send_offline_data();
    // Depois, publica a leitura atual
    Serial.println(" | Modo: Online - Publicando dados...");
    publishVitals(temperatura, umidade, bpm);
  } else {
    Serial.println(" | Modo: Offline - Salvando localmente...");
    salvarDados(temperatura, umidade, bpm); // Apenas salva localmente
  }

// 7. Intervalo entre coletas (5 segundos)
  delay(5000);
}
