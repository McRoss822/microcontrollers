#include <LiquidCrystal.h>

#define DDR_KEYPAD  DDRC
#define PORT_KEYPAD PORTC
#define PIN_KEYPAD  PINC
#include "keypad4x4.h"

const int buzzerPin = 21;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 5, rw = 6, en = 7, d4 = 47, d5 = 46, d6 = 45, d7 = 44;
LiquidCrystal lcd(rs, rw, en, d4, d5, d6, d7);

const PROGMEM  char sixty[60][3] = {
  {"00"}, {"01"}, {"02"}, {"03"}, {"04"}, {"05"}, {"06"}, {"07"}, {"08"}, {"09"},
  {"10"}, {"11"}, {"12"}, {"13"}, {"14"}, {"15"}, {"16"}, {"17"}, {"18"}, {"19"},
  {"20"}, {"21"}, {"22"}, {"23"}, {"24"}, {"25"}, {"26"}, {"27"}, {"28"}, {"29"},
  {"30"}, {"31"}, {"32"}, {"33"}, {"34"}, {"35"}, {"36"}, {"37"}, {"38"}, {"39"},
  {"40"}, {"41"}, {"42"}, {"43"}, {"44"}, {"45"}, {"46"}, {"47"}, {"48"}, {"49"},
  {"50"}, {"51"}, {"52"}, {"53"}, {"54"}, {"55"}, {"56"}, {"57"}, {"58"}, {"59"}
};

struct Time
{
  unsigned char second, minute, hour;
};
Time T1 = {0, 0, 0};
Time T2 = {0, 0, 0};

volatile int launchTimer1 = 0;
volatile int launchTimer2 = 0;

void LCD_WriteStrPROGMEM(char *str, int n)  //вивід масиву символів,
{ //записаних у флеші
  for (int i = 0; i < n; i++)
    lcd.print( (char)pgm_read_byte( &(str[i]) ) );
}

void setup() {
  noInterrupts();    

  TCCR5A = 0x00;
  TCCR5B = (1 << WGM52) | (1 << CS52) | (1 << CS50); //CTC mode & Prescaler @ 1024
  TIMSK5 = (1 << OCIE5A); // дозвіл на переривання по співпадінню
  OCR5A = 0x3D08;// compare value = 1 sec (16MHz AVR)
  
  //KeyPad 4x4
  initKeyPad();
  
  lcd.begin(16, 2);

  //Buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
}

void setTimer(Time& timer) {
  launchTimer1 = 0;
  launchTimer2 = 0;
  
  
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Set Timer"); 

  char key; 
  char timerDisplay[6]; 
  Time timer_set = {0, 0, 0}; 
  int digitCount = 0; 
  
  
  while (true) {
    lcd.setCursor(0, 1); 
    lcd.print("                "); 
    
    lcd.setCursor(0, 1);
    sprintf(timerDisplay, "%02d:%02d", timer_set.hour, timer_set.minute);
    lcd.print(timerDisplay);

    while (!isButtonPressed());
    
    key = readKeyFromPad4x4();

    switch(key) {
      case 'E': 
        if (timer_set.hour < 24 && timer_set.minute < 60) {
          timer = timer_set;
          buzzerOn(100);
          delay(300);
          buzzerOn(100);
         
          return;
        } else {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.write("ERROR");
          buzzerOn(100);
          timer_set = {0, 0, 0};
          digitCount = 0; 
        }
        break;
      
      case 'F': 
        buzzerOn(10000);
        return;
        
      default:
        buzzerOn(10000);
        if (key >= '0' && key <= '9') {
          if (digitCount < 2) {
            timer_set.hour = timer_set.hour * 10 + (key - '0');
            digitCount++;
          } else if (digitCount < 4) { 
            timer_set.minute = timer_set.minute * 10 + (key - '0');
            digitCount++;
          }
        }
        break;
    }
  }
}

void displayTimer(Time& t, int timer_number){
  if (timer_number == 1 || timer_number == 2){
  lcd.setCursor(3, timer_number-1);
  LCD_WriteStrPROGMEM(sixty[t.hour], 2);
  lcd.write(':');
  LCD_WriteStrPROGMEM(sixty[t.minute], 2);
  lcd.write(':');
  LCD_WriteStrPROGMEM(sixty[t.second], 2);
  }
}

void buzzerOn(unsigned long n){
   digitalWrite(buzzerPin, HIGH);
   delayMicroseconds(n);
   digitalWrite(buzzerPin, LOW);
  }

void loop() {
    if ( isButtonPressed() ) {
        char buttonPressed = readKeyFromPad4x4();
        buzzerOn(100);
        switch(buttonPressed){
          case 'A':
            noInterrupts();
            setTimer(T1);
            lcd.clear();
            displayTimer(T1, 1);
            displayTimer(T2, 2);
            break;
          case 'B':
            noInterrupts();
            setTimer(T2);
            lcd.clear();
            displayTimer(T1, 1);
            displayTimer(T2, 2);
//            interrupts();
            break; 
          case 'C':
            if (launchTimer1 == 1){
              launchTimer1 = 0;
              T1 = {0,0,0};
              displayTimer(T1, 1);
            } else{ 
                launchTimer1 = 1;
                interrupts();
              }
            break;
          case 'D':
            if (launchTimer2 == 1){
              launchTimer2 = 0;
              T2 = {0,0,0};
              displayTimer(T2, 2);
            } else{ 
                launchTimer2 = 1;
                interrupts();
              }
            break;
          default:
            break;
        }
    }
}

ISR(TIMER5_COMPA_vect)  // Таймер Т1 по співпадінню А, кожної 1 сек.
{
  int lnch1 = launchTimer1;
  int lnch2 = launchTimer2;
  if (lnch1 == 1){
    if (T1.second == 0) {
      T1.second = 59;
      if (T1.minute == 0) {
        T1.minute = 59;
        if (T1.hour == 0){
         T1.hour = 23; 
        }else T1.hour--;
      } else T1.minute--;
    } else {
        T1.second--;
        digitalWrite(buzzerPin, HIGH);
        _delay_ms(10);
        digitalWrite(buzzerPin, LOW);
   }
 }
  if (lnch2 == 1){
    if (T2.second == 0) {
      T2.second = 59;
      if (T2.minute == 0) {
        T2.minute = 59;
        if (T2.hour == 0)
          T2.hour = 23;
        else T2.hour--;
      } else T2.minute--;
    } else {
        T2.second--;
        digitalWrite(buzzerPin, HIGH);
        _delay_ms(10);
        digitalWrite(buzzerPin, LOW);
   }
 }
  
  displayTimer(T1, 1);
  displayTimer(T2, 2);
  
  if(T1.hour == 0 && T1.minute == 0 && T1.second  == 0 && launchTimer1 == 1){
    T1 = {0,0,0};
    launchTimer1 = 0;
    displayTimer(T1, 1);
    digitalWrite(buzzerPin, HIGH);
    _delay_ms(10000);
    digitalWrite(buzzerPin, LOW);
  } else if (T2.hour == 0 && T2.minute == 0 && T2.second  == 0 && launchTimer2 == 1){
      T2 = {0,0,0};
      launchTimer2 = 0;
      displayTimer(T2, 2);
      digitalWrite(buzzerPin, HIGH);
      _delay_ms(10000);
      digitalWrite(buzzerPin, LOW);
  }
}
