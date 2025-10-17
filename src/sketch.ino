#include <Arduino.h>
#include <DHT.h>
#include "SPIFFS.h"

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
// Salva as leituras no arquivo "dados.txt" dentro do sistema de arquivos do ESP32.
void salvarDados(float temperatura, float umidade, int bpm) {
  File arquivo = SPIFFS.open("/dados.txt", FILE_APPEND);
  if (!arquivo) {
    Serial.println("Erro ao abrir o arquivo para gravar!");
    return;
  }
// Salva os dados em formato de texto simples com timestamp (millis)
  arquivo.printf("%lu,Temp:%.2f,Umid:%.2f,BPM:%d\n", (unsigned long)millis(), temperatura, umidade, bpm);
  arquivo.close();
  Serial.println("Dados salvos localmente!");
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

// CONFIGURAÇÃO INICIAL
void setup() {
  Serial.begin(115200); 
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
// 1. Leitura dos sensores
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  int potValue = analogRead(POTPIN);
  int bpm = map(potValue, 0, 4095, 40, 160); // Converte valor do potenciômetro em BPM (40–160)

// 2. Verifica se as leituras são válidas
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler o DHT22!");
    delay(2000);
    return;
  }

// 3. Exibe os valores no monitor serial
  Serial.printf("Temp: %.2f°C | Umid: %.2f%% | BPM: %d", temperatura, umidade, bpm);

// 4. Atualiza o LED conforme o estado do paciente
  atualizarLED(temperatura, umidade, bpm);

// 5. Verifica se está "conectado" (simulação)
  if (conectado) {
    Serial.println(" | Modo: Online - Enviando dados pendentes e atuais...");
    enviarDados(); // Envia dados salvos
    salvarDados(temperatura, umidade, bpm); // Salva nova leitura
    enviarDados(); // Envia leitura atual
  } else {
    Serial.println(" | Modo: Offline - Salvando localmente...");
    salvarDados(temperatura, umidade, bpm); // Apenas salva localmente
  }

// 6. Simula alternância do estado de conexão (a cada 5 ciclos)
  static int contador = 0;
  contador++;
  if (contador % 5 == 0) {
    conectado = !conectado;
    Serial.print("\n[AVISO] Mudando estado da conexão para: ");
    Serial.println(conectado ? "ONLINE" : "OFFLINE");
  }

// 7. Intervalo entre coletas (5 segundos)
  delay(5000);
}
