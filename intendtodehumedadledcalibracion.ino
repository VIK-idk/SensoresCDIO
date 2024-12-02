#include <Wire.h>
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;

// Pines para los LEDs
const int ledLow = 5;   // LED para 0% - 33% (A1)
const int ledMid = 0;   // LED para 33% - 66% (A2)
const int ledHigh = 4;  // LED para 66% - 100% (A3)

// Variables globales para calibración
int16_t humedadCrudoSeco;   // Valor en seco
int16_t humedadCrudoMojado; // Valor en mojado

void setup() {
  Serial.begin(9600);

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

  // Asegurarnos de que los LEDs estén apagados al inicio
  digitalWrite(ledLow, LOW);
  digitalWrite(ledMid, LOW);
  digitalWrite(ledHigh, LOW);
  delay(5000);
  // Instrucciones de calibración
  Serial.println("SECA EL SENSOR TOTALMENTE");
  delay(5000); // Esperar 5 segundos para que el usuario se prepare
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  Serial.println("LEYENDO VALOR EN SECO...");
  delay(5000);
  humedadCrudoSeco = ads.readADC_SingleEnded(0);
  Serial.print("Valor seco: ");
  Serial.println(humedadCrudoSeco);
  Serial.println("PON EL SENSOR EN EL AGUA HASTA LA LÍNEA");
  delay(5000);
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);// Esperar 5 segundos para que el usuario coloque el sensor en el agua
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

void loop() {
  // Leer el valor crudo del canal A0
  int16_t humedadCrudo = ads.readADC_SingleEnded(0);

  // Mapear el valor crudo al rango de 0-100 para el porcentaje de humedad
  int humedadPorcentaje = map(humedadCrudo, humedadCrudoMojado, humedadCrudoSeco, 100, 0);

  // Limitar el rango a 0-100%
  humedadPorcentaje = constrain(humedadPorcentaje, 0, 100);

  // Mostrar los valores en el monitor serie para depuración
  Serial.print("Valor crudo del ADC: ");
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

  delay(1000); // Esperar antes de la siguiente lectura
}
