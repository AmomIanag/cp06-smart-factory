
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

// ===== CONFIGURACOES DO CIRCUITO =====

const int PINO_DHT = 15;
const int LED_VERDE = 25;
const int LED_VERMELHO = 26;

const float LIMITE_TEMPERATURA = 35.0;

// ===== CONFIGURACOES WI-FI =====

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_SENHA = "";

// ===== CONFIGURACOES MQTT =====

const char* SERVIDOR_MQTT = "broker.hivemq.com";
const int PORTA_MQTT = 1883;


const char* TOPICO_MQTT =
  "edge/2ano/2CCPH/superequipe/cp06-a7f92c/dados";

// ===== OBJETOS =====

DHTesp dht;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

String idClienteMQTT;

unsigned long ultimaPublicacao = 0;
unsigned long ultimaTentativaMQTT = 0;

const unsigned long INTERVALO = 5000;

// ===== CONFIGURACAO INICIAL =====

void setup() {
  Serial.begin(115200);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);

  dht.setup(PINO_DHT, DHTesp::DHT22);

  // Conectar a rede virtual do Wokwi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA, 6);

  // ID diferente do cliente que sera usado pela API
  randomSeed(micros());
  idClienteMQTT =
    "cp06-esp32-" + String(random(0xFFFFFF), HEX);

  mqtt.setServer(SERVIDOR_MQTT, PORTA_MQTT);

  Serial.println("Smart Factory iniciada!");
  Serial.println("Iniciando conexao Wi-Fi...");
}

// ===== CONEXAO MQTT =====

void conectarMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (mqtt.connected()) {
    return;
  }

  // Evita tentativas excessivas de conexao
  if (millis() - ultimaTentativaMQTT < 3000) {
    return;
  }

  ultimaTentativaMQTT = millis();

  Serial.println("Conectando ao HiveMQ...");

  if (mqtt.connect(idClienteMQTT.c_str())) {
    Serial.println("MQTT conectado!");
    Serial.print("Topico: ");
    Serial.println(TOPICO_MQTT);
  } else {
    Serial.print("Falha MQTT, codigo: ");
    Serial.println(mqtt.state());
  }
}

// ===== LEITURA E PROCESSAMENTO LOCAL =====

void processarLeitura() {
  TempAndHumidity dados = dht.getTempAndHumidity();

  float temperatura = dados.temperature;
  float umidade = dados.humidity;

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler DHT22!");
    return;
  }

  const char* estado;

  // Decisao ocorre localmente no ESP32
  if (temperatura > LIMITE_TEMPERATURA) {
    estado = "ALERTA";

    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_VERDE, LOW);
  } else {
    estado = "NORMAL";

    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_VERMELHO, LOW);
  }

  // Criar mensagem JSON
  char mensagem[160];

  snprintf(
    mensagem,
    sizeof(mensagem),
    "{\"temperatura\":%.1f,\"umidade\":%.1f,\"estado\":\"%s\"}",
    temperatura,
    umidade,
    estado
  );

  Serial.println("--------------------------");
  Serial.print("Leitura: ");
  Serial.println(mensagem);

  // Publicar somente se estiver conectado
  if (mqtt.connected()) {
    bool enviado = mqtt.publish(TOPICO_MQTT, mensagem);

    if (enviado) {
      Serial.println("Mensagem MQTT publicada!");
    } else {
      Serial.println("Falha na publicacao MQTT!");
    }
  } else {
    Serial.println("MQTT offline. Leitura processada localmente.");
  }
}

// ===== EXECUCAO CONTINUA =====

void loop() {
  static bool wifiConectado = false;

  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiConectado) {
      Serial.println("Wi-Fi conectado!");
      wifiConectado = true;
    }
  } else {
    wifiConectado = false;
  }

  conectarMQTT();

  if (mqtt.connected()) {
    mqtt.loop();
  }

  // Processar e publicar a cada 5 segundos
  if (millis() - ultimaPublicacao >= INTERVALO) {
    ultimaPublicacao = millis();
    processarLeitura();
  }

  delay(10);
}
