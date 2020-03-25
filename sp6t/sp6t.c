#define F_CPU 16000000UL

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "uart.h"

#define R1 _BV(PD2)
#define R2 _BV(PD3)
#define R3 _BV(PD4)
#define R4 _BV(PD5)
#define R5 _BV(PD6)
#define R6 _BV(PD7)

const int delay = 10;

int main()
{
    // initialize serial
    uart_init();

    // set output pins
    DDRD = R1 | R2 | R3 | R4 | R5 | R6;
    DDRB = _BV(PB5);

    while(1) {
        // read user input
        char c = uart_getchar();

        // decide what to do with it
        switch (c) {
            case 'a':
                PORTB = _BV(PB5);
                break;
            case 'b':
                PORTB = PORTB & ~(_BV(PB5));
                break;
            case '0':
                PORTD = 0;
                break;
            case '1':
                PORTD = 0;
                _delay_ms(delay);
                PORTD = R1;
                break;
            case '2':
                PORTD = 0;
                _delay_ms(delay);
                PORTD = R2;
                break;
            case '3':
                PORTD = 0;
                _delay_ms(delay);
                PORTD = R3;
                break;
            case '4':
                PORTD = 0;
                _delay_ms(delay);
                PORTD = R4;
                break;
            case '5':
                PORTD = 0;
                _delay_ms(delay);
                PORTD = R5;
                break;
            case '6':
                PORTD = 0;
                _delay_ms(delay);
                PORTD = R6;
                break;
        }
    }
}
