#include <LedControl.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

// ================= PINES MATRIZ =================
#define CS  53
#define SCK 52
#define DIN 51

// ================= PINES MOTOR =================
#define D1 50
#define D0 49

// ================= PULSADORES =================
#define BTN_ON    48
#define BTN_START 47

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

// ================= KEYPAD TEMP =================
#define TC1 32
#define TF1 31
#define TF2 30

// ================= KEYPAD AGUA =================
#define NAC1 29
#define NAF1 28
#define NAF2 27

// ================= EXTRAS =================
#define BUZZER 26 

// ================= OBJETOS =================
LedControl lc(DIN, SCK, CS, 8);
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// ================= KEYPADS =================
char teclasTemp[2][1] = {
  {'o'}, // Tecla aumentar temperatura (TF1)
  {'k'}  // Tecla disminuir temperatura (TF2)
};

byte filasTemp[2] = {TF1, TF2};
byte colsTemp[1]  = {TC1};
Keypad tecladoTemp(makeKeymap(teclasTemp), filasTemp, colsTemp, 2, 1);

char teclasAgua[2][1] = {
  {'p'}, // Tecla aumentar agua (NAF1)
  {'l'}  // Tecla disminuir agua (NAF2)
};

byte filasAgua[2] = {NAF1, NAF2};
byte colsAgua[1]  = {NAC1};
Keypad tecladoAgua(makeKeymap(teclasAgua), filasAgua, colsAgua, 2, 1);

// ================= ESTADO =================
bool sistemaEncendido = false;
bool lavadoActivo = false;
bool pausa = false;
bool mostrarMarca = true;

byte nivelTemp = 0;  // 0=BAJO, 1=MEDIO, 2=ALTO
byte nivelAgua = 0;  // 0=BAJO, 1=MEDIO, 2=ALTO

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
  leerPulsadoresGenerales();

  if (sistemaEncendido) {
    // Permitir configuración cuando NO está activo O cuando está en PAUSA
    if (!lavadoActivo || pausa) {
      leerConfiguracionAvanzada();
    }
    actualizarLeds();
    ejecutarLavado();
  }
}

// ================= KEYPADS =================
void leerConfiguracionAvanzada() {
  
  // Leer teclado de temperatura
  char t = tecladoTemp.getKey();
  if (t) {
    if (t == 'o') { // Aumentar temperatura
      if (nivelTemp < 2) {
        nivelTemp++;
      } else {
        nivelTemp = 2; // Mantener en máximo
      }
      sonar(2);
      actualizarLeds();
      actualizarPantallaConfig();
      delay(200);
    }
    else if (t == 'k') { // Disminuir temperatura
      if (nivelTemp > 0) {
        nivelTemp--;
      } else {
        nivelTemp = 0; // Mantener en mínimo
      }
      sonar(2);
      actualizarLeds();
      actualizarPantallaConfig();
      delay(200);
    }
  }

  // Leer teclado de agua
  char a = tecladoAgua.getKey();
  if (a) {
    if (a == 'p') { // Aumentar agua
      if (nivelAgua < 2) {
        nivelAgua++;
      } else {
        nivelAgua = 2; // Mantener en máximo
      }
      sonar(2);
      actualizarLeds();
      actualizarPantallaConfig();
      delay(200);
    }
    else if (a == 'l') { // Disminuir agua
      if (nivelAgua > 0) {
        nivelAgua--;
      } else {
        nivelAgua = 0; // Mantener en mínimo
      }
      sonar(2);
      actualizarLeds();
      actualizarPantallaConfig();
      delay(200);
    }
  }
}

// ================= SONIDOS =================
void sonar(int tipo) {
  switch (tipo) {
    case 1: tone(BUZZER,1000,100); delay(150); tone(BUZZER,1500,300); break;
    case 2: tone(BUZZER,2000,50); break;
    case 3: tone(BUZZER,800,100); delay(100); tone(BUZZER,800,100); break;
    case 4: tone(BUZZER,500,300); delay(300); tone(BUZZER,300,400); break;
  }
}

// ================= BOTONES =================
void leerPulsadoresGenerales() {

  if (digitalRead(BTN_ON)) {
    delay(50);
    if (digitalRead(BTN_ON)) {
      if (!sistemaEncendido) {
        sistemaEncendido = true;
        sonar(1);
        lcd.display();
        lcd.clear();
        lcd.print("LAVAVAJILLAS");
        lcd.setCursor(5,1);
        lcd.print("ULTRA");
        delay(1500);
        mostrarMarca = false;
        lcd.clear();
        lcd.print("LISTO P/ INICIAR");
        actualizarPantallaConfig();
        actualizarLeds();
      } else if (!lavadoActivo || pausa) {
        sonar(4);
        sistemaEncendido = false;
        apagarTodo();
      }
      while(digitalRead(BTN_ON));
    }
  }

  if (digitalRead(BTN_START) && sistemaEncendido) {
    delay(50);
    if (digitalRead(BTN_START)) {
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
          limpiarMatriz();
          tiempoPausaInicio = millis();
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("PAUSA...        ");
          actualizarPantallaConfig();
        } else {
          tiempoInicioLavado += millis() - tiempoPausaInicio;
          lcd.clear();
        }
      }
      while(digitalRead(BTN_START));
    }
  }
}

// ================= LAVADO =================
void ejecutarLavado() {
  if (!lavadoActivo || pausa) return;

  unsigned long t = millis() - tiempoInicioLavado;

  if (t >= duracionLavado) {
    lavadoActivo = false;
    apagarMotores();
    limpiarMatriz();
    sonar(4);
    lcd.clear();
    lcd.print("LAVADO TERMINADO");
    delay(2000);
    apagarTodo();
    sistemaEncendido = true;
    lcd.display();
    lcd.print("LISTO P/ INICIAR");
    actualizarPantallaConfig();
    actualizarLeds();
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

// ================= LCD =================
void mostrarLCD(unsigned long t) {
  lcd.setCursor(0,0);
  const char* txt[] = {"PRELAVADO","LAVADO","ENJUAGUE","SECADO"};
  lcd.print(txt[etapa]);
  lcd.print("        ");

  lcd.setCursor(12,0);
  int seg = (duracionLavado - t) / 1000;
  if (seg < 10) lcd.print("0");
  lcd.print(seg);
  lcd.print("s");

  lcd.setCursor(0,1);
  int barras = map(t, 0, duracionLavado, 0, 16);
  for (int i=0;i<16;i++) lcd.print(i<barras?char(255):'_');
}

void actualizarPantallaConfig() {
  if (!mostrarMarca && (!lavadoActivo || pausa)) {
    lcd.setCursor(0,1);
    lcd.print("T:");
    if (nivelTemp == 0) lcd.print("BAJ ");
    else if (nivelTemp == 1) lcd.print("MED ");
    else lcd.print("ALT ");
    
    lcd.print("A:");
    if (nivelAgua == 0) lcd.print("BAJ ");
    else if (nivelAgua == 1) lcd.print("MED ");
    else lcd.print("ALT ");
    
    if (!lavadoActivo) {
      lcd.setCursor(0,0);
      lcd.print("LISTO P/ INICIAR");
    }
  }
}

// ================= LEDS =================
void actualizarLeds() {
  // TEMPERATURA - Apagar todos primero
  digitalWrite(T1, LOW);
  digitalWrite(T2, LOW);
  digitalWrite(T3, LOW);
  
  // Encender solo el LED correspondiente a la temperatura
  if (nivelTemp == 0) {
    digitalWrite(T1, HIGH); // Temperatura BAJA
  } else if (nivelTemp == 1) {
    digitalWrite(T2, HIGH); // Temperatura MEDIA
  } else if (nivelTemp == 2) {
    digitalWrite(T3, HIGH); // Temperatura ALTA
  }
  
  // AGUA - Apagar todos primero
  digitalWrite(N1, LOW);
  digitalWrite(N2, LOW);
  digitalWrite(N3, LOW);
  
  // Encender solo el LED correspondiente al agua
  if (nivelAgua == 0) {
    digitalWrite(N1, HIGH); // Agua BAJA
  } else if (nivelAgua == 1) {
    digitalWrite(N2, HIGH); // Agua MEDIA
  } else if (nivelAgua == 2) {
    digitalWrite(N3, HIGH); // Agua ALTA
  }
}

// ================= DELAY NO BLOQUEANTE =================
bool delayNoBloqueante(unsigned long ms) {
  unsigned long inicio = millis();
  while (millis() - inicio < ms) {
    leerPulsadoresGenerales();
    if (pausa || !sistemaEncendido) return false;
    delay(1); // pequeño delay para no saturar
  }
  return true;
}

// ================= ANIMACIONES =================
void limpiarMatriz() {
  for (int i=0; i<8; i++) {
    lc.clearDisplay(i);
  }
}

void aguaBajo(bool activo) {
  motorBajo(activo);
  for (int f=0; f<8; f++) {
    if(pausa||!sistemaEncendido) {
      apagarMotores();
      limpiarMatriz();
      return;
    }
    for (int c=0; c<8; c++) {
      if(pausa||!sistemaEncendido) {
        apagarMotores();
        limpiarMatriz();
        return;
      }
      
      lc.setLed(7,f,c,true); lc.setLed(6,f,c,true);
      lc.setLed(0,f,7-c,true); lc.setLed(1,f,7-c,true);
      
      if (!delayNoBloqueante(80)) {
        apagarMotores();
        limpiarMatriz();
        return;
      }
      
      lc.setLed(7,f,c,false); lc.setLed(6,f,c,false);
      lc.setLed(0,f,7-c,false); lc.setLed(1,f,7-c,false);
    }
  }
}

void aguaMedio(bool activo) {
  motorMedio(activo);
  for (int f=0; f<8; f++) {
    if(pausa||!sistemaEncendido) {
      apagarMotores();
      limpiarMatriz();
      return;
    }
    for (int c=0; c<8; c++) {
      if(pausa||!sistemaEncendido) {
        apagarMotores();
        limpiarMatriz();
        return;
      }
      
      lc.setLed(7,f,c,true); lc.setLed(6,f,c,true); lc.setLed(5,f,c,true);
      lc.setLed(0,f,7-c,true); lc.setLed(1,f,7-c,true); lc.setLed(2,f,7-c,true);
      
      if (!delayNoBloqueante(50)) {
        apagarMotores();
        limpiarMatriz();
        return;
      }
      
      lc.setLed(7,f,c,false); lc.setLed(6,f,c,false); lc.setLed(5,f,c,false);
      lc.setLed(0,f,7-c,false); lc.setLed(1,f,7-c,false); lc.setLed(2,f,7-c,false);
    }
  }
}

void aguaAlto(bool activo) {
  motorAlto(activo);
  for (int f=0; f<8; f++) {
    if(pausa||!sistemaEncendido) {
      apagarMotores();
      limpiarMatriz();
      return;
    }
    for (int c=0; c<8; c++) {
      if(pausa||!sistemaEncendido) {
        apagarMotores();
        limpiarMatriz();
        return;
      }
      
      for (int m=0;m<=3;m++) {
        lc.setLed(7-m,f,c,true);
        lc.setLed(m,f,7-c,true);
      }
      
      if (!delayNoBloqueante(30)) {
        apagarMotores();
        limpiarMatriz();
        return;
      }
      
      for (int m=0;m<=3;m++) {
        lc.setLed(7-m,f,c,false);
        lc.setLed(m,f,7-c,false);
      }
    }
  }
}

// ================= HARDWARE =================
void inicializarLedControl() {
  for (int i=0;i<8;i++) {
    lc.shutdown(i,false);
    lc.setIntensity(i,10);
    lc.clearDisplay(i);
  }
}

void inicializarLCD(){ 
  lcd.begin(16,2); 
}

void inicializarEntradas(){
  pinMode(BTN_ON,INPUT);
  pinMode(BTN_START,INPUT);
  pinMode(BUZZER,OUTPUT);
  
  // Configurar LEDs como salida
  pinMode(T1,OUTPUT); 
  pinMode(T2,OUTPUT); 
  pinMode(T3,OUTPUT);
  pinMode(N1,OUTPUT); 
  pinMode(N2,OUTPUT); 
  pinMode(N3,OUTPUT);
}

void inicializarMotores(){ 
  pinMode(D0,OUTPUT); 
  pinMode(D1,OUTPUT); 
}

// ================= MOTORES =================
void motorBajo(bool a){ 
  if(a){
    digitalWrite(D1,LOW);
    digitalWrite(D0,HIGH);
  } else {
    apagarMotores();
  }
}

void motorMedio(bool a){ 
  if(a){
    digitalWrite(D1,HIGH);
    digitalWrite(D0,LOW);
  } else {
    apagarMotores();
  }
}

void motorAlto(bool a){ 
  if(a){
    digitalWrite(D1,HIGH);
    digitalWrite(D0,HIGH);
  } else {
    apagarMotores();
  }
}

void apagarMotores(){ 
  digitalWrite(D0,LOW); 
  digitalWrite(D1,LOW); 
}

// ================= RESET =================
void apagarTodo() {
  lcd.clear();
  lcd.noDisplay();
  apagarMotores();
  limpiarMatriz();
  digitalWrite(T1,LOW); 
  digitalWrite(T2,LOW); 
  digitalWrite(T3,LOW);
  digitalWrite(N1,LOW); 
  digitalWrite(N2,LOW); 
  digitalWrite(N3,LOW);
  lavadoActivo=false;
  pausa=false;
  mostrarMarca=true;
  nivelTemp = 0;
  nivelAgua = 0;
}