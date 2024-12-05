#include <Arduino.h>
#include "config.h"
#include "sensor_humedad.h"
#include "sensor_salinidad.h"

void setup() {
  Serial.begin(9600);
  connectWiFi();           // Conectar al Wi-Fi
  iniciarSensorHumedad();  // Inicializar el sensor de humedad
  iniciarSensorSalinidad();// Inicializar el sensor de salinidad
}

void loop() {
  // Leer y procesar datos del sensor de humedad
  int humedadPorcentaje = leerHumedad();
  
  // Leer y procesar datos del sensor de salinidad
  float salinidad = leerSalinidad();

  // Enviar datos al servidor
  enviarDatosNube(humedadPorcentaje, salinidad);

  delay(1000); // Espera 1 segundo antes de la próxima iteración
}
