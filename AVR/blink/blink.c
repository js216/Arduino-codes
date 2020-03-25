#include <avr/io.h>
#include <util/delay.h>

const int dt = 1000;
 
int main()
{
    DDRB = _BV(PB5);
    PORTB = 0x00;

    while(1) {
        _delay_ms(15);
        PORTB = 0xff;
    }
}
