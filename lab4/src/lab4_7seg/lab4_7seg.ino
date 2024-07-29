const int buzzerPin = 29;

unsigned char number[10] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90};
unsigned char bcd_time1[6] = {number[0], number[0], number[0], number[0], number[0], number[0]};
unsigned char bcd_time2[6] = {number[0], number[0], number[0], number[0], number[0], number[0]};

struct Time {
  unsigned char second, minute, hour;
};

Time T1 = {0, 0, 0};
Time T2 = {0, 0, 0};

unsigned char key_cursor = 1;

unsigned char timer_chosen = 0; // 0 for Timer 1, 1 for Timer 2
unsigned char t1_is_launched = 0; // Flag to track if Timer 1 is launched
unsigned char t2_is_launched = 0; // Flag to track if Timer 2 is launched


void setup() {
  noInterrupts(); // Disable all interrupts

  DDRA = 0xFF; // Port A set as output for display
  PORTA = 0x00; // Initialize port A

  DDRB = 0xFF; // Port B set as output for display
  PORTB = 0xFF; // Turn off all segments initially

  DDRD = 0x00; // Port D set as input for buttons
  PORTD = 0xFF; // Enable pull-up resistors on all pins of Port D

  // Timer 0 setup for display multiplexing
  TCNT0 = 0;
  OCR0A = 25;
  TCCR0A = (1 << WGM01); // CTC mode
  TCCR0B = (1 << CS02) | (1 << CS00); // Prescaler 1024
  TIMSK0 |= (1 << OCIE0A); // Enable Timer 0 interrupt

  // Timer 1 setup for real-time clock
  TCCR1A = 0x00;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC mode & Prescaler @ 1024
  OCR1A = 0x12C0; // Compare value for 1 second (4.9152MHz AVR)
  TIMSK1 |= (1 << OCIE1A); // Enable Timer 1 interrupt

  interrupts(); // Enable global interrupts

  // Initialize buzzer pin
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
}

void loop() {
  // Toggle between Timer 1 and Timer 2 when PIND2 button is pressed
  if ((PIND & (1 << 2)) == 0) {
    if (timer_chosen == 0) {
      timer_chosen = 1;
    } else {
      timer_chosen = 0;
    }
    delay(200); // Button debounce delay
  }

  // Handle Timer reset or launch when PIND3 button is pressed
  if ((PIND & (1 << 3)) == 0) {
    if (timer_chosen == 0) {
      // Timer 1 selected
      T1.hour = 0;
      T1.minute = 0;
      T1.second = 0;
      t1_is_launched = 1;
      bcd(bcd_time1, T1.hour, 0); // Update hours display
      bcd(bcd_time1, T1.minute, 2); // Update minutes display
      bcd(bcd_time1, T1.second, 4); // Update seconds display
    } else {
      // Timer 2 selected
      T2.hour = 0;
      T2.minute = 0;
      T2.second = 0;
      t2_is_launched = 1;
      bcd(bcd_time2, T2.hour, 0); // Update hours display
      bcd(bcd_time2, T2.minute, 2); // Update minutes display
      bcd(bcd_time2, T2.second, 4); // Update seconds display
    }
    delay(200); // Button debounce delay
  }

  // Decrement minutes of the selected timer when PIND1 button is pressed
  if ((PIND & (1 << 1)) == 0) {
    if (timer_chosen == 0 && t1_is_launched) {
      if (T1.minute == 0) {
        T1.minute = 59;
        if (T1.hour == 0) {
          T1.hour = 23;
        } else {
          T1.hour--;
        }
      } else {
        T1.minute--;
      }
      bcd(bcd_time1, T1.hour, 0); // Update hours display
      bcd(bcd_time1, T1.minute, 2); // Update minutes display
      delay(100); // Button debounce delay
    } else if (timer_chosen == 1 && t2_is_launched) {
      if (T2.minute == 0) {
        T2.minute = 59;
        if (T2.hour == 0) {
          T2.hour = 23;
        } else {
          T2.hour--;
        }
      } else {
        T2.minute--;
      }
      bcd(bcd_time2, T2.hour, 0); // Update hours display
      bcd(bcd_time2, T2.minute, 2); // Update minutes display
      delay(100); // Button debounce delay
    }
  }
}

// Interrupt service routine for Timer 0 (display multiplexing)
ISR(TIMER0_COMPA_vect) {
  key_cursor <<= 1;
  key_cursor &= 0b00111111;
  if (key_cursor == 0) key_cursor = 1;

  if (timer_chosen == 0) {
    PORTA = key_cursor;
    PORTB = bcd_time1[key_cursor];
  } else {
    PORTA = key_cursor;
    PORTB = bcd_time2[key_cursor];
  }
}

// Interrupt service routine for Timer 1 (1-second timer for clock)
ISR(TIMER1_COMPA_vect) {
  if (timer_chosen == 0 && t1_is_launched) {
    if (T1.second == 0) {
      T1.second = 59;
      if (T1.minute == 0) {
        T1.minute = 59;
        if (T1.hour == 0) {
          // Timer 1 countdown complete
          t1_is_launched = 0; // Stop Timer 1
          // Optionally trigger a buzzer or perform other actions upon countdown completion
        } else {
          T1.hour--;
        }
      } else {
        T1.minute--;
      }
    } else {
      T1.second--;
    }
    bcd(bcd_time1, T1.hour, 0); // Update hours display
    bcd(bcd_time1, T1.minute, 2); // Update minutes display
    bcd(bcd_time1, T1.second, 4); // Update seconds display
  }

  if (timer_chosen == 1 && t2_is_launched) {
    if (T2.second == 0) {
      T2.second = 59;
      if (T2.minute == 0) {
        T2.minute = 59;
        if (T2.hour == 0) {
          // Timer 2 countdown complete
          t2_is_launched = 0; // Stop Timer 2
          // Optionally trigger a buzzer or perform other actions upon countdown completion
        } else {
          T2.hour--;
        }
      } else {
        T2.minute--;
      }
    } else {
      T2.second--;
    }
    bcd(bcd_time2, T2.hour, 0); // Update hours display
    bcd(bcd_time2, T2.minute, 2); // Update minutes display
    bcd(bcd_time2, T2.second, 4); // Update seconds display
  }
}

void bcd(unsigned char *bcd_time, unsigned char fig_in, unsigned char position) {
  unsigned char bcdL = fig_in;
  unsigned char bcdH = 0;
  while(bcdL>=10){
    bcdL -= 10;
    bcdH++;
  }
  bcd_time[position] = number[bcdH];
  bcd_time[position+1] = number[bcdL];
}
