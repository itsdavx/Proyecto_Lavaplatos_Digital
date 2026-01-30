#include <LedControl.h>

// ================= PINES =================

#define D0 49
#define D1 50

#define DIN 51
#define CLK 52
#define CS  53

// ================= OBJETOS =================
LedControl lc = LedControl(DIN, CLK, CS, 8);

// ================= SETUP =================
void setup() {
  inicializarLedControl();
  inicializarMotores();
}

// ================= LOOP =================
void loop() {
  aguaBajo(true);
  while (true); // detener ejecución
}

// ================= FUNCIONES =================

// ---------- LED CONTROL ----------
void inicializarLedControl() {
  for (int i = 0; i < 8; i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 15);
    lc.clearDisplay(i);
  }
}

void apagarTodo() {
  for (int m = 0; m < 8; m++) {
    lc.clearDisplay(m);
  }
}

// ---------- MOTORES CONTROL ------------
void inicializarMotores(){
  /*
  Binario para los niveles
    D1 D0
    0  0  - APAGADO
    0  1  - BAJO
    1  0  - MEDIO
    1  1  - ALTO
  */
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
}

void motorBajo(bool activo) {
  if(activo){ 
    digitalWrite(D1, 0);
    digitalWrite(D0, 1);
  } else {
    apagarMotores();
  }
}

void motorMedio(bool activo) {
  if(activo){
    digitalWrite(D1, 1);
    digitalWrite(D0, 0);
  } else {
    apagarMotores();
  }
}

void motorAlto(bool activo) {
  if(activo){
    digitalWrite(D1, 1);
    digitalWrite(D0, 1);
  } else {
    apagarMotores();
  }
}

void apagarMotores(){
  digitalWrite(D0,0);
  digitalWrite(D1,0);
}


// ---------- ANIMACIÓN AGUA ----------
void aguaBajo(bool activo) {
  while (activo) {
    motorBajo(activo);
    for (int fila = 0; fila < 8; fila++) {
      for (int col = 0; col < 8; col++) {

        // Izquierda → centro
        lc.setLed(7, fila, col, true);
        lc.setLed(6, fila, col, true);

        // Derecha → centro
        lc.setLed(0, fila, 7 - col, true);
        lc.setLed(1, fila, 7 - col, true);

        delay(80);

        // Apagar (efecto chorro)
        lc.setLed(7, fila, col, false);
        lc.setLed(6, fila, col, false);
        lc.setLed(0, fila, 7 - col, false);
        lc.setLed(1, fila, 7 - col, false);

        if (!activo) return;
      }
    }
  }
}
