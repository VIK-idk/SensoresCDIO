/*Sensor PH*/

#include <Adafruit_ADS1015.h>  // Librería para el ADS1115

// Definir el canal del ADS1115
#define channelValue 0         // Canal donde está conectado el sensor
#define Offset 0.0             // Offset del sensor
#define samplingInterval 20    // Intervalo de muestreo en ms
#define printInterval 800      // Intervalo de impresión en ms
#define ArrayLength 40         // Número de muestras para el promedio

float pHArray[ArrayLength];    // Almacena las muestras
uint8_t pHArrayIndex = 0;      // Índice del array

Adafruit_ADS1115 ads;          // Instancia del ADS1115

void setup() {
  Serial.begin(9600);          // Inicia el monitor serie
  Serial.println("Iniciando el medidor de pH...");
  
  ads.setGain(GAIN_ONE);       // Configura la ganancia del ADS1115
  ads.begin();                 // Inicia el ADS1115
}

void loop() {
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();

  if (millis() - samplingTime >= samplingInterval) {
    samplingTime = millis();
    
    // Leer el valor del ADS1115
    int16_t adcValue = ads.readADC_SingleEnded(channelValue);
    float voltage = adcValue * 0.1875 / 1000; // Conversión a voltaje (mV a V, con ganancia 1)
    
    // Almacenar en el array de pH
    pHArray[pHArrayIndex++] = voltage;
    if (pHArrayIndex == ArrayLength) pHArrayIndex = 0;
  }

  if (millis() - printTime >= printInterval) {
    printTime = millis();
    
    // Promedio del voltaje
    float avgVoltage = 0;
    for (int i = 0; i < ArrayLength; i++) avgVoltage += pHArray[i];
    avgVoltage /= ArrayLength;
    
    // Conversión a pH
    float pHValue = 3.5 * avgVoltage + Offset;
    
    // Imprimir resultados
    Serial.print("Voltaje promedio: ");
    Serial.print(avgVoltage, 3);
    Serial.print(" V, pH: ");
    Serial.println(pHValue, 2);
  }
}
