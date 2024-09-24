/*proyecto GELACC adquisición de solarimetro
 * V1.0.0 al 07-11-2023
 * ********************************************
 * Fecha y Hora
 * - Uso de funciones tomadas del ejemplo de RTClib.h en manejoCLock.h
 * - Configuración RTC
 * Manejo SD
 * - Funciones en manejoSD (configuración y almacenamiento de datos) 
 */
//#define _TEST_GPRS
//#include <SoftwareSerial.h>
#include "ManejoClock.h"
#include "ManejoSD.h"
#include <OneWire.h>                
#include <DallasTemperature.h>
#include "DHT.h"
#include "HX711.h"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Dirección I2C del módulo LCD
// Puedes encontrar la dirección correcta ejecutando un escáner I2C (I2C Scanner) en Arduino.
int lcdAddress = 0x27; // Cambia esto según la dirección de tu LCD I2C
// Definir el número de columnas y filas del LCD
int lcdColumns = 16;
int lcdRows = 2;
// Inicializar el objeto LCD
LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);
HX711 scale;

uint8_t dataPin = 9;
uint8_t clockPin = 8;
float temperaturaReferencia=50;
float histeresis=2;

OneWire ourWire(2);                //Se establece el pin 2  como bus OneWire 
DallasTemperature sensors(&ourWire); //Se declara una variable u objeto para nuestro sensor

#define tiempoDatos 600
#define DHTPIN1 3 
#define DHTPIN2 4    
#define DHTTYPE DHT22
//SoftwareSerial serialWifi(5, 6); // RX, TX
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
Registro Datos;
RTC_DS3231 rtc;
int pinRele=7;
int pin = 3;
volatile int state = LOW;
unsigned long tiempoInicial;
int tiempoRegistro=0;



/*void iniciarSoftwareSerial() {
  serialWifi.begin(19200);
};*/

void setup() {
  Serial.begin(57600);
  pinMode(3,OUTPUT);
  digitalWrite(3,HIGH);
  pinMode(pinRele,OUTPUT);
  digitalWrite(pinRele,LOW);
  analogReference(EXTERNAL);
  // put your setup code here, to run once:
  InicioSD();
  InicioRTC(rtc);
  Serial.println(F("Reset"));
  //iniciarSoftwareSerial();
  sensors.begin();   //Se inicia el sensor temperatura
  dht1.begin();
  dht2.begin();
  scale.begin(dataPin, clockPin);
  // TODO find a nice solution for this calibration..
  // load cell factor 20 KG
  scale.set_scale(105);
  // load cell factor 5 KG
  //scale.set_scale(420.0983);       // TODO you need to calibrate this yourself.
  // reset the scale to zero = 0
  //scale.tare();
  lcd.begin(lcdColumns, lcdRows);
  lcd.backlight();
  // Imprimir un mensaje de bienvenida
  lcd.setCursor(5, 0);
  lcd.print(F("GELACC"));
  lcd.setCursor(2, 1);
  lcd.print(F("SECADOR 2023"));
  // Esperar unos segundos
  delay(5000);

  // Limpiar la pantalla
  lcd.clear();
  
}

void loop() {
  comunicarSerial();
  
#ifndef _TEST_GPRS
if (millis() - tiempoInicial >= 1000)
  {
    //analogWrite(5, contadorPulso);
    tiempoInicial = millis();
  
     
    
    sensors.requestTemperatures();   //Se envía el comando para leer la temperatura
    float temperatura1= sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC
    float temperatura2= sensors.getTempCByIndex(1); //Se obtiene la temperatura en ºC
    
    Datos.Temperatura1=String(temperatura1);
    Datos.Temperatura2=String(temperatura2);
    Datos.dataString=String(TiempoActualUnix(rtc));
    float peso=scale.get_units(5);
    Datos.Peso=String(peso);
    tiempoRegistro++;
    float promedioTemperaturaInterior=0;
    float promedioHumedad=0;
    float temperaturaExterior=temperatura2;
    
    
    float h = dht1.readHumidity();
    float t = dht1.readTemperature();
    promedioHumedad=h;
    promedioTemperaturaInterior=t;
    
    Datos.TemperaturaH1=String(h);
    Datos.TemperaturaT1=String(t);
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      //return;
    /*}else{
      Serial.print("Humedad 1: ");
      Serial.println(h);
      Serial.print("Temperatura 1: ");
      Serial.println(t);*/
    }

    h = dht2.readHumidity();
    t = dht2.readTemperature();
    Datos.TemperaturaH2=String(h);
    Datos.TemperaturaT2=String(t);
    promedioHumedad+=h;
    promedioTemperaturaInterior+=t+temperatura1;
    promedioTemperaturaInterior/=3;
    promedioHumedad/=2;
    if(promedioTemperaturaInterior>temperaturaReferencia+histeresis)
      digitalWrite(pinRele,HIGH);
    if(promedioTemperaturaInterior<temperaturaReferencia-histeresis) 
      digitalWrite(pinRele,LOW);
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      //return;
    /*}else{
      Serial.print("Humedad 2: ");
      Serial.println(h);
      Serial.print("Temperatura 2: ");
      Serial.println(t);*/
    }
    lcd.setCursor(0, 0);
    lcd.print("Ti=");
    lcd.print((int)promedioTemperaturaInterior);
    lcd.print("C P=");
    lcd.print(peso/1000,3);
    lcd.print("Kg");
    lcd.setCursor(0, 1);
    lcd.print("Te=");
    lcd.print((int)temperaturaExterior);
    lcd.print("C H=");
    lcd.print((int)promedioHumedad);
    lcd.print("%");

    if(tiempoRegistro>=tiempoDatos){
      EscribirRegistro(Datos);
      tiempoRegistro=0;
      
      
    }
   }
  
#else
  //delay(10000);
  if (serialWifi.available()) {
    Serial.write(serialWifi.read());
  }
  if (Serial.available()) {
    serialWifi.write(Serial.read());
  }
#endif
}


void comunicarSerial() {
  char DatoSerial;
  if (Serial.available() > 0) {
    DatoSerial = Serial.read();
    if (DatoSerial == 'L') {
      delay(500);
      LeerRegistro();
    }
    if (DatoSerial == 'K') {
      BorrarSD();

    }
  }
}
