// pin configuration
#define R1 _BV(PB1)
#define R2 _BV(PB0)
#define R3 _BV(PD7)
#define R4 _BV(PD6)
#define R5 _BV(PD5)
#define R6 _BV(PD4)
#define R7 _BV(PD3)
#define R8 _BV(PD2)

const int relay_delay = 10;

void setup() {
  Serial.begin(9600);

  DDRB = R1 | R2;
  DDRD = R3 | R4 | R5 | R6 | R7 | R8;
  PORTB = 0;
  PORTD = 0;
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '0':
        clear_all();
        break;
      case '1':
        clear_all();
        delay(relay_delay);
        PORTB = R1;
        break;
      case '2':
        clear_all();
        delay(relay_delay);
        PORTB = R2;
        break;
      case '3':
        clear_all();
        delay(relay_delay);
        PORTD = R3;
        break;
      case '4':
        clear_all();
        delay(relay_delay);
        PORTD = R4;
        break;
      case '5':
        clear_all();
        delay(relay_delay);
        PORTD = R5;
        break;
      case '6':
        clear_all();
        delay(relay_delay);
        PORTD = R6;
        break;
      case '7':
        clear_all();
        delay(relay_delay);
        PORTD = R7;
        break;
      case '8':
        clear_all();
        delay(relay_delay);
        PORTD = R8;
        break;
      case '?':
        Serial.write("Degaussing relay board v1.2 ready.\n");
        break;
    }
  }
}

void clear_all()
{
  PORTB = 0;
  PORTD = 0;
}

void loop() {
}
