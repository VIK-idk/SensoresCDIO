/*****************************************************************************
    Practica 5: ADs1115 + Sensor de Temperatura
*****************************************************************************/

// Incluir la librería Adafruit_ADS1015.h
#include <Adafruit_ADS1X15.h>
#include <ESP8266WiFi.h>

// Definir el Canal del ADS1115 por el que leeremos
// la tensión del sensor de Temperatura
#define channelValue 1; // Usaremos el canal 0 (A0) del ADS1115

// Construimos el ADS1115
Adafruit_ADS1115 ads; // Crea un objeto para manejar el ADS1115

void setup() {
  // Inicializamos el monitor serie
  Serial.begin(9600);
  Serial.println("Inicializando el medidor de Temperatura");

  // Inicializamos el ADS1115
  if (!ads.begin()) { // Verifica que el ADS1115 esté conectado y responda
    Serial.println("Error: ADS1115 no encontrado");
    while (1); // Detenemos el programa si no se detecta
  }

  // Configuramos la ganancia del ADS1115
  ads.setGain(GAIN_ONE); // Configuración de ganancia de 1x (±4.096V)
  // Usamos GAIN_ONE ya que la mayoría de sensores de temperatura operan en un rango de 0-3.3V
}

void loop() {
  // Captura una muestra del ADS1115
  int16_t adc0 = ads.readADC_SingleEnded(1); 
  // `readADC_SingleEnded` permite leer valores en el canal especificado (0-3)

  // Convertimos la lectura digital a voltaje
  float voltage = adc0 * 0.125 / 1000.0; 
  // Cada bit del ADS1115 (modo 16 bits) representa 0.125 mV en GAIN_ONE

  // Calculamos la temperatura (ejemplo: sensor LM35, donde 10mV = 1°C)
  float temperatura = voltage * 100.0; // Para LM35: voltaje (V) * 100 = °C

  // Mostramos la temperatura en el monitor serie
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  delay(1000); // Espera 1 segundo antes de leer de nuevo
}
