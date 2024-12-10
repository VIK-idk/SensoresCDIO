/*****************************************************************************
    Gestión de Luminosidad: ADS1115 + Sensor de Luz + Noche/Día
*****************************************************************************/

#include <Adafruit_ADS1X15.h>

// Inicializamos el objeto ADS1115
Adafruit_ADS1115 ads;

// Definimos un umbral para determinar si es de noche o día (ajústalo según tu sensor)
#define UMBRAL_LUMINOSIDAD_NOCHE 50 // Valor en lux por debajo del cual se considera "noche"

void setup() {
  // Inicializamos el monitor serie
  Serial.begin(9600);
  Serial.println("Inicializando sensor de luz");

  // Inicializamos el ADS1115
  if (!ads.begin()) { 
    Serial.println("Error: ADS1115 no encontrado");
    while (1); // Detenemos si no se detecta el ADS1115
  }

  // Configuramos la ganancia del ADS1115
  ads.setGain(GAIN_ONE); // ±4.096V, ideal para rangos de sensores comunes
}

void loop() {
  // Leer el valor del sensor de luz (suponemos conectado al canal 0)
  int16_t adcValue = ads.readADC_SingleEnded(0);

  // Convertir el valor ADC a voltaje
  float voltage = adcValue * 0.125 / 1000.0; // 0.125 mV por bit en GAIN_ONE

  // Convertir el voltaje a luminosidad (lux)
  // Ejemplo genérico: Lux = (Voltaje / Vmax) * Lux_max
  // Adapta según el sensor utilizado. Aquí asumimos Vmax = 3.3V, Lux_max = 1000
  float luminosidad = (voltage / 3.3) * 1000.0;

  // Llamar a la función nocheDia para determinar si es de noche o día
  nocheDia(luminosidad);

  delay(1000); // Leer cada segundo
}

// Función para determinar si es de noche o día
void nocheDia(float luminosidad) {
  if (luminosidad < UMBRAL_LUMINOSIDAD_NOCHE) {
    Serial.println("Es de noche.");
  } else {
    Serial.println("Es de día.");
  }
  // Mostrar la lectura actual de luminosidad
  Serial.print("Luminosidad actual: ");
  Serial.print(luminosidad);
  Serial.println(" lux");
}
