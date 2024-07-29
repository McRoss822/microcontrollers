#include <Wire.h>
#include <RTCx.h>

const int buttonPin1 =  34; // PC3 для кнопки 1
const int buttonPin2 =  32; // PC5 для кнопки 2
int numDiodes = 8;
int diodePins[] = {62, 63, 64, 65, 66, 67, 68, 69}; // Піни світлодіодів
int inByte;

void setup() {
  DDRK = 0xFF; // Встановлюємо всі піни порту K як виходи
  PORTK = 0; 
  DDRB = 0xff;
  PORTB = 0;  // Вимикаємо всі світлодіоди на початку
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP); 
  Serial.begin(9600);
  if(rtc.autoprobe()) PORTK = 0xff;;
  rtc.enableBatteryBackup();
	rtc.startClock(); 
}

void loop() {
  if (Serial.available()) {
    inByte = Serial.read();
    if (inByte == 0xA1) {
      
    } else if (inByte == 0xA2) {
     
    }
    }
  }


