#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <ESP8266WiFi.h>
Adafruit_ADS1115 ads;
// Pines para los LEDs
const int ledLow = 5;   // LED para 0% - 33% (A1)
const int ledMid = 0;   // LED para 33% - 66% (A2)
const int ledHigh = 4;  // LED para 66% - 100% (A3)
 //Pin para sensor salinidad

// Variables globales para calibración
int16_t humedadCrudoSeco;   // Valor en seco
int16_t humedadCrudoMojado; // Valor en mojado

// Configuración Wi-Fi y servidor
const char WiFiSSID[] = "iClone de Joan";
const char WiFiPSK[] = "123qweasd";
const char Server_Host[] = "dweet.io";
const int Server_HttpPort = 80;
const String MyWriteAPIKey = "cdiocurso2024g06"; // Canal de Dweet.io
WiFiClient client;

#define power_pin 15 
const int sal = 5; 
float valorDigital[sal] = {462, 530, 620, 720, 960}; 
float gramosSal[sal] = {0, 5, 10, 15, 20 };         

int choice; // Variable para la selección del sensor

void connectWiFi() {
  byte ledStatus = LOW;
  Serial.print("Conectando a WiFi");
  WiFi.begin(WiFiSSID, WiFiPSK);
  while (WiFi.status() != WL_CONNECTED) {
    // Mostrar progreso de conexión
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
    // Leer respuesta del servidor
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

  // Selección del sensor
  Serial.println("Seleccione el sensor a utilizar:");
  Serial.println("1. Sensor de Humedad");
  Serial.println("2. Sensor de Salinidad");
  while (Serial.available() == 0) {}  // Esperar entrada
  choice = Serial.parseInt();

  // Configuración del ADS1115
  if (!ads.begin()) {
    Serial.println("No se encontró el ADS1115. Verifica la conexión.");
    while (1);
  }
  ads.setGain(GAIN_ONE); // Configuramos el rango de entrada a ±4.096V
  Serial.println("ADS1115 iniciado correctamente.");

  // Configurar pines de los LEDs como salida
  pinMode(ledLow, OUTPUT);
  pinMode(ledMid, OUTPUT);
  pinMode(ledHigh, OUTPUT);
  pinMode(power_pin, OUTPUT);

  // Asegurarnos de que los LEDs estén apagados al inicio
  digitalWrite(ledLow, LOW);
  digitalWrite(ledMid, LOW);
  digitalWrite(ledHigh, LOW);

  // Conectar a Wi-Fi
  connectWiFi();

  if (choice == 1) {
    // Calibración del sensor de humedad
    delay(5000);
    Serial.println("SECA EL SENSOR TOTALMENTE");
    delay(5000);
    Serial.println("LEYENDO VALOR EN SECO...");
    humedadCrudoSeco = ads.readADC_SingleEnded(0);
    Serial.print("Valor seco: ");
    Serial.println(humedadCrudoSeco);

    Serial.println("PON EL SENSOR EN EL AGUA HASTA LA LÍNEA");
    delay(5000);
    Serial.println("LEYENDO VALOR EN MOJADO...");
    humedadCrudoMojado = ads.readADC_SingleEnded(0);
    Serial.print("Valor mojado: ");
    Serial.println(humedadCrudoMojado);

    // Verificar si la calibración tiene sentido
    if (humedadCrudoSeco <= humedadCrudoMojado) {
      Serial.println("Error en la calibración. Asegúrate de que el sensor esté bien colocado.");
      while (1);
    }

    Serial.println("Calibración completada. Iniciando lectura...");
  }

}

float calcularSalinidad(int adc0){
  float salinidad=0.0;

  for (int i=0; i<sal; i++) {
   float Li = 1.0;
   for (int j=0; j<sal; j++) {
     if (j != i) {
      Li *= (adc0- valorDigital[j]) / (valorDigital[i] - valorDigital[j]);   //Lagrange
      }
    }
    salinidad += gramosSal[i] * Li;
  }

  if (adc0 < valorDigital[0]) {
    salinidad = 0.0;
  }

  return salinidad;
}

void loop() {
  if (choice == 1) {
    // Leer el valor crudo del canal A0 para humedad
    int16_t humedadCrudo = ads.readADC_SingleEnded(0);

    // Mapear el valor crudo al rango de 0-100 para el porcentaje de humedad
    int humedadPorcentaje = map(humedadCrudo, humedadCrudoMojado, humedadCrudoSeco, 100, 0);

    // Limitar el rango a 0-100%
    humedadPorcentaje = constrain(humedadPorcentaje, 0, 100);

    // Mostrar los valores en el monitor serie para depuración
    Serial.print("Valor crudo del ADC (Humedad): ");
    Serial.print(humedadCrudo);
    Serial.print(" -> Humedad: ");
    Serial.print(humedadPorcentaje);
    Serial.println("%");

    // Lógica para controlar los LEDs
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

    // Enviar los datos a la nube
    enviarDatosNube(humedadPorcentaje, 0.0); // Enviar solo humedad
  } else if (choice == 2) {
    // Leer el valor digital del sensor de salinidad
    int16_t adc0;
    digitalWrite(power_pin, HIGH);
    delay(100);
    adc0 = analogRead(A0);
    digitalWrite(power_pin, LOW);
    delay(100);

    Serial.print("Lectura digital = ");
    Serial.println(adc0, DEC);

    // Convertir a gramos de sal usando el polinomio de Lagrange
    float salinidad = calcularSalinidad(adc0);
    Serial.print("Gramos de sal = ");
    Serial.println(salinidad, 2); 

    // Enviar los datos a la nube
    enviarDatosNube(0, salinidad); // Enviar solo salinidad
  }

  delay(1000); // Esperar 1 segundo antes de la siguiente lectura
}
