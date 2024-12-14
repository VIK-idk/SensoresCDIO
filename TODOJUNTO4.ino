#include <Wire.h>                       // Biblioteca para la comunicación I2C
#include <Adafruit_ADS1X15.h>           // Biblioteca para el convertidor ADC ADS1115
#include <cmath>                        // Biblioteca matemática para cálculos

//------------------------------- Definiciones de pines y variables globales -----------------------------------
#define CANAL_HUMEDAD 1         // Canal del ADS1115 asignado al sensor de humedad
#define PIN_ENERGIA 5           // Pin para alimentar el sensor de salinidad
#define CANAL_TEMPERATURA 0     // Canal del ADS1115 asignado al sensor de temperatura
#define CANAL_LUMINOSIDAD 2     // Canal del ADS1115 asignado al sensor de luminosidad

int seleccion = 0;              // Variable para la opción seleccionada en el menú

Adafruit_ADS1115 ads;           // Crear objeto ADS1115 para manejar el ADC

// Calibración y valores de humedad
int humedadSeco = 20200;        // Valor de referencia cuando el sensor está seco
int humedadMojado = 9800;       // Valor de referencia cuando el sensor está mojado
int lecturaHumedad = 0;         // Variable para la lectura actual del sensor de humedad
int porcentajeHumedad = 0;      // Porcentaje de humedad calculado

// Relación analítica para la calibración de humedad (pendiente y punto de intersección)
double pendienteHumedad = -0.0096; 
double interseccionHumedad = 194.23;   

// Variables para el cálculo de temperatura
float pendienteTemperatura = 35e-3;    
float voltajeReferencia = 0.79;         
float rangoMaximoVoltaje = 4.096;      

// Luminosidad
#define UMBRAL_NOCHE 50        // Límite de lux para determinar si es de noche
int lecturaLuminosidad = 0;    

//------------------------------------------ Setup ------------------------------------------------------
void setup() {
  Serial.begin(9600);                      // Configurar comunicación serial
  ads.setGain(GAIN_ONE);                   // Configuración de ganancia (1x: -4.096V a +4.096V)
  ads.begin(0x48);                         // Inicializar el ADS1115 con dirección I2C 0x48
  pinMode(PIN_ENERGIA, OUTPUT);            // Configurar el pin para controlar el sensor de salinidad

  realizarCalibracionHumedad();            // Llamada a la función para calibrar el sensor de humedad
}

//------------------------------------- Funciones de utilidad ------------------------------------------
// Esta función permite pausar la ejecución hasta que el usuario ingrese un comando esperado
void esperarIngreso(String comandoEsperado) {
  while (true) {
    if (Serial.available() > 0) {                   // Verificar si hay datos disponibles
      String comando = Serial.readStringUntil('\n'); // Leer hasta el salto de línea
      comando.trim();                               // Eliminar espacios o caracteres innecesarios
      if (comando.equalsIgnoreCase(comandoEsperado)) {
        break;                                      // Salir del bucle si el comando coincide
      }
    }
  }
}

//------------------------------------- Calibración de humedad -----------------------------------------
void realizarCalibracionHumedad() {
  Serial.println("Inicio del proceso de calibración del sensor de humedad.");

  // Paso 1: Calibración en seco
  Serial.println("Coloque el sensor en seco y escriba 'OK'.");
  esperarIngreso("OK");   // Espera al usuario
  humedadSeco = ads.readADC_SingleEnded(CANAL_HUMEDAD); // Leer valor en seco
  Serial.println("Calibración en seco completada.");

  // Paso 2: Calibración en mojado
  Serial.println("Coloque el sensor en mojado y escriba 'OK'.");
  esperarIngreso("OK");   // Espera al usuario
  humedadMojado = ads.readADC_SingleEnded(CANAL_HUMEDAD); // Leer valor en mojado
  Serial.println("Calibración en mojado completada.");

  // Mostrar resultados
  Serial.println("Valores ajustados:");
  Serial.print("Seco: ");
  Serial.println(humedadSeco);
  Serial.print("Mojado: ");
  Serial.println(humedadMojado);
}

//------------------------------------- Lectura de humedad ---------------------------------------------
void obtenerHumedad() {
  lecturaHumedad = ads.readADC_SingleEnded(CANAL_HUMEDAD); // Leer el valor del sensor de humedad
  porcentajeHumedad = map(lecturaHumedad, humedadSeco, humedadMojado, 0, 100); // Escalar a porcentaje

  // Mostrar los resultados
  Serial.print("Humedad actual: ");
  Serial.print(porcentajeHumedad);
  Serial.println(" %");
  delay(1000);
  Serial.print("Valor digital crudo: ");
  Serial.println(lecturaHumedad);
  delay(1000);
}
//----------------------------------- Medición de temperatura -----------------------------------------
double calcularPromedio(double* muestras, int totalMuestras) {
  double suma = 0.0;                            // Variable para acumular las lecturas
  for (int i = 0; i < totalMuestras; i++) {
    muestras[i] = ads.readADC_SingleEnded(CANAL_TEMPERATURA); // Leer cada muestra
    suma += muestras[i];                       // Sumar las lecturas
  }
  return suma / totalMuestras;                 // Retornar el promedio
}

void obtenerTemperatura() {
  const int muestrasTotales = 30;              // Número de muestras para promediar
  double muestras[muestrasTotales];            // Array para almacenar las muestras
  double promedio = calcularPromedio(muestras, muestrasTotales); // Calcular el promedio

  Serial.print("Promedio de lecturas: ");
  Serial.println(promedio);

  float voltaje = (promedio * rangoMaximoVoltaje) / 32767; // Convertir el valor ADC a voltaje
  float temperatura = (voltaje - voltajeReferencia) / pendienteTemperatura; // Ecuación de temperatura

  // Mostrar resultados
  Serial.print("Voltaje: ");
  Serial.println(voltaje);
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" ºC");
  delay(1000);
}

//----------------------------------- Medición de salinidad -------------------------------------------
void obtenerSalinidad() {
  int16_t valorADC;                         // Variable para almacenar la lectura digital
  digitalWrite(PIN_ENERGIA, HIGH);          // Encender el sensor de salinidad
  delay(100);                               // Esperar estabilización
  valorADC = analogRead(0);                 // Leer valor del sensor
  digitalWrite(PIN_ENERGIA, LOW);           // Apagar el sensor
  delay(100);

  // Aplicación de interpolación de Lagrange para estimar salinidad
  double puntosADC[] = {920, 930, 960, 940, 945};
  double valores[] = {5, 10, 15, 20, 25};
  double salinidad = 0.0;

  for (int i = 0; i < 5; i++) {
    double producto = valores[i];
    for (int j = 0; j < 5; j++) {
      if (i != j) {
        producto *= (valorADC - puntosADC[j]) / (puntosADC[i] - puntosADC[j]);
      }
    }
    salinidad += producto;
  }

  // Ajustar límites de salinidad
  if (valorADC < 850) salinidad = 0;
  if (valorADC >= 1000) salinidad = 30;

  // Mostrar resultados
  Serial.print("Valor digital: ");
  Serial.println(valorADC);
  Serial.print("Salinidad estimada: ");
  Serial.println(salinidad);
}

//----------------------------------- Medición de luminosidad -----------------------------------------
void obtenerLuminosidad() {
  int16_t valorADC = ads.readADC_SingleEnded(CANAL_LUMINOSIDAD); // Leer valor ADC
  float voltaje = valorADC * 0.125 / 1000.0;                     // Convertir a voltaje (GAIN_ONE)
  float lux = (voltaje / 3.3) * 1000.0;                          // Calcular luminosidad en lux

  // Determinar si es de noche o día
  if (lux < UMBRAL_NOCHE) {
    Serial.println("Es de noche.");
  } else {
    Serial.println("Es de día.");
  }

  // Mostrar luminosidad
  Serial.print("Luminosidad: ");
  Serial.print(lux);
  Serial.println(" lux");
}

//----------------------------------- Bucle principal -------------------------------------------------
void loop() {
  // Leer la opción seleccionada por el usuario
  if (Serial.available() > 0) {
    String entrada = Serial.readStringUntil('\n'); // Leer entrada del usuario
    seleccion = entrada.toInt();                  // Convertir a número entero
    Serial.print("Opción seleccionada: ");
    Serial.println(seleccion);
  }

  // Mostrar menú de opciones
  Serial.println("\n--- Menú de Sensores ---");
  Serial.println("1. Humedad");
  Serial.println("2. Salinidad");
  Serial.println("3. Temperatura");
  Serial.println("4. Luminosidad");
  Serial.print("Seleccione una opción: ");

  // Ejecutar la función correspondiente según la opción seleccionada
  switch (seleccion) {
    case 1:
      obtenerHumedad();         // Medir humedad
      break;
    case 2:
      obtenerSalinidad();       // Medir salinidad
      break;
    case 3:
      obtenerTemperatura();     // Medir temperatura
      break;
    case 4:
      obtenerLuminosidad();     // Medir luminosidad
      break;
    default:
      Serial.println("Opción no válida."); // Mensaje de error
      break;
  }
  delay(2000);                  // Pausa entre mediciones
}
