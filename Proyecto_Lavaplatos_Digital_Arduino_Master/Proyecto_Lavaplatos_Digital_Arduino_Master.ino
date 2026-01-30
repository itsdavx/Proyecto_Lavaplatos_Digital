#include <LedControl.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

// ================= PINES MATRIZ =================
#define DIN 51
#define SCK 52
#define CS  53

// ================= PINES MOTOR =================
#define D0 49
#define D1 50

// ================= PULSADORES =================
#define BTN_ON     48
#define BTN_START  47
#define BTN_PAUSE  46

// ================= LCD =================
#define LCD_RS 40
#define LCD_EN 41
#define LCD_D4 42
#define LCD_D5 43
#define LCD_D6 44
#define LCD_D7 45

// ================= LEDS =================
#define T1 38
#define T2 37
#define T3 36
#define N1 35
#define N2 34
#define N3 33

// ================= TECLADO TEMPERATURA =================
#define TC1 32
#define TF1 31
#define TF2 30

// ================= TECLADO AGUA =================
#define NAC1 29
#define NAF1 28
#define NAF2 27

// ================= OBJETOS =================
LedControl lc = LedControl(DIN, SCK, CS, 8);
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// ================= KEYPADS =================
char teclasTemp[2][1] = {{'o'}, {'k'}};
byte filasTemp[2] = {TF1, TF2};
byte colsTemp[1]  = {TC1};
Keypad tecladoTemp = Keypad(makeKeymap(teclasTemp), filasTemp, colsTemp, 2, 1);

char teclasAgua[2][1] = {{'p'}, {'l'}};
byte filasAgua[2] = {NAF1, NAF2};
byte colsAgua[1]  = {NAC1};
Keypad tecladoAgua = Keypad(makeKeymap(teclasAgua), filasAgua, colsAgua, 2, 1);

// ================= ESTADOS =================
bool sistemaEncendido = false;
bool lavadoActivo = false;
bool pausa = false;

byte nivelTemp = 0;
byte nivelAgua = 0;

unsigned long tiempoInicioLavado = 0;
unsigned long duracionLavado = 60000;
byte etapa = 0;

// ================= SETUP =================
void setup() {
  inicializarLedControl();
  inicializarMotores();
  inicializarLCD();
  inicializarEntradas();
  apagarTodo();
}

// ================= LOOP =================
void loop() {
  leerPulsadores();

  if (sistemaEncendido) {
    leerTeclados();
    actualizarLeds();
    ejecutarLavado();
  }
}

// ================= INICIALIZACIONES =================
void inicializarLedControl() {
  for (int i = 0; i < 8; i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 10);
    lc.clearDisplay(i);
  }
}

void inicializarLCD() {
  lcd.begin(16, 2);
  delay(50);
  lcd.clear();
}

void inicializarEntradas() {
  pinMode(BTN_ON, INPUT);
  pinMode(BTN_START, INPUT);
  pinMode(BTN_PAUSE, INPUT);

  pinMode(T1, OUTPUT);
  pinMode(T2, OUTPUT);
  pinMode(T3, OUTPUT);
  pinMode(N1, OUTPUT);
  pinMode(N2, OUTPUT);
  pinMode(N3, OUTPUT);
}

// ================= APAGADO TOTAL =================
void apagarTodo() {
  lcd.clear();
  apagarMotores();

  digitalWrite(T1, LOW);
  digitalWrite(T2, LOW);
  digitalWrite(T3, LOW);
  digitalWrite(N1, LOW);
  digitalWrite(N2, LOW);
  digitalWrite(N3, LOW);
}

// ================= PULSADORES =================
void leerPulsadores() {

  if (digitalRead(BTN_ON)) {
    sistemaEncendido = !sistemaEncendido;

    if (!sistemaEncendido) {
      lavadoActivo = false;
      pausa = false;
      apagarTodo();
    } else {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Lavaplatos ON");
    }
    delay(300);
  }

  if (sistemaEncendido && digitalRead(BTN_START)) {
    lavadoActivo = true;
    pausa = false;
    etapa = 0;
    tiempoInicioLavado = millis();
    lcd.clear();
    delay(300);
  }

  if (sistemaEncendido && lavadoActivo && digitalRead(BTN_PAUSE)) {
    pausa = !pausa;
    apagarMotores();
    delay(300);
  }
}

// ================= TECLADOS =================
void leerTeclados() {
  char tTemp = tecladoTemp.getKey();
  char tAgua = tecladoAgua.getKey();

  if (tTemp == 'o' && nivelTemp < 2) nivelTemp++;
  if (tTemp == 'k' && nivelTemp > 0) nivelTemp--;

  if (tAgua == 'p' && nivelAgua < 2) nivelAgua++;
  if (tAgua == 'l' && nivelAgua > 0) nivelAgua--;
}

// ================= LEDS =================
void actualizarLeds() {
  digitalWrite(T1, nivelTemp == 0);
  digitalWrite(T2, nivelTemp == 1);
  digitalWrite(T3, nivelTemp == 2);

  digitalWrite(N1, nivelAgua == 0);
  digitalWrite(N2, nivelAgua == 1);
  digitalWrite(N3, nivelAgua == 2);
}

// ================= MOTORES =================
void inicializarMotores(){
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
}

void motorBajo(bool activo) {
  if(activo){ digitalWrite(D1,0); digitalWrite(D0,1); }
  else apagarMotores();
}

void motorMedio(bool activo) {
  if(activo){ digitalWrite(D1,1); digitalWrite(D0,0); }
  else apagarMotores();
}

void motorAlto(bool activo) {
  if(activo){ digitalWrite(D1,1); digitalWrite(D0,1); }
  else apagarMotores();
}

void apagarMotores(){
  digitalWrite(D0,0);
  digitalWrite(D1,0);
}

// ================= LAVADO =================
void ejecutarLavado() {
  if (!lavadoActivo || pausa) return;

  unsigned long t = millis() - tiempoInicioLavado;

  if (t >= duracionLavado) {
    lavadoActivo = false;
    apagarMotores();
    lcd.clear();
    lcd.print("Lavado listo");
    return;
  }

  if (t < duracionLavado * 0.25) etapa = 0;
  else if (t < duracionLavado * 0.5) etapa = 1;
  else if (t < duracionLavado * 0.75) etapa = 2;
  else etapa = 3;

  mostrarLCD(t);

  if (nivelAgua == 0) aguaBajo(true);
  if (nivelAgua == 1) aguaMedio(true);
  if (nivelAgua == 2) aguaAlto(true);
}

// ================= LCD PROGRESO =================
void mostrarLCD(unsigned long t) {
  lcd.setCursor(0,0);
  switch (etapa) {
    case 0: lcd.print("PRELAVADO     "); break;
    case 1: lcd.print("LAVADO        "); break;
    case 2: lcd.print("ENJUAGUE      "); break;
    case 3: lcd.print("SECADO        "); break;
  }

  lcd.setCursor(0,1);
  int barras = map(t, 0, duracionLavado, 0, 16);
  for (int i=0; i<16; i++) {
    lcd.print(i < barras ? char(255) : ' ');
  }
}
// ================= ANIMACIONES AGUA =================
void aguaBajo(bool activo) {
  while (activo) {
    motorBajo(activo);
    for (int fila = 0; fila < 8; fila++) {
      for (int col = 0; col < 8; col++) {
        lc.setLed(7, fila, col, true);
        lc.setLed(6, fila, col, true);
        lc.setLed(0, fila, 7 - col, true);
        lc.setLed(1, fila, 7 - col, true);
        delay(80);
        lc.setLed(7, fila, col, false);
        lc.setLed(6, fila, col, false);
        lc.setLed(0, fila, 7 - col, false);
        lc.setLed(1, fila, 7 - col, false);
        if (!activo) return;
      }
    }
  }
}

void aguaMedio(bool activo) {
  while (activo) {
    motorMedio(activo);
    for (int fila = 0; fila < 8; fila++) {
      for (int col = 0; col < 8; col++) {
        lc.setLed(7, fila, col, true);
        lc.setLed(6, fila, col, true);
        lc.setLed(5, fila, col, true);
        lc.setLed(0, fila, 7 - col, true);
        lc.setLed(1, fila, 7 - col, true);
        lc.setLed(2, fila, 7 - col, true);
        delay(50);
        lc.setLed(7, fila, col, false);
        lc.setLed(6, fila, col, false);
        lc.setLed(5, fila, col, false);
        lc.setLed(0, fila, 7 - col, false);
        lc.setLed(1, fila, 7 - col, false);
        lc.setLed(2, fila, 7 - col, false);
        if (!activo) return;
      }
    }
  }
}

void aguaAlto(bool activo) {
  while (activo) {
    motorAlto(activo);
    for (int fila = 0; fila < 8; fila++) {
      for (int col = 0; col < 8; col++) {
        for (int m = 0; m <= 3; m++) {
          lc.setLed(7 - m, fila, col, true);
          lc.setLed(m, fila, 7 - col, true);
        }
        delay(30);
        for (int m = 0; m <= 3; m++) {
          lc.setLed(7 - m, fila, col, false);
          lc.setLed(m, fila, 7 - col, false);
        }
        if (!activo) return;
      }
    }
  }
}