#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
 
int main()
{
    DDRB = _BV(PB5);  // Port D2 (Pin 4 in the ATmega) made output 
    PORTB = 0x00; // Turn LED off

    while(1) {
	PORTB = _BV(PB5); // Turn LED on
	_delay_ms(200);     // delay of 200 millisecond
	PORTB = 0x00; // Turn LED off
	_delay_ms(200);     // delay of 200 millisecond
    }
}
