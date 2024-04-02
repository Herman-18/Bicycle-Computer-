//Configuracion Pantalla LCD
#include <LiquidCrystal.h>
const int rs=22, en=23,d4=24,d5=25,d6=26,d7=27;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);
#include "Wire.h"
#include "iarduino_RTC.h"
iarduino_RTC time(RTC_DS1302,44,42,43);
//Maquina de Estados para el menu 
typedef enum{ MenuPrincipal, MenuVelocidadK,MenuVelocidadM,MostarVelocidadInsKm,MostarVelocidadInsM,MostarVelocidadPromKm,MostarVelocidadPromM,MostarVelocidadMaxM,MostarVelocidadMaxKm,MostrarHora,MostrarDistanciaKm,MostrarDistanciaM,AdjustRadius,SelecciondeUnitV,SelecciondeUnitD } STATE_T;
STATE_T state;


//variables globales
static double velocidadInstantanea=0; //Velocidad Instantanea de la Bicicleta
static double velocidadPromedio=0; //Velocidad Promedio de la Bicicleta
static double velocidadMaxima=0; //Velocidad Maxima de la Bicicleta
static double distanciaTotal=0; //Distancia Total Recorrida de la Bicicleta
static double radioRueda=0.3;// Radio de la bicicleta
unsigned long presente = 0; // Identificador de Flanco
unsigned long pasado = 0; // Identificador de Flanco
static double in = 0; //Registra tiempo en el cual empieza el flanco
static double timeTotal=0; //Registra el Tiempo total del recorrido
static int valPul1=0;//Valor del pulsador 1
static int valPul2=0;//Valor del pulsador 2
static int valPul3=0;//Valor del pulsador 3
static int valPul4=0;//Valor del pulsador 4
void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  in=millis();
  presente=digitalRead(32);
  pasado=digitalRead(32);
  state=MenuPrincipal;
  time.begin();
  time.settime(00,36,13,31,3,24,7);
  

}
void loop() {
    //Serial.println(time.gettime("d-m-Y, H:i:s, D"));
    timeTotal=micros();
    bicycleComputer(velocidadInstantanea,velocidadPromedio,distanciaTotal,velocidadMaxima,radioRueda);
    menuLcd(velocidadInstantanea,velocidadPromedio,distanciaTotal,velocidadMaxima,radioRueda,valPul1,valPul2,valPul3,valPul4,state);
}

void bicycleComputer(double &velIns,double &velP,double &distanceT,double &velmax,double &r){
static double out = 0; //Registra tiempo en el cual acaba el flanco
static double rps = 0;
static int conTotal = 0;
static double pi=3.14159;
static double diff=0;
//Serial.println(presente);
//Serial.println(pasado);
  typedef enum{ Calculating } STATE_T;
  STATE_T state;
  state = Calculating;
    switch(state){
      case Calculating:
        if(pasado==0 && presente==1)
        {
          out=millis();
          diff=out-in;
          diff/=1000;
          rps=1/diff;
          velIns=((2*3.1416*r)*rps)*3.6;
          findVelMax(velIns,velmax);
          conTotal++;
          in=out;
          pasado=presente;
          presente=digitalRead(32);
          state=Calculating;
        }
        else if(!(pasado==0 && presente==1))
        {
          pasado=presente;
          presente=digitalRead(32);
          distanceT=(2*pi*r)*conTotal;
          velP=((distanceT/(timeTotal/1000000))*3.6);
          state=Calculating;
        }
        break;
    }
  
}
void menuLcd(double &velIns, double &velP, double &distanceT,double &velmax,double &r, int &pul1, int &pul2, int &pul3, int &pul4, STATE_T &state) {
    static unsigned long lastScrollTime = 0;  // Variable estática para almacenar el tiempo del último desplazamiento
    unsigned long scrollInterval = 350;
    static unsigned long lastButtonPressTime = 0;  // Variable para almacenar el tiempo del último botón presionado
    unsigned long debounceDelay = 250; // Intervalo de desplazamiento en milisegundos

    pul1 = digitalRead(34);
    pul2 = digitalRead(35);
    pul3 = digitalRead(36);
    pul4 = digitalRead(37);
    
  if (millis() - lastButtonPressTime > debounceDelay) {
    switch(state) {
        case MenuPrincipal:
            // Realizar el desplazamiento del texto cada vez que la función se ejecuta en MenuPrincipal
            if (millis() - lastScrollTime > scrollInterval) {
                lcd.scrollDisplayLeft();
                lastScrollTime = millis(); // Actualizar el tiempo del último desplazamiento
            }

            lcd.setCursor(0, 0);
            lcd.print("1.Velocity 2.Distance  ");
            lcd.setCursor(0, 1);
            lcd.print("3.Time 4.Wheel Radius    ");
            if(pul1 == 1) {
                lcd.clear();
                state =SelecciondeUnitV ;
                lastButtonPressTime = millis();
            }
            else if(pul2 == 1) {
                lcd.clear();
                state = SelecciondeUnitD;
                lastButtonPressTime = millis();
            }
            else if(pul3==1){
              lcd.clear();
              state=MostrarHora;
              lastButtonPressTime = millis();
            }
            else if(pul4==1){
              lcd.clear();
              state=AdjustRadius;
              lastButtonPressTime = millis();
            }
            break;
        case SelecciondeUnitV:
            lcd.setCursor(0,0);
            lcd.print("Select a Unit");
            lcd.setCursor(0,1);
            lcd.print("1. Km/h");
            lcd.setCursor(8, 1);
            lcd.print("2. MPH");
             if(pul1 == 1) {
                lcd.clear();
                state =MenuVelocidadK;
                lastButtonPressTime = millis();
            }
            else if(pul2==1){
                lcd.clear();
                state =MenuVelocidadM;
                lastButtonPressTime = millis();
            }
            else if(pul4==1){
              lcd.clear();
              state=MenuPrincipal;
              lastButtonPressTime = millis();
            }
        break;
        case MenuVelocidadK:
        if (millis() - lastScrollTime > scrollInterval) {
                lcd.scrollDisplayLeft();
                lastScrollTime = millis(); // Actualizar el tiempo del último desplazamiento
            }
            lcd.setCursor(0,0);
            lcd.print("Velocity: ");
            lcd.setCursor(0,1);
            lcd.print("1.Instant 2.Average 3.Maximum");
            if(pul1==1){
              lcd.clear();
              state=MostarVelocidadInsKm;
              lastButtonPressTime = millis();

            }else if(pul2==1){
              lcd.clear();
              state=MostarVelocidadPromKm;
              lastButtonPressTime = millis();
            }
            else if(pul3==1){
              lcd.clear();
              state=MostarVelocidadMaxKm;
              lastButtonPressTime = millis();
            }
            else if(pul4==1){
              lcd.clear();
              state=MenuPrincipal;
              lastButtonPressTime = millis();
            }
        break;
        case MenuVelocidadM:
        if (millis() - lastScrollTime > scrollInterval) {
                lcd.scrollDisplayLeft();
                lastScrollTime = millis(); // Actualizar el tiempo del último desplazamiento
            }
            lcd.setCursor(0,0);
            lcd.print("Velocity: ");
            lcd.setCursor(0,1);
            lcd.print("1.Instant 2.Average 3.Maximum");;
            if(pul1==1){
              lcd.clear();
              state=MostarVelocidadInsM;
              lastButtonPressTime = millis();

            }else if(pul2==1){
              lcd.clear();
              state=MostarVelocidadPromM;
              lastButtonPressTime = millis();
            }
            else if(pul3==1){
              lcd.clear();
              state=MostarVelocidadMaxM;
              lastButtonPressTime = millis();
            }
            else if(pul4==1){
              lcd.clear();
              state=MenuPrincipal;
              lastButtonPressTime = millis();
            }
        break;
        case MostarVelocidadInsKm:
            lcd.setCursor(0,0);
            lcd.print("Instant Speed");
            lcd.setCursor(0,1);
            lcd.print(velIns);
            lcd.setCursor(8, 1);
            lcd.print("km/h");
            if(pul4==1){
              lcd.clear();
              state=MenuVelocidadK;
              lastButtonPressTime = millis();
            }
            break;
        case MostarVelocidadPromKm:
            lcd.setCursor(0,0);
            lcd.print("Average Speed");
            lcd.setCursor(0,1);
            lcd.print(velP);
            lcd.setCursor(8, 1);
            lcd.print("km/h");
            if(pul4==1){
              lcd.clear();
              state=MenuVelocidadK;
              lastButtonPressTime = millis();
            }
        break;
        case MostarVelocidadMaxKm:
            lcd.setCursor(0,0);
            lcd.print("Maximum Speed");
            lcd.setCursor(0,1);
            lcd.print(velmax);
            lcd.setCursor(8, 1);
            lcd.print("km/h");
            if(pul4==1){
              lcd.clear();
              state=MenuVelocidadK;
              lastButtonPressTime = millis();
            }
        break;
        case MostarVelocidadInsM:
            lcd.setCursor(0,0);
            lcd.print("Instant Speed");
            lcd.setCursor(0,1);
            lcd.print(velIns*0.621);
            lcd.setCursor(8, 1);
            lcd.print("mph");
            if(pul4==1){
              lcd.clear();
              state=MenuVelocidadM;
              lastButtonPressTime = millis();
            }
            break;
        case MostarVelocidadPromM:
            lcd.setCursor(0,0);
            lcd.print("Average Speed");
            lcd.setCursor(0,1);
            lcd.print(velP*0.621);
            lcd.setCursor(8, 1);
            lcd.print("mph");
            if(pul4==1){
              lcd.clear();
              state=MenuVelocidadM;
              lastButtonPressTime = millis();
            }
        break;
        case MostarVelocidadMaxM:
            lcd.setCursor(0,0);
            lcd.print("Maximum Speed");
            lcd.setCursor(0,1);
            lcd.print(velmax*0.621);
            lcd.setCursor(8, 1);
            lcd.print("km/h");
            if(pul4==1){
              lcd.clear();
              state=MenuVelocidadM;
              lastButtonPressTime = millis();
            }
        break;
        case SelecciondeUnitD:
            lcd.setCursor(0,0);
            lcd.print("Select a Unit");
            lcd.setCursor(0,1);
            lcd.print("1. MTS");
            lcd.setCursor(8, 1);
            lcd.print("2. FTS");
             if(pul1 == 1) {
                lcd.clear();
                state =MostrarDistanciaKm;
                lastButtonPressTime = millis();
            }
            else if(pul2==1){
                lcd.clear();
                state =MostrarDistanciaM;
                lastButtonPressTime = millis();
            }
            else if(pul4==1){
              lcd.clear();
              state=MenuPrincipal;
              lastButtonPressTime = millis();
            }
        break;
        case MostrarDistanciaKm:
            lcd.setCursor(0,0);
            lcd.print("Distance");
            lcd.setCursor(0,1);
            lcd.print(distanceT);
            lcd.setCursor(12, 1);
            lcd.print("mts");
            if(pul4==1){
              lcd.clear();
              state=MenuPrincipal;
              lastButtonPressTime = millis();
            }
        break;
        case MostrarDistanciaM:
            lcd.setCursor(0,0);
            lcd.print("Distance");
            lcd.setCursor(0,1);
            lcd.print(distanceT*3.208);
            lcd.setCursor(12, 1);
            lcd.print("ft");
            if(pul4==1){
              lcd.clear();
              state=MenuPrincipal;
              lastButtonPressTime = millis();
            }
        break;
        case MostrarHora:
            lcd.setCursor(0,0);
            lcd.print(time.gettime("d M Y,D"));
            lcd.setCursor(0,1);
            lcd.print(time.gettime("H:i:s"));
            if(pul4==1){
              lcd.clear();
              state=MenuPrincipal;
              lastButtonPressTime = millis();
            }
        break;
        case AdjustRadius:
            lcd.setCursor(0,0);
            lcd.print("1.Up 2.Down");
            lcd.setCursor(0,1);
            lcd.print(r);
            lcd.setCursor(10, 1);
            lcd.print("mt");
            if(pul4==1){
              lcd.clear();
              state=MenuPrincipal;
              lastButtonPressTime = millis();
            }
            else if(pul1==1){
              lcd.clear();
              r=r+0.01;
              state=AdjustRadius;
              lastButtonPressTime = millis();
            }
            else if(pul2==1){
              lcd.clear();
              r=r-0.01;
              state=AdjustRadius;
              lastButtonPressTime = millis();
            }
        break;
    }
  }
}
void findVelMax(double &veli, double &velmax){
  if(velmax<veli){
    velmax=veli;
  }
}
