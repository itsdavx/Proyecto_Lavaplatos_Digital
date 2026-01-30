#include <LedControl.h>
#include <LiquidCrystal.h>

// ================= PINES MATRIZ =================
#define DIN 51
#define SCK 52
#define CS  53

// ================= PINES MOTOR =================
#define D0 49
#define D1 50

// ================= PINES EXTRAS =================
#define BUZZER 9 

// ================= PULSADORES PRINCIPALES (CON RESISTENCIA) =================
#define BTN_ON    48
#define BTN_START 47

// ================= PULSADORES CONFIGURACION (MATRIZ) =================
// Definimos los pares de pines para el escaneo manual
#define PIN_TEMP_A 31 // TF1
#define PIN_TEMP_B 32 // TC1

#define PIN_AGUA_A 28 // NAF1
#define PIN_AGUA_B 29 // NAC1

// ================= LCD =================
#define LCD_RS 40
#define LCD_EN 41
#define LCD_D4 42
#define LCD_D5 43
#define LCD_D6 44
#define LCD_D7 45

// ================= LEDS INDICADORES =================
#define T1 38
#define T2 37
#define T3 36
#define N1 35
#define N2 34
#define N3 33

// ================= OBJETOS =================
LedControl lc = LedControl(DIN, SCK, CS, 8);
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// ================= VARIABLES ESTADO =================
bool sistemaEncendido = false;
bool lavadoActivo = false;
bool pausa = false;
bool mostrarMarca = true;

byte nivelTemp = 0;
byte nivelAgua = 0;

unsigned long tiempoInicioLavado = 0;
unsigned long tiempoPausaInicio = 0;
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
  leerPulsadoresGenerales(); // Revisa ON/OFF y START

  if (sistemaEncendido) {
    if (!lavadoActivo) {
       leerConfiguracionAvanzada(); // Lectura forzada de matriz
    }
    actualizarLeds();
    ejecutarLavado();
  }
}

// ================= ESCANEO ROBUSTO DE BOTONES MATRIZ =================
// Esta función fuerza la lectura eléctrica para evitar errores de simulación
bool leerBotonMatriz(int pinTierra, int pinLectura) {
  bool presionado = false;
  
  // Configuramos al instante para leer
  pinMode(pinTierra, OUTPUT);
  digitalWrite(pinTierra, LOW); // Simula GND
  pinMode(pinLectura, INPUT_PULLUP); // Activa resistencia interna
  
  delay(5); // Pequeña espera para estabilizar simulación
  
  if (digitalRead(pinLectura) == LOW) {
    presionado = true;
  }
  
  // Dejamos los pines en alta impedancia para no cruzar señales
  pinMode(pinTierra, INPUT);
  pinMode(pinLectura, INPUT);
  
  return presionado;
}

void leerConfiguracionAvanzada() {
  // --- TEMPERATURA ---
  // Probamos leer el botón conectado entre 31 y 32
  if (leerBotonMatriz(PIN_TEMP_A, PIN_TEMP_B)) { 
    nivelTemp++;
    if (nivelTemp > 2) nivelTemp = 0;
    sonar(2);
    actualizarLeds(); // Actualizar Inmediatamente
    actualizarPantallaConfig();
    delay(250); // Anti-rebote
  }

  // --- AGUA ---
  // Probamos leer el botón conectado entre 28 y 29
  if (leerBotonMatriz(PIN_AGUA_A, PIN_AGUA_B)) {
    nivelAgua++;
    if (nivelAgua > 2) nivelAgua = 0;
    sonar(2);
    actualizarLeds(); // Actualizar Inmediatamente
    actualizarPantallaConfig();
    delay(250); // Anti-rebote
  }
}

// ================= SONIDOS =================
void sonar(int tipo) {
  switch (tipo) {
    case 1: tone(BUZZER, 1000, 100); delay(150); tone(BUZZER, 1500, 300); break; // ON
    case 2: tone(BUZZER, 2000, 50); break; // CLICK
    case 3: tone(BUZZER, 800, 100); delay(100); tone(BUZZER, 800, 100); break; // START
    case 4: tone(BUZZER, 500, 300); delay(300); tone(BUZZER, 300, 400); break; // OFF
  }
}

// ================= LECTURA START / ON =================
void leerPulsadoresGenerales() {
  // --- ON / OFF ---
  if (digitalRead(BTN_ON)) {
    delay(50); 
    if(digitalRead(BTN_ON)) {
      if (!sistemaEncendido) {
        sistemaEncendido = true;
        sonar(1);
        lcd.display(); lcd.clear();
        lcd.setCursor(0, 0); lcd.print("LAVAVAJILLAS");
        lcd.setCursor(5, 1); lcd.print("ULTRA");
        delay(1500);
        mostrarMarca = false;
        lcd.clear(); lcd.print("LISTO P/ INICIAR");
        actualizarPantallaConfig(); // Mostrar config inicial
      } else {
        if (!lavadoActivo || pausa) { 
          sonar(4);
          sistemaEncendido = false;
          apagarTodo();
        }
      }
      while(digitalRead(BTN_ON)); 
    }
  }

  // --- START / PAUSE ---
  if (sistemaEncendido && digitalRead(BTN_START)) {
    delay(50); 
    if(digitalRead(BTN_START)) {
      sonar(3);
      if (!lavadoActivo) {
        lavadoActivo = true;
        pausa = false;
        tiempoInicioLavado = millis();
        lcd.clear();
      } else {
        pausa = !pausa;
        if (pausa) {
          apagarMotores();
          tiempoPausaInicio = millis();
          lcd.setCursor(0, 0); lcd.print("PAUSA...        ");
        } else {
          tiempoInicioLavado += (millis() - tiempoPausaInicio); 
        }
      }
      while(digitalRead(BTN_START)); 
    }
  }
}

void actualizarPantallaConfig() {
    if(!mostrarMarca && !lavadoActivo) {
        lcd.setCursor(0,0); lcd.print("T:"); 
        lcd.print(nivelTemp == 0 ? "BAJ " : (nivelTemp == 1 ? "MED " : "ALT "));
        lcd.print(" A:"); 
        lcd.print(nivelAgua == 0 ? "BAJ " : (nivelAgua == 1 ? "MED " : "ALT "));
        lcd.setCursor(0,1); lcd.print("MODE: CONFIG    ");
    }
}

// ================= ANIMACIONES CON REVISION RAPIDA =================
void chequearPausaEnBucle() {
  leerPulsadoresGenerales(); 
}

void aguaBajo(bool activo) {
    motorBajo(activo);
    for (int fila = 0; fila < 8; fila++) {
      chequearPausaEnBucle(); if(pausa || !sistemaEncendido) return; 
      for (int col = 0; col < 8; col++) {
        lc.setLed(7, fila, col, true); lc.setLed(6, fila, col, true);
        lc.setLed(0, fila, 7 - col, true); lc.setLed(1, fila, 7 - col, true);
        delay(80);
        lc.setLed(7, fila, col, false); lc.setLed(6, fila, col, false);
        lc.setLed(0, fila, 7 - col, false); lc.setLed(1, fila, 7 - col, false);
        chequearPausaEnBucle(); if(pausa || !sistemaEncendido) return;
      }
    }
}

void aguaMedio(bool activo) {
    motorMedio(activo);
    for (int fila = 0; fila < 8; fila++) {
      chequearPausaEnBucle(); if(pausa || !sistemaEncendido) return;
      for (int col = 0; col < 8; col++) {
        lc.setLed(7, fila, col, true); lc.setLed(6, fila, col, true); lc.setLed(5, fila, col, true);
        lc.setLed(0, fila, 7 - col, true); lc.setLed(1, fila, 7 - col, true); lc.setLed(2, fila, 7 - col, true);
        delay(50);
        lc.setLed(7, fila, col, false); lc.setLed(6, fila, col, false); lc.setLed(5, fila, col, false);
        lc.setLed(0, fila, 7 - col, false); lc.setLed(1, fila, 7 - col, false); lc.setLed(2, fila, 7 - col, false);
        chequearPausaEnBucle(); if(pausa || !sistemaEncendido) return;
      }
    }
}

void aguaAlto(bool activo) {
    motorAlto(activo);
    for (int fila = 0; fila < 8; fila++) {
      chequearPausaEnBucle(); if(pausa || !sistemaEncendido) return;
      for (int col = 0; col < 8; col++) {
        for (int m = 0; m <= 3; m++) {
          lc.setLed(7 - m, fila, col, true); lc.setLed(m, fila, 7 - col, true);
        }
        delay(30);
        for (int m = 0; m <= 3; m++) {
          lc.setLed(7 - m, fila, col, false); lc.setLed(m, fila, 7 - col, false);
        }
        chequearPausaEnBucle(); if(pausa || !sistemaEncendido) return;
      }
    }
}

// ================= RESTO DEL SISTEMA =================
void ejecutarLavado() {
  if (!lavadoActivo || pausa) return;

  unsigned long t = millis() - tiempoInicioLavado;

  if (t >= duracionLavado) {
    lavadoActivo = false;
    apagarMotores();
    sonar(4);
    lcd.clear(); lcd.print("LAVADO TERMINADO");
    delay(2000);
    apagarTodo();
    sistemaEncendido = true; 
    lcd.display(); lcd.print("LISTO P/ INICIAR");
    actualizarPantallaConfig();
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

void mostrarLCD(unsigned long t) {
  lcd.setCursor(0,0);
  switch (etapa) {
    case 0: lcd.print("PRELAVADO      "); break;
    case 1: lcd.print("LAVADO         "); break;
    case 2: lcd.print("ENJUAGUE       "); break;
    case 3: lcd.print("SECADO         "); break;
  }
  lcd.setCursor(12, 0);
  int seg = (duracionLavado - t) / 1000;
  if(seg<10) lcd.print("0"); lcd.print(seg); lcd.print("s");

  lcd.setCursor(0,1);
  int barras = map(t, 0, duracionLavado, 0, 16);
  for (int i=0; i<16; i++) lcd.print(i < barras ? char(255) : '_');
}

void actualizarLeds() {
  if (sistemaEncendido) {
      digitalWrite(T1, nivelTemp == 0); digitalWrite(T2, nivelTemp == 1); digitalWrite(T3, nivelTemp == 2);
      digitalWrite(N1, nivelAgua == 0); digitalWrite(N2, nivelAgua == 1); digitalWrite(N3, nivelAgua == 2);
  }
}

void inicializarLedControl() {
  for (int i = 0; i < 8; i++) {
    lc.shutdown(i, false); lc.setIntensity(i, 10); lc.clearDisplay(i);
  }
}
void inicializarLCD() { lcd.begin(16, 2); lcd.clear(); }
void inicializarEntradas() {
  pinMode(BTN_ON, INPUT); pinMode(BTN_START, INPUT); pinMode(BUZZER, OUTPUT);
  pinMode(T1, OUTPUT); pinMode(T2, OUTPUT); pinMode(T3, OUTPUT);
  pinMode(N1, OUTPUT); pinMode(N2, OUTPUT); pinMode(N3, OUTPUT);
}
void inicializarMotores(){ pinMode(D0, OUTPUT); pinMode(D1, OUTPUT); }
void motorBajo(bool activo) { if(activo){ digitalWrite(D1,0); digitalWrite(D0,1); } else apagarMotores(); }
void motorMedio(bool activo) { if(activo){ digitalWrite(D1,1); digitalWrite(D0,0); } else apagarMotores(); }
void motorAlto(bool activo) { if(activo){ digitalWrite(D1,1); digitalWrite(D0,1); } else apagarMotores(); }
void apagarMotores(){ digitalWrite(D0,0); digitalWrite(D1,0); }

void apagarTodo() {
  lcd.clear(); lcd.noDisplay(); apagarMotores();
  digitalWrite(T1, LOW); digitalWrite(T2, LOW); digitalWrite(T3, LOW);
  digitalWrite(N1, LOW); digitalWrite(N2, LOW); digitalWrite(N3, LOW);
  lavadoActivo = false; pausa = false; mostrarMarca = true;
}