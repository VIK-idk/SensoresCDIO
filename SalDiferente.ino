#define power_pin 5 // Pin para alimentar el sensor de salinidad

// Definir los coeficientes del polinomio de Lagrange obtenido previamente
float a =  769.0 /1130976000.0 ; // Coeficiente para x^4
float b = -222379.0 / 125664000.0; // Coeficiente para x^3
float c = 194295151.0 / 113097600.0; // Coeficiente para x^2
float d = -231396623.0 / 314160.0; // Coeficiente para x
float e = 4747356355.0 /40392.0; // Término independiente

void setup() {
  pinMode(power_pin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int16_t adc0;

  // Alimentamos la sonda con un tren de pulsos
  digitalWrite(power_pin, HIGH);
  delay(100);
  
  // Leemos cuando hay un nivel alto
  adc0 = analogRead(A0);
  digitalWrite(power_pin, LOW);
  delay(100);

  // Evaluar el polinomio de Lagrange manualmente con el valor digital leído (adc0)
  float salinidad = a * pow(adc0, 4) + b * pow(adc0, 3) + c * pow(adc0, 2) + d * adc0 + e;

  Serial.print("Valor digital = ");
  Serial.println(adc0);
  Serial.print("Salinidad en gramos = ");
  Serial.println(salinidad );

  delay(1000); // Espera un segundo antes de la siguiente lectura
}
