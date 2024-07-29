; Definitions
.def _temp1 = r16
.def _temp2 = r17
.def _temp3 = r18

; Code segment
.CSEG

; Initialization
ldi _temp1, 0x00        ; Load immediate 0x00 into _temp1
ldi _temp2, 0xFF        ; Load immediate 0xFF into _temp2

; Configure PORTA as output (LEDs)
out DDRA, _temp2        ; Set all pins on PORTA as output
out PORTA, _temp1       ; Initialize PORTA output to 0x00

; Configure PORTC
ldi _temp1, 0b00001000  ; Load immediate 0b00001000 into _temp1 (PC3 input, all others output)
out DDRC, _temp1        ; Set DDRC (PC3 as input, others as output)
sbi PORTC, 3            ; Enable pull-up resistor on PC3

main:
    ; Check button status on PC3
    sbis PINC, 3         ; Skip next instruction if PC3 is set (button not pressed)
    rjmp elseButton      ; If button pressed, jump to elseButton

    ; Button not pressed, set fast blink rate
    ldi _temp1, 0x00     ; Load immediate 0x00 into _temp1
    ldi _temp2, 0x6A     ; Load immediate 0x6A into _temp2
    ldi _temp3, 0x18     ; Load immediate 0x18 into _temp3
    rjmp delay           ; Jump to delay

elseButton:
    ; Button pressed, set slow blink rate
    ldi _temp1, 0x00     ; Load immediate 0x00 into _temp1
    ldi _temp2, 0x35     ; Load immediate 0x35 into _temp2
    ldi _temp3, 0x0C     ; Load immediate 0x0C into _temp3

delay:
    subi _temp1, 1       ; Subtract 1 from _temp1
    sbci _temp2, 0       ; Subtract with carry from _temp2
    sbci _temp3, 0       ; Subtract with carry from _temp3
    brne delay           ; Branch if result is not zero

    ; Toggle LEDs on PORTA
    in _temp1, PORTA     ; Read PORTA into _temp1
    com _temp1           ; Complement _temp1 (invert bits)
    out PORTA, _temp1    ; Write back to PORTA

    rjmp main            ; Jump back to main loop

