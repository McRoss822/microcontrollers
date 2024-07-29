#define F_CPU 16000000L
#define BAUD 9600
#define UBRRcalc (F_CPU/(BAUD*16L)-1)
#define BUFF_SIZE 128
#define BUFF_MASK (BUFF_SIZE-1)

byte BuffIN[BUFF_SIZE], StartBuff = 0, EndBuff = 0, BuffErr = 0;

byte slv_a = B01001100;
byte slv_b = B01000011;

void setWriteMode(){
  PORTD |= 1<<PD1;  
}

byte writeToBuff(byte chr){
  BuffIN[EndBuff++] = chr;
  EndBuff &= BUFF_MASK;
  if(StartBuff==EndBuff) BuffErr = 1;

}

byte readFromBuff(void){
  byte read = BuffIN[StartBuff++];
  StartBuff &= BUFF_MASK;
  if(StartBuff==EndBuff) BuffErr = 1;
  return read;
}

ISR(USART1_RX_vect){
  byte received = UDR1;
  writeToBuff(received);
}

ISR(USART1_TX_vect){
  PORTD &= ~(1<<PD1);
}

void setup() {
  //Enable
  DDRD |= 1<<PD1;
  PORTD |= (1<<PD1);

  DDRB = 0xFF;
  PORTB = 0x00;

  //UART
  Serial.begin(9600);

  UBRR1L = (unsigned char)(UBRRcalc);
  UBRR1H = (unsigned char)(UBRRcalc>>8);
  //обнулення регістра UCSRA
  UCSR1A = 0;
  UCSR1C = (1<<UCSZ11)|(1<<UCSZ10)|(1<<USBS1); // Налаштування режиму передачі: 8 біт, 1 стоповий біт (8n1) для Serial 0
  // дозвіл прийому-передачі+перерив.прийому, заверш.перед.+ 9n
  UCSR1B = (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1);

}

void loop() {

  if(Serial.available()>0){
    byte inByte = Serial.read();
    PORTD |= 1<<PD1;
    // PORTB = 0xF0;
    if(inByte==0xA1){
      StartBuff = EndBuff;
      // PORTB = 0x0F;
      // PORTD |= 1<<PD1;
      UDR1 = slv_a;
    }else{ 
      StartBuff = EndBuff;
      UDR1 = slv_b;
      };
  }

  while(StartBuff!=EndBuff){
    Serial.write(readFromBuff());
  }
  
    // Serial.write('a');

}
