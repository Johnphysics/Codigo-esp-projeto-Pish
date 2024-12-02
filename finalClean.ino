#include <arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HCSR04.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
// Definição de pinos para o display TFT
#define TFT_CS 5    // Chip select conectado ao pino 5
#define TFT_RST 15  // Reset conectado ao pino 15
#define TFT_DC 32   // A0 conectado ao pino 32
#define TFT_MOSI 23 // Dados conectados ao pino 23
#define TFT_SCLK 18 // Clock conectado ao pino 18


#define DHTPIN 33     // Pino de dados do DHT22
#define DHTTYPE DHT22 
#define D2 4
#define D3 0
#define pino_trigger 14
#define pino_echo 12
#define TOPICO_UMIDADE "UMIDADE"
#define TOPICO_TEMPERATURA "TEMPERATURA"
#define TOPICO_MONOXIDO "MONOXIDO"
#define TOPICO_DIOXIDO "DIOXIDO"
#define ID_MQTT "b9112678-efad-444f-aa37-bbab4953f5db"
double monoxido = 10;
double dioxido = 12;
double umidade = 14;
double temperatura = 16; 
int mq135 = 35; 
int mq9 = 34; //
int valorco;
int valorco2;

double caixa = 100;
double distancia;
double reserva;
char str[3];
char SSID[] = "TECTOY";
char PASSWORD[] = "senha180990";
const char *BROKER_MQTT = "test.mosquitto.org";
int BROKER_PORT = 1883;
String registrosOffline = ""; // Variável para armazenar registros quando offline
// Inicializa o sensor DHT
DHT dht(DHTPIN, DHTTYPE);

int a = 0;

WiFiClient espClient;
PubSubClient MQTT(espClient);
long numAleatorio;
UltraSonicDistanceSensor distanceSensor(pino_trigger, pino_echo);

unsigned long previousMillis = 0;
const long interval = 5000; // Intervalo para verificar a distância e publicar mensagens MQTT

void initWiFi(void);
void initMQTT(void);
void mqtt_callback(char *topic, byte *payload, unsigned int length);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);


void initWiFi(void)
{
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");
  reconnectWiFi();
}

void initMQTT(void)
{
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  String msg;
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
  }
}

void reconnectMQTT(void)
{
  while (!MQTT.connected())
  {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT))
    {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_MONOXIDO);
      MQTT.subscribe(TOPICO_DIOXIDO);
      MQTT.subscribe(TOPICO_TEMPERATURA);
      MQTT.subscribe(TOPICO_UMIDADE);
    }
    else
    {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      delay(2000);
    }
  }
}

void VerificaConexoesWiFIEMQTT(void)
{
  if (!MQTT.connected())
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
  reconnectWiFi();    //se não há conexão com o WiFI, a conexão é refeita
}

void reconnectWiFi(void)
{
  if (WiFi.status() == WL_CONNECTED)
    return;
  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("\nIP obtido: ");
  Serial.println(WiFi.localIP());
}

// Inicializa o display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void setup()
{
  Serial.begin(9600); //Enviar e receber dados em 9600 baud
  delay(1000);
//  pinMode(D2, OUTPUT);
//  pinMode(16, OUTPUT);
//  pinMode(5, OUTPUT);
  dht.begin();
  initWiFi();
  initMQTT();
  
    // Inicializa o display
  tft.initR(INITR_BLACKTAB); // Configuração padrão para display ST7735
  tft.fillScreen(ST7735_BLACK); // Limpa a tela
  tft.setRotation(0); // Define orientação (0 para retrato)
}


void exibirDados(int a, int b, int c, int d) {
  // Define as posições verticais para cada linha de texto
  int linha1 = 10;
  int linha2 = 50;
  int linha3 = 90;
  int linha4 = 130;
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);  

  // Exibe as linhas horizontais para separação
  tft.drawLine(0, linha2 - 10, 128, linha2 - 10, ST7735_WHITE);
  tft.drawLine(0, linha3 - 10, 128, linha3 - 10, ST7735_WHITE);
  tft.drawLine(0, linha4 - 10, 128, linha4 - 10, ST7735_WHITE);

  // Exibe o primeiro dado
  if(a < 400){
  tft.setCursor(5, linha1);
  tft.setTextSize(2); // Tamanho grande para o texto principal
  tft.print("CO ");
  tft.print(a);
  tft.setTextSize(1); // Tamanho menor para "ppm"
  tft.print("ppm");
  }else{
  tft.setTextColor(ST7735_RED);    
  tft.setCursor(5, linha1);
  tft.setTextSize(2); // Tamanho grande para o texto principal
  tft.print("CO ");
  tft.print(a);
  tft.setTextSize(1); // Tamanho menor para "ppm"
  tft.print("ppm");
  tft.setTextColor(ST7735_WHITE); 
  }
  // Exibe a barra vertical ao lado direito
  desenharBarraVertical(115, linha1 - 5, linha2 - 15, a/4); // Barra vertical (12% preenchida)

  // Exibe o segundo dado
  if(b < 1000){
  tft.setCursor(5, linha2);
  tft.setTextSize(2);
  tft.print("CO");
  tft.setTextSize(1);
  tft.print("2");
  tft.setTextSize(2);
  tft.print(" ");
  tft.print(b);
  tft.setTextSize(1);
  tft.print("ppm");
  }else{
  tft.setTextColor(ST7735_RED);    
  tft.setCursor(5, linha2);
  tft.setTextSize(2);
  tft.print("CO");
  tft.setTextSize(1);
  tft.print("2");
  tft.setTextSize(2);
  tft.print(" ");
  tft.print(b);
  tft.setTextSize(1);
  tft.print("ppm");
  tft.setTextColor(ST7735_WHITE); 
  }


  // Exibe a barra vertical ao lado direito
  desenharBarraVertical(115, linha2 - 5, linha3 - 15, b/10); // Barra vertical (75% preenchida)

  // Exibe o terceiro dado
  tft.setCursor(10, linha3);
  tft.setTextSize(2);
  tft.print("UMI ");
  tft.print(c);
  tft.setTextSize(1);
  tft.print("%");

  // Exibe a barra vertical ao lado direito
  desenharBarraVertical(115, linha3 - 5, linha4 - 15, c); // Barra vertical (55% preenchida)

  // Exibe o quarto dado
  tft.setCursor(10, linha4);
  tft.setTextSize(2);
  tft.print("TEMP ");
  tft.print(d);
  tft.setTextSize(1);
  tft.print("C");

  // Exibe a barra vertical ao lado direito
  desenharBarraVertical(115, linha4 - 5, 155, d*2); // Barra vertical (25% preenchida)
}

uint16_t determinarCor(int nivel) {
  // Retorna a cor com base no nível
  if (nivel <= 30) {
    return ST7735_GREEN; // Verde para níveis baixos
  } else if (nivel <= 70) {
    return ST7735_YELLOW; // Amarelo para níveis moderados
  } else {
    return ST7735_RED; // Vermelho para níveis altos
  }
}

void desenharBarraVertical(int x, int yTop, int yBottom, int nivel) {
  if(nivel > 100)
  nivel = 100;
  int barraMaxHeight = yBottom - yTop;                  // Altura máxima da barra
  int barraHeight = map(nivel, 0, 100, 0, barraMaxHeight); // Ajusta a altura da barra com base no nível
  int barraY = yBottom - barraHeight;                  // Calcula a posição inicial da barra (crescendo para cima)

  uint16_t cor = determinarCor(nivel);

  // Determina a cor da barra
  if(yTop == 85)
  cor = ST7735_BLUE;
  
  // Desenha a barra preenchida
  tft.fillRect(x, barraY, 10, barraHeight, cor);

  // Adiciona contorno branco para a barra
  tft.drawRect(x, yTop, 10, barraMaxHeight, ST7735_WHITE);
}


void loop() {

  int valorAnalogico = analogRead(mq9);  // Lê o valor analógico do pino
  int valorAnalogico2 = analogRead(mq135); 

  
 if(valorAnalogico2 < 0)
  valorAnalogico2 = 350;
  else
  valorAnalogico2 += 350;

  valorco2 = valorAnalogico2;
  valorco = valorAnalogico;

 if(valorco2 > 600)
  valorco2 = valorAnalogico2 * 5;
  
  if(valorAnalogico > 100)
  valorco = valorAnalogico * 3;

  
VerificaConexoesWiFIEMQTT();
    MQTT.loop();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  MQTT.publish(TOPICO_UMIDADE, String(humidity).c_str());
  MQTT.publish(TOPICO_TEMPERATURA, String(temperature).c_str());
  MQTT.publish(TOPICO_MONOXIDO, String(valorco).c_str());
  MQTT.publish(TOPICO_DIOXIDO, String(valorco2).c_str());

  exibirDados(valorco,valorco2,humidity,temperature);
    Serial.print("CO ");
  Serial.println(valorco);
    Serial.print("CO2 ");
  Serial.println(valorco2);
  delay(5000);
  }
