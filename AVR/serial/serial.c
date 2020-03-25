#include <avr/io.h>
#include <util/delay.h>

int main()
{
    // set serial
    const unsigned int ubrr = 71;//F_CPU/(16*BAUD_RATE) - 1;
    UBRR0H = (ubrr>>8);
    UBRR0L = (ubrr);
    UCSR0C = 0x06;       // set frame format: 8data, 1stop bit
    UCSR0B = (1<<TXEN0); /* Enable  transmitter                 */

    int i = 0;
    unsigned char data[] = "Hello from ATmega328p";

    while(1)
    { 
        i = 0;
        while(data[i] != 0)
        {
            while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer       */
            UDR0 = data[i];                  /* Put data into buffer, sends the data */
            i++;                             /* increment counter                    */
        }
        _delay_ms(1000);
    }   
}

