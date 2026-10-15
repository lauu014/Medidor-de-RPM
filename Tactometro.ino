#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Instancia del LCD I2C (Dirección 0x27, 16 columnas, 2 filas)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definición de pines
const byte pinSensor = 2;          // Sensor Hall en Pin 2 (Interrupción)
const int pinPot = A0;             // Potenciómetro de ajuste de límite
const int pinBuzzer = 10;          // Buzzer de alerta
const int pinLED = 11;             // LED indicador de exceso de velocidad

const int imanesPorVuelta = 1;     // 1 imán en el eje

volatile unsigned long contadorPulsos = 0;
unsigned long tiempoAnterior = 0;
float rpm = 0;
int limiteRPM = 0;                 // Límite seteado por el potenciómetro

// Rutina de Servicio de Interrupción (ISR)
void contadorISR() {
  contadorPulsos++;
}

void setup() {
  Serial.begin(9600);
  
  // Inicialización del LCD
  lcd.init();
  lcd.backlight();
  
  // Pantalla de presentación
  lcd.setCursor(0, 0);
  lcd.print(" TACOMETRO PRO  ");
  lcd.setCursor(0, 1);
  lcd.print(" SISTEMA ACTIVO ");
  delay(2000);
  lcd.clear();

  pinMode(pinSensor, INPUT_PULLUP);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinLED, OUTPUT);
  
  // Activamos interrupción por flanco descendente
  attachInterrupt(digitalPinToInterrupt(pinSensor), contadorISR, FALLING);
  
  tiempoAnterior = millis();
}

void loop() {
  // Leemos el potenciómetro analógico (0 a 1023) y lo mapeamos a un rango de RPM (0 a 5000 RPM)
  int valorPot = analogRead(pinPot);
  limiteRPM = map(valorPot, 0, 1023, 0, 5000);

  // Realizamos el cálculo de velocidad cada 1 segundo (1000 ms)
  if (millis() - tiempoAnterior >= 1000) {
    
    detachInterrupt(digitalPinToInterrupt(pinSensor));
    
    // Cálculo matemático de RPM
    rpm = (float)(contadorPulsos * 60.0) / imanesPorVuelta;
    
    // Depuración por Monitor Serie
    Serial.print("RPM: ");
    Serial.print(rpm);
    Serial.print(" | Límite: ");
    Serial.println(limiteRPM);
    
    // ACTUALIZACIÓN DE PANTALLA LCD
    // Fila 0: Muestra las RPM actuales
    lcd.setCursor(0, 0);
    lcd.print("RPM: ");
    lcd.print((int)rpm);
    lcd.print("     "); // Limpiar residuos
    
    // Fila 1: Muestra el límite ajustado con el pote
    lcd.setCursor(0, 1);
    lcd.print("MAX: ");
    lcd.print(limiteRPM);
    lcd.print("     ");
    
    // LÓGICA DE ALARMA POR SOBRERREVOLUCIÓN
    if (rpm > limiteRPM && limiteRPM > 0) {
      digitalWrite(pinLED, HIGH);
      digitalWrite(pinBuzzer, HIGH);
    } else {
      digitalWrite(pinLED, LOW);
      digitalWrite(pinBuzzer, LOW);
    }
    
    // Reseteamos contadores
    contadorPulsos = 0;
    tiempoAnterior = millis();
    
  	attachInterrupt(digitalPinToInterrupt(pinSensor), contadorISR, FALLING);
  }
}
