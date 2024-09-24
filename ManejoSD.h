#define NombreArchivo "dataSol.txt"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#ifndef _MANEJOSD
#define _MANEJOSD


class Registro{
  public:
  String dataString;
  String Temperatura1;
  String Temperatura2;
  String TemperaturaH1;
  String TemperaturaH2;
  String TemperaturaT1;
  String TemperaturaT2;
  String Peso;
  String Cadena() {
	  return dataString + "," + Temperatura1 + "," + Temperatura2 
	  + "," + TemperaturaH1 + "," + TemperaturaT1 + "," + TemperaturaH2 + "," + TemperaturaT2
    + "," + Peso;
    }
  };
 
const int chipSelect = 10; // 53;//
/*
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10
 **
  */

void InicioSD();
void EscribirRegistro(Registro Dato);
void LeerRegistro(void);
void BorrarSD(void);


#endif // !_MANEJOSD
