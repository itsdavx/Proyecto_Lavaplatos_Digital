#include <Wire.h>

/* ================= LEDS ================= */
// Agua (32)
const byte ledsAgua[32] = {
  22,23,24,25,26,27,28,29,
  30,31,32,33,34,35,36,37,
  38,39,40,41,42,43,44,45,
  46,47,48,49,50,51,52,53
};

// Temperatura (8)
const byte ledsTemp[8] = {2,3,4,5,6,7,8,9};

/* ================= MOTORES ================= */
#define MOTOR_LAVADO   10
#define MOTOR_DRENAJE  11
#define MOTOR_SECADO   12

/* ================= VARIABLES ================= */
byte nivelAgua = 0;
byte nivelTemp = 0;
byte estado = 0;

/* ================= SETUP ================= */
void setup() {
  Wire.begin(8); // SLAVE
  Wire.onReceive(recibirDatos);

  for (int i = 0; i < 32; i++) pinMode(ledsAgua[i], OUTPUT);
  for (int i = 0; i < 8; i++) pinMode(ledsTemp[i], OUTPUT);

  pinMode(MOTOR_LAVADO, OUTPUT);
  pinMode(MOTOR_DRENAJE, OUTPUT);
  pinMode(MOTOR_SECADO, OUTPUT);

  apagarTodo();
}

/* ================= LOOP ================= */
void loop() {
  mostrarAgua();
  mostrarTemp();
  controlarMotores();
}

/* ================= FUNCIONES ================= */

void recibirDatos(int bytes) {
  nivelAgua = Wire.read();
  nivelTemp = Wire.read();
  estado = Wire.read();
}

void mostrarAgua() {
  int objetivo = (nivelAgua == 1) ? 10 :
                 (nivelAgua == 2) ? 20 : 32;

  for (int i = 0; i < 32; i++) {
    digitalWrite(ledsAgua[i], i < objetivo ? HIGH : LOW);
  }
}

void mostrarTemp() {
  int objetivo = (nivelTemp == 1) ? 2 :
                 (nivelTemp == 2) ? 5 : 8;

  for (int i = 0; i < 8; i++) {
    digitalWrite(ledsTemp[i], i < objetivo ? HIGH : LOW);
  }
}

void controlarMotores() {
  apagarMotores();

  if (estado == 2) digitalWrite(MOTOR_LAVADO, HIGH);
  if (estado == 3) digitalWrite(MOTOR_DRENAJE, HIGH);
  if (estado == 4) digitalWrite(MOTOR_SECADO, HIGH);
}

void apagarMotores() {
  digitalWrite(MOTOR_LAVADO, LOW);
  digitalWrite(MOTOR_DRENAJE, LOW);
  digitalWrite(MOTOR_SECADO, LOW);
}

void apagarTodo() {
  for (int i = 0; i < 32; i++) digitalWrite(ledsAgua[i], LOW);
  for (int i = 0; i < 8; i++) digitalWrite(ledsTemp[i], LOW);
  apagarMotores();
}