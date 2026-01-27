#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Wire.h>

/* ================= LCD ================= */
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

/* ================= TECLADO ================= */
const byte FILAS = 4;
const byte COLUMNAS = 4;

char keys[FILAS][COLUMNAS] = {
  {'O','X','S','X'},
  {'W','X','T','X'},
  {'w','X','t','X'},
  {'A','X','B','X'}
};

byte pinesFilas[FILAS] = {22,23,24,25};
byte pinesColumnas[COLUMNAS] = {26,27,28,29};

Keypad teclado = Keypad(makeKeymap(keys), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

/* ================= VARIABLES ================= */
byte nivelAgua = 2;     // 1-3
byte nivelTemp = 2;     // 1-3
byte estado = IDLE;

bool encendido = false;
bool puertaBloqueada = false;

/* ================= SETUP ================= */
void setup() {
  lcd.begin(16,2);
  lcd.print("Lavaplatos");

  Wire.begin();   // MASTER
  enviarDatos();
}

/* ================= LOOP ================= */
void loop() {
  char tecla = teclado.getKey();
  if (!tecla || tecla == 'X') return;

  switch (tecla) {
    case 'O':
      encendido = true;
      estado = IDLE;
      break;

    case 'S':
      if (encendido && puertaBloqueada) estado = LLENANDO;
      break;

    case 'W':
      if (nivelAgua < 3) nivelAgua++;
      break;

    case 'w':
      if (nivelAgua > 1) nivelAgua--;
      break;

    case 'T':
      if (nivelTemp < 3) nivelTemp++;
      break;

    case 't':
      if (nivelTemp > 1) nivelTemp--;
      break;

    case 'A':
      puertaBloqueada = false;
      break;

    case 'B':
      puertaBloqueada = true;
      break;
  }

  mostrarLCD();
  enviarDatos();

  if (estado == LLENANDO) {
    delay(3000);
    estado = LAVANDO; enviarDatos();
    delay(5000);
    estado = DRENANDO; enviarDatos();
    delay(4000);
    estado = SECANDO; enviarDatos();
    delay(4000);
    estado = IDLE; enviarDatos();
  }
}

/* ================= FUNCIONES ================= */

void mostrarLCD() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(encendido ? "ON " : "OFF ");
  lcd.print(puertaBloqueada ? "PUERTA OK" : "PUERTA AB");

  lcd.setCursor(0,1);
  lcd.print("Ag:");
  lcd.print(nivelAgua);
  lcd.print(" Tp:");
  lcd.print(nivelTemp);
}

void enviarDatos() {
  Wire.beginTransmission(8);   // SLAVE addr
  Wire.write(nivelAgua);
  Wire.write(nivelTemp);
  Wire.write(estado);
  Wire.endTransmission();
}
