#include <LiquidCrystal.h>

/* =========================
   DEFINICIÓN DE PINES
   ========================= */

// Entradas
#define START_BUTTON   22
#define DOOR_SWITCH    23
#define WATER_LEVEL    A0
#define TEMPERATURE    A1

// Salidas
#define VALVE_LED      30   // Válvula de entrada de agua
#define WASH_MOTOR     31   // Motor de lavado
#define DRAIN_MOTOR    32   // Bomba de drenaje
#define HEATER_LED     33   // Calentador
#define DRY_LED        34   // Secado

// LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

/* =========================
   PARÁMETROS DEL SISTEMA
   ========================= */

const int WATER_LEVEL_OK = 600;   // Nivel mínimo de agua
const int TEMP_OK        = 500;   // Temperatura objetivo

/* =========================
   SETUP
   ========================= */

void setup() {
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(DOOR_SWITCH, INPUT_PULLUP);

  pinMode(VALVE_LED, OUTPUT);
  pinMode(WASH_MOTOR, OUTPUT);
  pinMode(DRAIN_MOTOR, OUTPUT);
  pinMode(HEATER_LED, OUTPUT);
  pinMode(DRY_LED, OUTPUT);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Lavaplatos");
  lcd.setCursor(0, 1);
  lcd.print("Listo");
}

/* =========================
   FUNCIONES DEL CICLO
   ========================= */

void fillWater() {
  lcd.clear();
  lcd.print("Llenando agua");

  digitalWrite(VALVE_LED, HIGH);

  while (analogRead(WATER_LEVEL) < WATER_LEVEL_OK) {
    delay(200);
  }

  digitalWrite(VALVE_LED, LOW);
}

void heatWater() {
  lcd.clear();
  lcd.print("Calentando");

  digitalWrite(HEATER_LED, HIGH);

  while (analogRead(TEMPERATURE) < TEMP_OK) {
    delay(200);
  }

  digitalWrite(HEATER_LED, LOW);
}

void wash() {
  lcd.clear();
  lcd.print("Lavando...");

  digitalWrite(WASH_MOTOR, HIGH);
  delay(5000);
  digitalWrite(WASH_MOTOR, LOW);
}

void drain() {
  lcd.clear();
  lcd.print("Drenando");

  digitalWrite(DRAIN_MOTOR, HIGH);
  delay(4000);
  digitalWrite(DRAIN_MOTOR, LOW);
}

void rinse() {
  lcd.clear();
  lcd.print("Enjuagando");

  fillWater();
  digitalWrite(WASH_MOTOR, HIGH);
  delay(3000);
  digitalWrite(WASH_MOTOR, LOW);
  drain();
}

void dry() {
  lcd.clear();
  lcd.print("Secando");

  digitalWrite(DRY_LED, HIGH);
  delay(4000);
  digitalWrite(DRY_LED, LOW);
}

/* =========================
   LOOP PRINCIPAL
   ========================= */

void loop() {

  // Verificación de puerta
  if (digitalRead(DOOR_SWITCH) == HIGH) {
    lcd.setCursor(0, 1);
    lcd.print("Puerta abierta ");
    return;
  }

  lcd.setCursor(0, 1);
  lcd.print("Presione START");

  // Inicio del ciclo
  if (digitalRead(START_BUTTON) == LOW) {

    lcd.clear();
    lcd.print("Iniciando...");
    delay(1000);

    fillWater();
    heatWater();
    wash();
    drain();
    rinse();
    dry();

    lcd.clear();
    lcd.print("Ciclo finalizado");
    delay(5000);

    lcd.clear();
    lcd.print("Lavaplatos");
    lcd.setCursor(0, 1);
    lcd.print("Listo");
  }
}
