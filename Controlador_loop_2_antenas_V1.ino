//CODIGO PARA EL CONTROL DE 3 MAGNETIC LOOP ANTENNAS
//ESCRITO POR JOSÉ BASCÓN EA7HVO
//LICENCIA PARA USO NO COMERCIAL

#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#define UP 17      //pin 2 para boton up
#define DOWN  11   //pin 5 para boton down
#define memUP  15  // pin 12 para boton memUP
#define memDOWN  16  //pin 13 para memDOWN
#define menuPin  14   // para funciones de menu
#define desabilitar  8 //pin 8 que es el enable de los pololu en la shield
#define encoderPinA  10   // cable amarillo del encoder en el pin 10
#define encoderPinB  9    //cable verde del encoder en el pin 9
#define lowSwitch 4   // pin 12 para final de carrera inferior
#define upSwitch 7   //pin 13 para fin de carrera superior

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

unsigned int lastEncoded = 0; //ultimo valor que teniamos del encoder para ver si hemos aumentado o disminuido la cuenta
unsigned int encoderValue = 0; //valor que extraemos del encoder
unsigned int lastencoderValue = 0;
unsigned int Eepromread = 64500; //se pone a cero para que se lea la memoria al comenzar
unsigned int i = 0; //para los bucles
unsigned long Time = 0; // temporizador
unsigned long Time2 = 0;    //Temporizador para antenas
unsigned long Time3 = 0;    //Temporizador para adjust
unsigned int a = 0; //marcador de  memUP y memDOWN para que se tome como paso del encoder
unsigned int b = 0; //marcador de destino de encodervalue tras recuperar una memoria
int direccion = 5;  //toma el valor de el pin DIR del pololu diferente en cada antena
int pololu = 2;    //toma el valor de el pin STEP del pololu diferente en cada antena
int memstep = 0; //marcador para la seleccion de memoria de pasos
int savmem = 0;   //marcador para activar la gravacion en memoria
int funcionmenu = 0; //marcador para seleccionar el menu entre ajuste, memorias, memorizacion...
int ajuste = 0;   //para el ajuste fino de los pasos en caso de que haga falta
int antena = 1;   //para la seleccion de antena entre 1 y 4 
int mempointer = 32; //puntero para la seleccion de grupo de memorias
int upperlimit = 60000; //limite superior para que el condensador nunca pase de aqui
int nodata = 0;   // variable para cuando no existen datos en la memoria
int backlash = 0;   //Variable para backlash correction
int sp = 2;   //variable para la velocidad
int enPolo = 0;   //Variable para habilitar o no el pololu en reposo
int microst = 0;   //variable para los micropasos
int mcr = 0;    //marcador auxiliar de microst
int switchlimit = 0; //variable para cundo se tocan los limites fisicos
int dir = 0;
unsigned int khz = 0;
unsigned int lastkhz = 0;


void setup() {

digitalWrite(2, LOW);
digitalWrite(3, LOW);
digitalWrite(4, LOW);
digitalWrite(5, LOW);
digitalWrite(6, LOW);
digitalWrite(7, LOW);
digitalWrite(8, HIGH);

 
 pinMode(2, OUTPUT);  // pin 2 como salida para controlar pololu 1
 pinMode(5, OUTPUT);   // pin 5 como salida para controlar pololu 1
 pinMode(3, OUTPUT);  // pin 3 como salida para controlar pololu 2
 pinMode(6, OUTPUT);   // pin 6 como salida para controlar pololu 2
 pinMode(4, OUTPUT);  // pin 4 como salida para controlar pololu 3
 pinMode(7, OUTPUT);   // pin 7 como salida para controlar pololu 3


 pinMode(desabilitar,OUTPUT); //pin 8 como salida para controlar pololu habilitar o desabilitar stepper
 pinMode(encoderPinA, INPUT_PULLUP); // cable verde del encoder como input puuup
 pinMode(encoderPinB, INPUT_PULLUP); // cable amarillo del encoder como input puuup
 pinMode(menuPin, INPUT_PULLUP);  // pulsador en pin 11 como input puuup
 pinMode(UP, INPUT_PULLUP); // pulsador en pin 2 como input puuup
 pinMode(DOWN, INPUT_PULLUP); // pulsador en pin 5 como input puuup
 pinMode(memUP, INPUT_PULLUP); // pulsador en pin 12 como input puuup
 pinMode(memDOWN, INPUT_PULLUP); // pulsador en pin 11  como input puuup
 pinMode(upSwitch, INPUT_PULLUP);  // pin 12 como lectura lower limit switch
 pinMode(lowSwitch, INPUT_PULLUP);   // pin 13 como lectura upper limit switch
 
 Serial.begin (9600); //solo por si es necesari visualizar algun proceso
 lcd.init();                      // initialize the lcd 
 lcd.backlight();                 // Encendemos el backlight
 
 int lastMSB = digitalRead(encoderPinA); //get starting position en el codificador rotatorio pin 3
 int lastLSB = digitalRead(encoderPinB); //get starting position en el codificador rotatorio pin 6

 //let start be lastEncoded so will index on first click
 lastEncoded = (lastMSB << 1) |lastLSB;

}
void etiquetador()   //FUNCION QUE COMPONE LA FRECUENCIA PARA MOSTRAR EN LA MEMORIA
    {         
 lcd.setCursor(0, 0);      // ubica cursor en columna 13 y linea 1  
 lcd.print("SAVE ");   // escribe el valor de khz  
 lcd.setCursor(0, 1);      // ubica cursor en columna 13 y linea 1  
 lcd.print("FREQ         KHZ");   // escribe el valor de khz  
 lcd.setCursor(6, 1);      // ubica cursor en columna 12 y linea 0
 lcd.print(khz);  
 Time = millis();   
 Serial.println(khz); 
 delay(300);
 while (digitalRead (menuPin) == HIGH ){

 int MSB = digitalRead(encoderPinA); //MSB = most significant bit
 int LSB = digitalRead(encoderPinB); //LSB = least significant bit

 int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
 int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

 if(sum == 0b1101) khz ++;
 if(sum == 0b1110) khz --;
 if(khz>64000) khz=0;
 if(khz>60000) khz=60000;

 lastEncoded = encoded; //store this value for next time
if (millis() - Time>= 5000)break; 
if(digitalRead(DOWN)==LOW){
  if(khz>1000)khz=khz-1000; 
  Time = millis();    
  delay(200);
}
if(digitalRead(UP)==LOW){
  if(khz<58000)khz=khz+1000; 
  Time = millis();    
  delay(200);
}
if(digitalRead(memUP)==LOW){
  if(khz<58000)khz=khz+100; 
  Time = millis();    
  delay(200);
}
if(digitalRead(memDOWN)==LOW){
  if(khz>1000)khz=khz-100; 
  Time = millis();    
  delay(200);
}
 if(khz != lastkhz){  
   Serial.println(khz);     
   lastkhz=khz;
   lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
   if(khz<10000 && khz>=1000) lcd.print("0");
   if(khz<1000 && khz>=100) lcd.print("00");
   else if(khz<100 && khz>=10) lcd.print("000");
   else if(khz<10) lcd.print("0000");
   lcd.print(khz);   // escribe el valor de khz
   Time = millis();   
 }  
  
}
 lcd.setCursor(6, 1);      // ubica cursor en columna 13 y linea 1  
 lcd.print("          ");   // borra el valor de khz  
}
void ajustapasos(){ if (dir == 1){ while ((encoderValue % microst) != 0){                              
                            encoderValue++;                           
                            digitalWrite(desabilitar, LOW);
                            delay(2);
                            digitalWrite(direccion, HIGH);        // giro en un sentido POSITIVO
                            digitalWrite(pololu, HIGH);       // nivel alto  
                            delay(12);                            
                            digitalWrite(pololu, LOW);        // nivel bajo
                            Serial.println(encoderValue);    
                            Serial.println("hi");                            
                            } 
                          }
                  if (dir == 0){ while ((encoderValue % microst) != 0){                              
                            encoderValue--;                           
                            digitalWrite(desabilitar, LOW);
                            delay(2);
                            digitalWrite(direccion, LOW);        // giro en un sentido NEGATIVO
                            digitalWrite(pololu, HIGH);       // nivel alto  
                            delay(12);                            
                            digitalWrite(pololu, LOW);        // nivel bajo
                            Serial.println(encoderValue);                              
                            } 
                          }
  
                   }
void autozero(){
               lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
               lcd.print(" LOOKING FOR 0  "); 
               while (digitalRead (lowSwitch) == HIGH){      
               digitalWrite(desabilitar, LOW);
               delay(2);
               digitalWrite(direccion, LOW);        // giro en un sentido negativo
               digitalWrite(pololu, HIGH);       // nivel alto  
               delay(sp);                            
               digitalWrite(pololu, LOW);        // nivel bajo 
               if(digitalRead(menuPin)==LOW )break;
               if(digitalRead(upSwitch)==LOW) {lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                                               lcd.print("                ");  
                                               limitesup ();
                                               delay (1000);
                                              }
                           } 
               lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
               lcd.print("                ");  
               limiteinf();
               pantalla();
  }

void pantalla(){
  lcd.clear();
  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
  lcd.print("MEM");   
  lcd.print(memstep);        //
  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
  lcd.print("ANT");   
  lcd.print(antena);        // 
  funcionmenu = 0;
  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
  lcd.print("MEM             "); 
  lcd.setCursor(6, 1);      // ubica cursor en columna 12 y linea 0
  lcd.print(khz); 
  lcd.print(" Khz"); 
  
   
  EEPROM.put(4,antena);     //grabamos el numero de memoria en que estamos
  EEPROM.put(mempointer,encoderValue);  
  EEPROM.put(mempointer+2,memstep);                                   
  EEPROM.put(mempointer+30,upperlimit);                  
  EEPROM.put(mempointer+32,sp);                  
  EEPROM.put(mempointer+34,backlash);
  EEPROM.put(mempointer+36,enPolo);                  
  EEPROM.put(mempointer+38,microst);
  EEPROM.put(mempointer+40,khz);
                  }

void limiteinf (){  
  lcd.setCursor(5, 1);      // ubica cursor en columna 6 y linea 1
  lcd.print("DOWN LIMIT");
  delay(2000);
  lcd.setCursor(5, 1);      // ubica cursor en columna 6 y linea 1
  lcd.print("ADJUSTING 0");
  while (digitalRead (lowSwitch) == LOW){      
                           digitalWrite(desabilitar, LOW);
                           delay(2);
                           digitalWrite(direccion, HIGH);        // giro en un sentido positivo
                           digitalWrite(pololu, HIGH);       // nivel alto  
                           delay(30);                            
                           digitalWrite(pololu, LOW);        // nivel bajo 
                           if(digitalRead(upSwitch)==LOW) {
                                                           limitesup ();
                                                           delay (1000);
                                                          }
                           }
                              
  encoderValue = 0; 
  digitalWrite(desabilitar, LOW); 
  delay(1000);
  lcd.setCursor(5, 1);      // ubica cursor en columna 6 y linea 1
  lcd.print("           ");
  
}

void limitesup(){  //funcion de bucle de salida de final de carrera superior
  lcd.setCursor(5, 1);      // ubica cursor en columna 6 y linea 1
  lcd.print("UPPER LIMIT");
  delay(2000);
  lcd.setCursor(5, 1);      // ubica cursor en columna 6 y linea 1
  lcd.print("ADJUST     ");
  while (digitalRead (upSwitch) == LOW){
                           digitalWrite(desabilitar, LOW);
                           delay(2);
                           digitalWrite(direccion, LOW);        // giro en un sentido negativo
                           digitalWrite(pololu, HIGH);       // nivel alto  
                           delay(30);                            
                           digitalWrite(pololu, LOW);        // nivel bajo
                           if(digitalRead(lowSwitch)==LOW) {
                                                           limiteinf ();
                                                           delay (1000);
                                                          } 
                             
    } 
  digitalWrite(desabilitar, LOW);    
  delay(1000);
  encoderValue--;
  lcd.setCursor(5, 1);      // ubica cursor en columna 6 y linea 1
  lcd.print("           ");
}   

void antmemupdown () {  //funcion dedicada a llamar y a guardar las memorias de antena y canal 
  i=0;
  while(i<=200)
  {
  if(digitalRead(memUP)==LOW || digitalRead(memDOWN)==LOW || savmem==1){  //Seleccionamos botones memoria arriba y abajo
    if(digitalRead(memUP)==LOW && funcionmenu != 1){
                                memstep++;
                                delay (500);
                                if(memstep>=15) memstep = 1;
                                Serial.println (memstep );
                                i = 0;
                                Time3 = millis();
                                }
    if(digitalRead(memDOWN)==LOW && funcionmenu != 1){
                                memstep--;
                                delay (500);
                                if(memstep<=0) memstep = 14;
                                Serial.println (memstep );
                                i = 0;
                                Time3 = millis();
                                }    
    if(digitalRead(memUP)==LOW && funcionmenu == 1){
                                if ( digitalRead (upSwitch) == HIGH && digitalRead (lowSwitch) == HIGH ) antena++;
                                delay (500);
                                if(antena>=3) antena = 1;
                                Serial.println (antena );
                                i = 0;
                                Time3 = millis();
                                }
    if(digitalRead(memDOWN)==LOW && funcionmenu == 1){
                                if ( digitalRead (upSwitch) == HIGH && digitalRead (lowSwitch) == HIGH ) antena--;
                                delay (500);
                                if(antena<=0) antena = 2;
                                Serial.println (antena );
                                i = 0;
                                Time3 = millis();
                                }   
                 switch(antena){
                                
                  case 1:         //                                     
                  EEPROM.put(mempointer,encoderValue);  
                  EEPROM.put(mempointer+2,memstep);  
                  EEPROM.put(mempointer+30,upperlimit);
                  EEPROM.put(mempointer+40,khz);
                  mempointer = 32;   
                  direccion =6;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 3;    //toma el valor de el pin STEP del pololu diferente en cada antena 
                  EEPROM.put(4,antena);     //grabamos el numero de memoria en que estamos
                  EEPROM.get(mempointer,encoderValue);  
                  EEPROM.get(mempointer+2,memstep); 
                  if (memstep == 0) memstep = 1;                  
                  EEPROM.get(mempointer+30,upperlimit);
                  if (upperlimit == 0) upperlimit = 64000; 
                  EEPROM.get(mempointer+32,sp);
                  if (sp <= 2) sp = 2;
                  EEPROM.get(mempointer+34,backlash);
                  EEPROM.get(mempointer+36,enPolo);
                  if(enPolo == 1) digitalWrite(desabilitar, LOW);
                  EEPROM.get(mempointer+38,microst);
                  if (microst > 60 || microst < 0) microst = 0;                  
                  b = encoderValue;                  
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep); 
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        // 
                  Time2 = millis();                                 
                  break;
                  
                  case 2:         //                                     
                  EEPROM.put(mempointer,encoderValue);  
                  EEPROM.put(mempointer+2,memstep);  
                  EEPROM.put(mempointer+30,upperlimit);
                  EEPROM.put(mempointer+40,khz);
                  mempointer = 128;   
                  direccion =5;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 2;    //toma el valor de el pin STEP del pololu diferente en cada antena
                  EEPROM.put(4,antena);     //grabamos el numero de memoria en que estamos 
                  EEPROM.get(mempointer,encoderValue);  
                  EEPROM.get(mempointer+2,memstep);
                  if (memstep == 0) memstep = 1;                   
                  EEPROM.get(mempointer+30,upperlimit); 
                  if (upperlimit == 0) upperlimit = 64000; 
                  EEPROM.get(mempointer+32,sp);
                  if (sp <= 2) sp = 2;
                  EEPROM.get(mempointer+34,backlash);
                  EEPROM.get(mempointer+36,enPolo);
                  if(enPolo == 1) digitalWrite(desabilitar, LOW);
                  EEPROM.get(mempointer+38,microst); 
                  if (microst > 60 || microst < 0) microst = 0;
                  EEPROM.get(mempointer+40,khz);                
                  b = encoderValue;                  
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep); 
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        // 
                  Time2 = millis();                                
                  break;
                  
                                
                                 
                 }                                                                                                           
                 switch(memstep){
                                
                  case 1:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+4,b);
                  EEPROM.get(mempointer+44,khz);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+44,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+44,khz); //guardamos los khz 
                  EEPROM.put(mempointer+4,encoderValue);                  
                  savmem = 0;
                  EEPROM.get(mempointer+4,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE1      "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                                    
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        //
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 2:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+6,b);
                  EEPROM.get(mempointer+46,khz);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+46,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+46,khz); //guardamos los khz 
                  EEPROM.put(mempointer+6,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+6,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE2      ");
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //  
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        // 
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 3:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+8,b);
                  EEPROM.get(mempointer+48,khz);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+48,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+48,khz); //guardamos los khz 
                  EEPROM.put(mempointer+8,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+8,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE3      "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);  
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        //
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;;
                  
                  case 4:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+50,khz);                  
                  EEPROM.get(mempointer+10,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+50,khz); 
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+50,khz); //guardamos los khz 
                  EEPROM.put(mempointer+10,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+10,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE4      "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        //
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;

                  case 5:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+52,khz); 
                  EEPROM.get(mempointer+12,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+52,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+52,khz); //guardamos los khz 
                  EEPROM.put(mempointer+12,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+12,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE5      "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        // 
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        //  
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 6:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+54,khz); 
                  EEPROM.get(mempointer+14,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+54,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+54,khz); //guardamos los khz 
                  EEPROM.put(mempointer+14,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+14,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE6      "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        //
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;                  
                  
                  case 7:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+56,khz); 
                  EEPROM.get(mempointer+16,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+56,khz); 
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+56,khz); //guardamos los khz 
                  EEPROM.put(mempointer+16,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+16,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE7      "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //   
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        //
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 8:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+58,khz);
                  EEPROM.get(mempointer+18,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+58,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+58,khz); //guardamos los khz
                  EEPROM.put(mempointer+18,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+18,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE8      "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //  
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        // 
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;

                  case 9:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+60,khz);
                  EEPROM.get(mempointer+20,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+60,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+60,khz); //guardamos los khz 
                  EEPROM.put(mempointer+20,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+20,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE9      "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        //   
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 10:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+62,khz);
                  EEPROM.get(mempointer+22,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+62,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+62,khz); //guardamos los khz 
                  EEPROM.put(mempointer+22,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+22,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE10     "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        // 
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        //  
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break; 

                  case 11:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+64,khz);
                  EEPROM.get(mempointer+24,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+64,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+64,khz); //guardamos los khz 
                  EEPROM.put(mempointer+24,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+24,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE11     ");
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //  
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        // 
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 12:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+66,khz);
                  EEPROM.get(mempointer+26,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+66,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+66,khz); //guardamos los khz 
                  EEPROM.put(mempointer+26,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+26,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE12     "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000); 
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //  
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        //
                  lcd.print("     ");    
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;

                  case 13:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+68,khz);
                  EEPROM.get(mempointer+28,b);
                  } 
                  if (savmem == 1) {
                  EEPROM.get(mempointer+68,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+68,khz); //guardamos los khz 
                  EEPROM.put(mempointer+28,encoderValue);
                  savmem = 0;
                  EEPROM.get(mempointer+28,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("SAVE13     "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //  
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");        // 
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;
                  
                  case 14:         //                  
                  if (funcionmenu == 0){
                  EEPROM.get(mempointer+70,khz);
                  EEPROM.get(mempointer+30,b);
                  if(b == 64000) { //si encontramos que b = 64000 quiere decir que venimos de un borrado de memoria
                  b=encoderValue;
                  nodata=1;
                  }
                  } 
                  if (savmem == 1) {  //si se pulsa boton up and down a la vez grabamos encodervalue en la posicion de memoria
                  EEPROM.get(mempointer+70,khz);
                  etiquetador(); //llamamos a la funcion que etiqueta los khz
                  EEPROM.put(mempointer+70,khz); //guardamos los khz 
                  EEPROM.put(mempointer+30,encoderValue);
                  upperlimit = encoderValue;
                  savmem = 0;
                  EEPROM.get(mempointer+30,b);
                  lcd.setCursor(5, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("UPLIM      "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print(b);                                  
                  delay(3000);
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
                  }                   
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("UPLIM");   
                  //lcd.print(memstep);        //
                  lcd.print("  ");
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        // 
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(khz);   
                  lcd.print(" Khz");  
                  lcd.print("     ");   
                  if (b == 0) {
                    b = encoderValue; // si no hay ningun valor en memoria se igualan para que no se muevan pasos
                    nodata=1;
                  }                  
                  break;  
                 }
              }
    i++; 
    delay (10);                             
    }
    EEPROM.put(mempointer+2,memstep);     //grabamos el numero de memoria en que estamos
    
    if (encoderValue < b) 
                        {  //Comparamos encodervalue con el paso de destino que en este caso es superior en numero
    lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 0
    lcd.print(b);   // escribe el valor de encodervalue 
    lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 0
    lcd.print("        UP");   // escribe el valor de encodervalue
    
    
    
    
    i= b+backlash;
    if(i>upperlimit) backlash = upperlimit - b;
    i= b+backlash;      
    while ( encoderValue < i ) //bucle de pasos hasta que encodervalue y b se igualan
    {    
    encoderValue++;   
    digitalWrite(desabilitar, LOW);
    delay(2);   
    digitalWrite(direccion, HIGH);        // giro en un sentido positivo
    digitalWrite(pololu, HIGH);       // nivel alto  
    delay(sp);                            
    digitalWrite(pololu, LOW);        // nivel bajo     
    if(digitalRead(DOWN)==LOW) break ;   
    if(digitalRead(upSwitch)==LOW)  {
                                      limitesup ();                                      
                                      goto saltoup;
                                      }
    }       
    
    
    delay(500);
    while ( encoderValue > i-backlash ) //bucle de pasos hasta que encodervalue y b se igualan
    {                         
                                
    encoderValue--;  
    digitalWrite(desabilitar, LOW); 
    delay(2);
    digitalWrite(direccion, LOW);        // giro en un sentido negativo  
    digitalWrite(pololu, HIGH);       // nivel alto  
    delay(sp);                            
    digitalWrite(pololu, LOW);        // nivel bajo  
    if(digitalRead(lowSwitch)==LOW) limiteinf ();    
    if(digitalRead(DOWN)==LOW) break ;          
    }
saltoup:   
    delay(500);
    lcd.setCursor(9,1);      // ubica cursor en columna 6 y linea 0
    lcd.print("       ");   // escribe el valor de encodervalue      
                       
                       }
  
  if (encoderValue > b)
                      {  //Comparamos encodervalue con el paso de destino que en este caso es inferior en numero
    lcd.setCursor(7, 1);      // ubica cursor en columna 6 y linea 0
    lcd.print(b);   // escribe el valor de encodervalue 
    lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 0
    lcd.print("      DOWN");   // escribe el valor de encodervalue     
    //if (backlash>b)  backlash = b-1; 
    //i= b-backlash;      
    while ( encoderValue > b ) //bucle de pasos hasta que encodervalue y b se igualan
    { 

    encoderValue-- ; 
    digitalWrite(desabilitar, LOW);  
    delay(2);
    digitalWrite(direccion, LOW);        // giro en un sentido negativo
    digitalWrite(pololu, HIGH);       // nivel alto  
    delay(sp);                            
    digitalWrite(pololu, LOW);        // nivel bajo 
    if(digitalRead(lowSwitch)==LOW) { limiteinf ();
                                      encoderValue = 0;
                                      goto saltodown;}
    if(digitalRead(DOWN)==LOW) break ;                 
    }
saltodown:    
    delay(500);
    lcd.setCursor(7, 1);      // ubica cursor en columna 6 y linea 0
    lcd.print("         ");   // escribe el valor de encodervalue     
                              
                       }
    if (nodata==1 && funcionmenu==0) {
      lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 0
      lcd.print("NO DATA  ");   // escribe el valor de encodervalue
      delay (1000);
      lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 0
      lcd.print("0 Khz     ");   // escribe el valor de encodervalue
      lcd.setCursor(7, 1);      // ubica cursor en columna 6 y linea 0      
      nodata=0;
      }
   lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 0
   lcd.print(khz);   // escribe el valor de encodervalue  
   lcd.print(" Khz");   // escribe el valor de encodervalue  
   EEPROM.put(mempointer+40,khz);
                      
  } //final del bucle memUP-memDOWN

void loop(){ 
  if (Eepromread == 64500)//Si es el encendido, el valor será siempre 64500
                      //entonces leemos todos los valores almacenados en memoria
                      //Inicializamos todos los valores 
  {  

  EEPROM.get(4,antena);  //lee la posicion 4 de la memoria el valor de la antena antre 1 y 4
  if(antena > 4 || antena < 1 ){      //si la lectura de antena no es un numero entre 1 y 4 se entiende que es la primera vez que se utiliza el controlador  
                                      //es necesario poner a 0 todos los valores menos el de la antena que queda a 1
                  lcd.clear();                    
                  lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0
                  lcd.print(" PREPARING FOR"); 
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1  
                  lcd.print(" THE FIRST USE"); 
                  delay(2000);                                     
        for (i=0; i <= 1000; i++) {
          EEPROM.put(i,0);     //escribimos el valor o en todas las posiciones
          delay(1);
          }  
        lcd.clear();
        EEPROM.put(4,1);     //escribimos el valor de antena a 1 en la memoria 
        antena = 1;           //ponemos antena a 1 
        encoderValue=0;
        khz=0;
        pantalla();                                 
    
    }
                 switch(antena){
                                
                  case 1:         //
                  mempointer = 32;    
                  EEPROM.get(mempointer,Eepromread);  
                  EEPROM.get(mempointer+2,memstep); 
                  if (memstep == 0) memstep = 1;                  
                  EEPROM.get(mempointer+30,upperlimit);
                  if (upperlimit == 0) upperlimit = 64000;
                  EEPROM.get(mempointer+32,sp);
                  if (sp <= 2) sp = 2;
                  EEPROM.get(mempointer+34,backlash);
                  EEPROM.get(mempointer+36,enPolo);
                  if(enPolo == 1) digitalWrite(desabilitar, LOW);
                  EEPROM.get(mempointer+38,microst); 
                  if (microst > 60 || microst < 0) microst = 0;
                  EEPROM.get(mempointer+40,khz);                  
                  direccion =6;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 3;    //toma el valor de el pin STEP del pololu diferente en cada antena                           
                  pantalla();                                 
                  break;
                  
                  case 2:         //
                  mempointer = 128;    
                  EEPROM.get(mempointer,Eepromread);  
                  EEPROM.get(mempointer+2,memstep);
                  if (memstep == 0) memstep = 1;                  
                  EEPROM.get(mempointer+30,upperlimit);
                  if (upperlimit == 0) upperlimit = 64000;
                  EEPROM.get(mempointer+32,sp);
                   if (sp <= 2) sp = 2;
                  EEPROM.get(mempointer+34,backlash);
                  EEPROM.get(mempointer+36,enPolo);
                  if(enPolo == 1) digitalWrite(desabilitar, LOW);
                  EEPROM.get(mempointer+38,microst); 
                  if (microst > 60 || microst < 0) microst = 0;
                  EEPROM.get(mempointer+40,khz);                 
                  direccion =5;  //toma el valor de el pin DIR del pololu diferente en cada antena
                  pololu = 2;    //toma el valor de el pin STEP del pololu diferente en cada antena                    
                  pantalla();                
                  break;                 

                  }  
                
  if(ajuste == 0 && enPolo == 0) digitalWrite(desabilitar, HIGH); //desabilita el stepper para que no consuma
  encoderValue = Eepromread ;  //se iguala la lectura de memoria con la primera posicion de codificador
  Eepromread = 0; //se pone a 1 para que no repita este proceso de lectura
  lcd.clear();
  lcd.setCursor(0, 0);      // ubica cursor en columna 8 y linea 1
  lcd.print(" EA7HVO CONTROL ");
  lcd.setCursor(0, 1);      // ubica cursor en columna 8 y linea 1
  delay(1000);
  lcd.print(" 2 LOOPS  VER 1"); 
  delay(2000);
  lcd.clear();
  pantalla();
  //lcd.setCursor(0, 1);      // ubica cursor en columna 8 y linea 1
  //lcd.print("MEM    ");    
  if(encoderValue > 64000) encoderValue=0 ; 
                                                    
    }
  
 int MSB = digitalRead(encoderPinA); //MSB = most significant bit
 int LSB = digitalRead(encoderPinB); //LSB = least significant bit

 int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
 int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
 if(digitalRead(memUP)==LOW || digitalRead(memDOWN)==LOW || savmem==1) antmemupdown ();
 

 if(digitalRead(UP)==LOW) //Si pulsamos el boton de UP
                            { if(ajuste == 1){          //si estamos en modo ajuste no cuenta pasos ( no suma a encodervalue)
                              
                              
                              digitalWrite(desabilitar, LOW);
                              delay(2);
                              digitalWrite(direccion, HIGH);        // giro en un sentido positivo
                              digitalWrite(pololu, HIGH);       // nivel alto  
                              delay(sp*5); 
                              Time3 = millis();                           
                              digitalWrite(pololu, LOW);        // nivel bajo 
                              if(digitalRead(upSwitch)==LOW) {lcd.setCursor(0,1);
                                                              lcd.print("                ");
                                                              ajuste=0;
                                                              limitesup ();
                                                              pantalla();
                                                              }                     
                              }
                              
                              else if(funcionmenu == 2) {Time3 = millis();  
                                                         savmem=1;                                                         
                                                         antmemupdown ();                                                         
                                                         }

                              else if(funcionmenu == 4) {
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("    ");                              
                                backlash++;
                                if (backlash > 200) backlash = 200;
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print(backlash); 
                                delay(100);
                                Time2 = millis();}

                              else if(funcionmenu == 5) {
                                lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("    ");                              
                                sp++;
                                if (sp > 40) sp = 40;
                                lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print(sp); 
                                delay(100);
                                Time2 = millis();}

                              else if(funcionmenu == 6) {
                                lcd.setCursor(10, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("  ");   
                                enPolo++;
                                if (enPolo >1 ) enPolo = 0;
                                lcd.setCursor(11, 1);      // ubica cursor en columna 6 y linea 1
                                if(enPolo == 0) lcd.print("Y");  // si desabilitamos el pololu en reposo
                                if(enPolo == 1) lcd.print("N");  // si desabilitamos el pololu en reposo
                                delay(200);
                                Time2 = millis();}

                              else if(funcionmenu == 7) autozero();

                              else if(funcionmenu == 8) {
                                lcd.setCursor(10, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("   ");   
                                mcr++;
                                if (mcr >= 5) mcr = 5;
                                lcd.setCursor(10, 1);      // ubica cursor en columna 10 y linea 1
                                if(mcr == 0) microst= 0; //convertimos mcr en microst
                                if(mcr == 1) microst= 2; //convertimos mcr en microst
                                if(mcr == 2) microst= 4; //convertimos mcr en microst
                                if(mcr == 3) microst= 8; //convertimos mcr en microst
                                if(mcr == 4) microst= 16; //convertimos mcr en microst
                                if(mcr == 5) microst= 32; //convertimos mcr en microst
                                lcd.print(microst);
                                delay(200);
                                Time2 = millis();}
                              
                              else {
                                a = 1;
                                b = encoderValue;                                
                                }                                                   
                              }
 if(digitalRead(DOWN)==LOW) //si Pulsamos el boton de down
                            { if(ajuste == 1){      //si estamos en modo ajuste no cuenta pasos ( no suma a encodervalue)                              
                              
                              digitalWrite(desabilitar, LOW);
                              delay(2);
                              digitalWrite(direccion, LOW);        // giro en un sentido negativo
                              digitalWrite(pololu, HIGH);       // nivel alto  
                              delay(sp*5);      
                              Time3 = millis();                          
                              digitalWrite(pololu, LOW);        // nivel bajo
                              if(digitalRead(lowSwitch)==LOW) {lcd.setCursor(0,1);
                                                               lcd.print("                ");
                                                               ajuste=0;
                                                               limiteinf ();
                                                               pantalla();
                                                               } 
                                                              }
                               
                               else if(funcionmenu == 2) {Time3 = millis();  
                                                          savmem=1;
                                                          antmemupdown ();
                                                          }

                               else if(funcionmenu == 4) {
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("    ");   
                                backlash--;
                                if (backlash <= 0) backlash = 0;
                                lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print(backlash); 
                                delay(100);
                                Time2 = millis();}

                               else if(funcionmenu == 5) {
                                lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("    ");   
                                sp--;
                                if (sp <= 2) sp = 2;
                                lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print(sp); 
                                delay(100);
                                Time2 = millis();}

                               else if(funcionmenu == 6) {
                                lcd.setCursor(10, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("  ");   
                                enPolo--;
                                if (enPolo < 0) enPolo = 1;
                                lcd.setCursor(11, 1);      // ubica cursor en columna 6 y linea 1
                                if(enPolo == 0) lcd.print("Y");  // si desabilitamos el pololu en reposo
                                if(enPolo == 1) lcd.print("N");  // si desabilitamos el pololu en reposo
                                delay(200);
                                Time2 = millis();}

                               else if(funcionmenu == 7) autozero();

                                else if(funcionmenu == 8) {
                                lcd.setCursor(10, 1);      // ubica cursor en columna 6 y linea 1
                                lcd.print("   ");   
                                mcr--;
                                if (mcr <= 0) mcr = 0;
                                lcd.setCursor(10, 1);      // ubica cursor en columna 10 y linea 1
                                if(mcr == 0) microst= 0; //convertimos mcr en microst
                                if(mcr == 1) microst= 2; //convertimos mcr en microst
                                if(mcr == 2) microst= 4; //convertimos mcr en microst
                                if(mcr == 3) microst= 8; //convertimos mcr en microst
                                if(mcr == 4) microst= 16; //convertimos mcr en microst
                                if(mcr == 5) microst= 32; //convertimos mcr en microst
                                lcd.print(microst);
                                delay(200);
                                Time2 = millis();}
                              
                              else{
                              a = 2;    
                              b = encoderValue;                              
                              }                                         
                              }

 if(ajuste == 0 && funcionmenu == 0 &&(sum == 0b1101 || a == 1)) 
                            {                               
                              if( b == encoderValue)  //el valor b es el que se pone como destino y aqui se alcanza
                             {                                                        
                                a = 0;
                                b = 0;                              
                               }
                             
                                                            
                              encoderValue ++; 
                              
                              if(encoderValue > upperlimit) {        //tope por arriba 
                                                        encoderValue=upperlimit;
                                                        lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                                                        lcd.print("UPLIM"); 
                                                        delay(2000);
                                                        lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                                                        lcd.print("MEM");   
                                                        lcd.print(memstep);        //
                                                        lcd.print("  ");
                                                        a = 0;
                                                        b = 0;
                                                        }
                              else {                      
                              digitalWrite(desabilitar, LOW);
                              delay(2);
                              digitalWrite(direccion, HIGH);        // giro en un sentido positivo
                              digitalWrite(pololu, HIGH);       // nivel alto  
                              delay(sp);                            
                              digitalWrite(pololu, LOW);        // nivel bajo 
                              dir = 1;  //dir se pone a 1 porque crecemos para calcular microst
                              if(digitalRead(upSwitch)==LOW) limitesup ();                                                                                          
                                    }                         
                              Time = millis();                                                                                                          
                              }
 if(ajuste == 0 && funcionmenu == 0 &&(sum == 0b1110 || a == 2)) 
                            { 
                               
                                                  
                              if( b == encoderValue) //el valor b es el que se pone como destino y aqui se alcanza      
                                {
                                a = 0;
                                b = 0;                                
                                }                              
                              encoderValue --;    
                              
                              if(encoderValue > 64000) {                  // por abajo
                                                     encoderValue=0; 
                                                     lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                                                     lcd.print("DOWN LIMIT"); 
                                                     delay(1000);
                                                     lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                                                     lcd.print("          ");
                                                     a = 0;
                                                     b = 0;
                                                    }
                              else {
                                
                              digitalWrite(desabilitar, LOW);
                              delay(2);
                              digitalWrite(direccion, LOW);        // giro en un sentido negativo
                              digitalWrite(pololu, HIGH);       // nivel alto  
                              delay(sp);                              
                              digitalWrite(pololu, LOW);        // nivel bajo 
                              dir = 0; // dir se pone a 0 porque decrecemos 
                              if(digitalRead(lowSwitch)==LOW) limiteinf ();                           
                                }                
                              Time = millis();                                                                      
                              }

 lastEncoded = encoded; //store this value for next time
 
  
 if(encoderValue != lastencoderValue) //Condiciones que se cumplen cuando se ha contado algun paso
 {
 lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0
 if(encoderValue<10000 && encoderValue>=1000) lcd.print("0");
 if(encoderValue<1000 && encoderValue>=100) lcd.print("00");
 if(encoderValue<100 && encoderValue>=10) lcd.print("000");
 if(encoderValue<10) lcd.print("0000");
 lcd.print(encoderValue);   // escribe el valor de encodervalue
 

 lastencoderValue=encoderValue; //se iguala el valor obtenido como el ultimo leido
 } 

   if(digitalRead(DOWN)==LOW && digitalRead(menuPin)==LOW){      //Restaura a los valores de fabrica el sistema 
                                      //es necesario poner a 0 todos los valores menos el de la antena que queda a 1
                  lcd.clear();                    
                  lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0
                  lcd.print(" PREPARING FOR"); 
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1  
                  lcd.print(" THE FIRST USE"); 
                  delay(2000);                                     
        for (i=0; i <= 1000; i++) {
          EEPROM.put(i,0);     //escribimos el valor o en todas las posiciones
          delay(1);
          }  
        lcd.clear();
        EEPROM.put(4,1);     //escribimos el valor de antena a 1 en la memoria 
        antena = 1;           //ponemos antena a 1 
        encoderValue=0;
        khz=0;
        pantalla();                               
    
    }
  if(digitalRead(UP)==LOW && digitalRead(DOWN)==LOW)   //reset encoderValue si pulsamos pin 17 y 11
              {
                  lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0
                  lcd.print("DELETTING MEMORY"); 
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1  
                  lcd.print("      BANK      "); 
                  lcd.setCursor(11, 1);      // ubica cursor en columna 0 y linea 1 
                  lcd.print(antena);   
                  delay(3000); 
                  upperlimit=64000;                                  
        for (i=mempointer; i <= mempointer+60; i++) {
                  EEPROM.put(i,0);     //escribimos el valor o en todas las posiciones
                  }  
                  lcd.clear();
                  encoderValue=1;
                  lcd.setCursor(6, 0);      // ubica cursor en columna 6 y linea 0
                  lcd.print("MEM");   
                  lcd.print(memstep);        //
                  lcd.setCursor(12, 0);      // ubica cursor en columna 12 y linea 0
                  lcd.print("ANT");   
                  lcd.print(antena);        //  
                  funcionmenu = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 12 y linea 0
                  lcd.print("MEM             ");
               }

 if(digitalRead(menuPin)==LOW )   //Cambiamos el modo a MHZ de ajuste a amateur y a broadcast
              { 
                funcionmenu++;
                if(funcionmenu>=9) funcionmenu = 0; 
                switch(funcionmenu){
                                
                  case 0:         //ajuste para que UP y DOWN no cuenten pasos y nos permitan ajustar fino                          
                  ajuste = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("MEM    ");   // escribe el valor de encodervalue
                  lcd.print("          ");  
                  EEPROM.put(mempointer+38,microst);                                                                 
                  break;                  
                  
                  case 1:                                  
                  ajuste = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("ANT            ");   // escribe el valor de encodervalue
                   
                  Time2 = millis();                
                  break;

                  case 2:
                  ajuste = 0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("SAVE         U/D");   // escribe el valor de encodervalue
                  Time3 = millis();                                   
                  break;
                  
                  case 3:
                  ajuste=1;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("ADJUST");   // escribe el valor de encodervalue
                  lcd.print("       U/D");
                  digitalWrite(desabilitar, LOW); //Habilita el stepper para que no se pierdan pasos
                  Time3 = millis();                   
                  break;

                  case 4:
                  ajuste=0;
                  EEPROM.get(mempointer+34,backlash); 
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("BACKLASH");   // escribe el valor de encodervalue
                  lcd.print("     U/D"); 
                  lcd.setCursor(9, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(backlash);
                  Time2 = millis();                                               
                  break;

                  case 5:
                  ajuste=0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("SPEED   ");   // escribe el valor de encodervalue
                  lcd.print("     U/D"); 
                  lcd.setCursor(6, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(sp);  
                  EEPROM.put(mempointer+34,backlash); 
                  Time2 = millis();                                                                                  
                  break;

                  case 6:
                  ajuste=0;
                  EEPROM.get(mempointer+32,sp);
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("DIS POLOLU   U/D");                    
                  lcd.setCursor(11, 1);      // ubica cursor en columna 6 y linea 1
                  if(enPolo == 0) lcd.print("Y");  // si desabilitamos el pololu en reposo
                  if(enPolo == 1) lcd.print("N");  // si desabilitamos el pololu en reposo
                  EEPROM.put(mempointer+32,sp);                                  
                  Time2 = millis();                                                                              
                  break;

                  case 7:
                  ajuste=0;
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("AUTO ZERO    U/D");                    
                  lcd.setCursor(11, 1);      // ubica cursor en columna 6 y linea 1                  
                  EEPROM.put(mempointer+36,enPolo);
                  Time2 = millis();                                                                              
                  break;

                  case 8:
                  ajuste=0;
                  EEPROM.get(mempointer+38,microst);
                  lcd.setCursor(0, 1);      // ubica cursor en columna 0 y linea 1
                  lcd.print("MICROSTEP    U/D");                    
                  lcd.setCursor(10, 1);      // ubica cursor en columna 6 y linea 1
                  lcd.print(microst);                  
                  Time2 = millis();                                                                              
                  break;
                  
                } 
                                           
               delay(500);
                              
               } 
          

               
                      
 if(millis() - Time>= 500) //temporizador de cada 0.5 segundos
  {
  Time = millis();  
  if(microst !=0)ajustapasos();    //lamamos a la funcion ajustapasos por si no estamos en un paso magnetico completo
  EEPROM.get(mempointer,Eepromread);// leemos los ultimos datos que teniamos correspondientes a encoderValue
  if(ajuste == 0 && enPolo == 0) digitalWrite(desabilitar, HIGH);
  if(enPolo == 1) digitalWrite(desabilitar, LOW);  //si está establecido que no se desabilitan los popolu
  if(backlash < 0 || backlash > 200) backlash = 0;
  if(int(Eepromread)!= encoderValue) //escribimos en la memoria solo si se han modificado los datos
  {
    Eepromread=encoderValue;//Igualamos eepromread a encodervalue
    EEPROM.put(mempointer,Eepromread);//lo guardamos
    EEPROM.put(mempointer+2,memstep);//tambien guardamos la memoria
    EEPROM.put(4,antena);//tambien guardamos la antena
    lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0                   
    if(ajuste == 0 && enPolo == 0) digitalWrite(desabilitar, HIGH);//desabilita el stepper para que no consuma
    }
     
  lcd.setCursor(0, 0);      // ubica cursor en columna 0 y linea 0
  if(encoderValue<10000 && encoderValue>=1000) lcd.print("0");
  if(encoderValue<1000 && encoderValue>=100) lcd.print("00");
  if(encoderValue<100 && encoderValue>=10) lcd.print("000");
  if(encoderValue<10) lcd.print("0000");
  lcd.print(encoderValue);   // escribe el valor de encodervalue
 } 

if(millis()- Time2>= 4000 && (funcionmenu==1 || funcionmenu>=4)) //temporizador de cada ,5 segundos para antena
{
  Time2 = millis();
  pantalla();    
}
if(millis()- Time3>= 8000 && (funcionmenu == 2 || funcionmenu == 3)) //temporizador de cada ,5 segundos para antena
{
  ajuste = 0;
  Time3 = millis();
  pantalla();  
  
}   
}
