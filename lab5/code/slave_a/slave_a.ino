#define F_CPU 16000000L
#define BAUD 9600
#define UBRRcalc (F_CPU/(BAUD*16L)-1)
#define BuffSize 128
#define BuffMask (BuffSize - 1)
#define CRC8_POLY 0x31

byte address = B01001100;

unsigned char BuffOUT[BuffSize], StartBuff = 0, EndBuff = 0, BuffErr = 0;


void writeToBuff(byte chr){
  BuffOUT[EndBuff++] = chr;
  EndBuff &= BuffMask;
  if(StartBuff==EndBuff)  BuffErr = 1;
}

int counter = 0;
 // Polynomial for CRC-8/MAXIM
const uint8_t crc8_table[] = {
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
    0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
    0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
    0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
    0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
    0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
    0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
    0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
    0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
    0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
    0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
    0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
    0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
    0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
};

uint8_t crc8_rohc(uint8_t *data, uint8_t len) {
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; ++i) {
        crc = crc8_table[crc ^ data[i]];
    }
    return crc;
}

void setWriteMode(){
  PORTD |= 1<<PD2;  
}

ISR(USART_TX_vect){
  PORTD &= ~(1<<PD2);  
}

ISR(USART_RX_vect){
  if(UDR0 == address){
    byte name[] = "Yaroslav Holoborodko Oleksandrovych";
    int length = sizeof(name)/sizeof(name[0]);
    StartBuff = 0, EndBuff = 0;
    byte ugly = 0;
    switch(counter){
      case 0:        //check if address
        for(int i=0; i < length; i++){  //write name[] to buffer
          writeToBuff(name[i]);
          PORTB = name[i];
        }
        counter++;
        break;
      case 1:
        for(int i=0; i < length; i++){  
          if(i==0){
            name[i]^=(1<<0);
          }
          writeToBuff(name[i]);  //write name[] to buffer
          PORTB = name[i];
        }
        counter++;
        break;
      case 2:
        for(int i=0; i < length; i++){  
          if(i==6){
            name[i]^=(1<<0)|(1<<4)|(1<<5);
          }
          writeToBuff(name[i]);  //write name[] to buffer
          PORTB = name[i];
        }
        counter = 0;
        break;    
    }
    byte crc = crc8_rohc(BuffOUT, length-1);    
    writeToBuff(crc);
    sprintf(BuffOUT, "%s %d", name, crc);
    EndBuff+=2;
    UCSR0B |= (1<<UDRIE0);
  //enable udre
  }
}

ISR(USART_UDRE_vect){
  PORTD |= 1<<PD2;
  UDR0 = BuffOUT[StartBuff++];
  StartBuff &= BuffMask;
  if(StartBuff == EndBuff) UCSR0B &= ~(1<<UDRIE0);
}


void setup() {
  //Enable
  DDRD = 0b00000111;
  PORTD = 0b11111000;

  //Address
  DDRC = 0x00;
  PORTC = 0xFF;

  DDRB = 0xFF;
  PORTB = 0x00;

  //UART
  UBRR0L = (unsigned char)(UBRRcalc);
  UBRR0H = (unsigned char)(UBRRcalc>>8);

  UCSR0C = (1<<UCSZ01)|(1<<UCSZ00)|(1<<USBS0);
  UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<TXCIE0);

  PORTB = address;

}

void loop() {
  if(BuffErr == 1) PORTD |= 1<<PD7;
}
