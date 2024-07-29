.include "m2560def.inc"
.def _temp1 = r16
.def _temp2 = r17
.def _temp3 = r18
.def _temp4 = r21
.def _counter = r19
.def _counter1 = r24
.def _algo1_is_run = r22
.def _algo2_is_run = r23
.def _is_odd = r20

.macro outi
	ldi _temp4, @1
	.if @0 < 0x40
	out @0, _temp4
	.else
	sts @0, _temp4
	.endif
.endm

	.CSEG
	.org 0x0000
    rjmp RESET
	.org 0x005E
	rjmp TIMER5_COMPA_vect
	.org 0x0060
    rjmp TIMER5_COMPB_vect

TIMER5_COMPA_vect:
	sbrs _algo1_is_run, 0
	reti
	cpi _counter, 0b00000000
	breq COMPA_end
	out PORTF, _counter
	lsl _temp2
	lsr _temp3
	clr _counter
	or _counter, _temp2
	or _counter, _temp3
	reti

TIMER5_COMPB_vect:
	sbrs _algo2_is_run, 0
	reti 
	cpi _counter1, 0
	breq COMPB_end
	sbrc _is_odd, 0
	lsl _counter1
	sbrc _is_odd, 0
	lsl _counter1
	ldi _is_odd, 0
	sts PORTK, _counter1
	sbrc _counter1, 1
	ldi _is_odd, 1
	sbrc _counter1, 1
	ldi _counter1, 0x40
	lsr _counter1
	lsr _counter1
	reti


COMPA_end:
	dec _algo1_is_run
	ldi _counter, 0x18
	outi PORTF, 0x00
	reti 

COMPB_end:
	dec _algo2_is_run
	ldi _counter1, 0x80
	outi PORTK, 0x00
	reti


RESET:
	ldi r16, Low(RAMEND)
	out SPL, r16
	ldi r16, High(RAMEND)
	out SPH, r16

	ldi _temp1, 0x00
	ldi _temp2, 0xFF
	ldi _counter, 0x18
	ldi _counter1, 0x80
	ldi _algo1_is_run, 0x00
	ldi _algo2_is_run, 0x00
	;Порт B на вхід з підтягуючим резистором
	out DDRC, _temp1
	out PORTC, _temp2
	;Порт С на вихід
	out DDRF, _temp2
	out PORTF, _temp1

	outi DDRA, 0xFF
	outi PORTA, 0x00

	outi DDRK, 0xFF
	outi PORTK, 0x00
	;Налаштувати переривання, далі у перериваннях перевіряти чи не була 
	;нажата унопка і якшо була то почати виконувати алгоритм(доки не виконається умова)
	ldi _temp1, 0x00
	sts TCCR5A, _temp1
	ldi _temp1, (1<<WGM52)|(1<<CS52)|(1<<CS50)
	sts TCCR5B, _temp1
	ldi _temp1, (1<<OCIE5A)|(1<<OCIE5B)
	sts TIMSK5, _temp1
	ldi _temp1, 0x21
	sts OCR5AH, _temp1
	ldi _temp1, 0x91
	sts OCR5AL, _temp1
	ldi _temp1, 0x91
	sts OCR5BL, _temp1
	ldi _temp1, 0x21
	sts OCR5BH, _temp1
	ldi _temp2, 0b00001000
	ldi _temp3, 0b00010000
	sei

main: 
check_btn_1:
	sbic PINC, 3
	rjmp check_btn_2
	call delay_setup
	ldi _counter, 0x18
	ldi _temp2, 0b00001000
	ldi _temp3, 0b00010000
	ldi _algo1_is_run, 0x01
	nop
check_btn_2:
	sbic PINC, 5
	rjmp end
	call delay_setup
	ldi _counter1, 0x80
	ldi _is_odd, 0
	ldi _algo2_is_run, 0x01
	nop
end:
	rjmp main

delay_setup:
  ldi _temp1, 0x00
  ldi _temp2, 0x6A
  ldi _temp3, 0x18 
  call delay
  ret

delay:
  outi PORTA, 0x01
  subi _temp1, 1
  sbci _temp2, 0
  sbci _temp3, 0
  brne delay
  outi PORTA, 0x00
  ret



