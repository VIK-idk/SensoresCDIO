#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <ESP8266WiFi.h>

Adafruit_ADS1115 ads;

// Pines para los LEDs de humedad
const int ledLow = 14;   // LED para 0% - 33% (A1)
const int ledMid = 0;   // LED para 33% - 66% (A2)
const int ledHigh = 4;  // LED para 66% - 100% (A3)

// Variables globales para calibración de humedad
int16_t humedadCrudoSeco;   // Valor en seco
int16_t humedadCrudoMojado; // Valor en mojado

// Configuración Wi-Fi y servidor
const char WiFiSSID[] = "iClone de Joan";
const char WiFiPSK[] = "123qweasd";
const char Server_Host[] = "dweet.io";
const int Server_HttpPort = 80;
const String MyWriteAPIKey = "cdiocurso2024g06"; // Canal de Dweet.io
WiFiClient client;

// Pines para sensor de salinidad
#define power_pin 5 // Pin para alimentar el sensor de salinidad

// Definir los coeficientes del polinomio de Lagrange para salinidad
float a =  769.0 / 1130976000.0; // Coeficiente para x^4
float b = -222379.0 / 125664000.0; // Coeficiente para x^3
float c = 194295151.0 / 113097600.0; // Coeficiente para x^2
float d = -231396623.0 / 314160.0; // Coeficiente para x
float e = 4747356355.0 / 40392.0; // Término independiente

void connectWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(WiFiSSID, WiFiPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void enviarDatosNube(int humedadPorcentaje, float salinidad) {
  if (client.connect(Server_Host, Server_HttpPort)) {
    String PostData = "GET /dweet/for/" + MyWriteAPIKey + "?humedad=" + String(humedadPorcentaje) + "&salinidad=" + String(salinidad);
    client.print(PostData);
    client.println(" HTTP/1.1");
    client.println("Host: " + String(Server_Host));
    client.println("Connection: close");
    client.println();
    Serial.println("Datos enviados a la nube:");
    Serial.println(PostData);
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    client.stop();
  } else {
    Serial.println("Error conectando al servidor");
  }
}

void setup() {
  Serial.begin(9600);

  // Configuración del ADS1115
  if (!ads.begin()) {
    Serial.println("No se encontró el ADS1115. Verifica la conexión.");
    while (1);
  }
  ads.setGain(GAIN_ONE);
  Serial.println("ADS1115 iniciado correctamente.");

  pinMode(ledLow, OUTPUT);
  pinMode(ledMid, OUTPUT);
  pinMode(ledHigh, OUTPUT);
  
  pinMode(power_pin, OUTPUT);

  digitalWrite(ledLow, LOW);
  digitalWrite(ledMid, LOW);
  digitalWrite(ledHigh, LOW);

  connectWiFi();

  delay(5000);
  Serial.println("SECA EL SENSOR DE HUMEDAD TOTALMENTE");
  delay(5000);
  Serial.println("LEYENDO VALOR EN SECO...");
  humedadCrudoSeco = ads.readADC_SingleEnded(0);
  Serial.print("Valor seco: ");
  Serial.println(humedadCrudoSeco);

  Serial.println("PON EL SENSOR DE HUMEDAD EN EL AGUA HASTA LA LÍNEA");
  delay(5000);
  Serial.println("LEYENDO VALOR EN MOJADO...");
  humedadCrudoMojado = ads.readADC_SingleEnded(0);
  Serial.print("Valor mojado: ");
  Serial.println(humedadCrudoMojado);

  if (humedadCrudoSeco <= humedadCrudoMojado) {
    Serial.println("Error en la calibración. Asegúrate de que el sensor esté bien colocado.");
    while (1);
  }

  Serial.println("Calibración completada. Iniciando lectura...");
}

void loop() {
  int16_t humedadCrudo = ads.readADC_SingleEnded(0);

  int humedadPorcentaje = map(humedadCrudo, humedadCrudoMojado, humedadCrudoSeco, 100, 0);
  humedadPorcentaje = constrain(humedadPorcentaje, 0, 100);

  Serial.print("Valor crudo del ADC: ");
  Serial.print(humedadCrudo);
  Serial.print(" -> Humedad: ");
  Serial.print(humedadPorcentaje);
  Serial.println("%");

  if (humedadPorcentaje <= 33) {
    digitalWrite(ledLow, HIGH);
    digitalWrite(ledMid, LOW);
    digitalWrite(ledHigh, LOW);
  } else if (humedadPorcentaje <= 66) {
    digitalWrite(ledLow, LOW);
    digitalWrite(ledMid, HIGH);
    digitalWrite(ledHigh, LOW);
  } else {
    digitalWrite(ledLow, LOW);
    digitalWrite(ledMid, LOW);
    digitalWrite(ledHigh, HIGH);
  }

  int16_t adc0;

  digitalWrite(power_pin, HIGH);
  delay(100);
  adc0 = analogRead(A0);
  digitalWrite(power_pin, LOW);
  delay(100);

  float salinidad = a * pow(adc0, 4) + b * pow(adc0, 3) + c * pow(adc0, 2) + d * adc0 + e;

  Serial.print("Valor digital de salinidad = ");
  Serial.println(adc0);
  Serial.print("Salinidad en gramos = ");
  Serial.println(salinidad);

  enviarDatosNube(humedadPorcentaje, salinidad);

  delay(1000);
}
