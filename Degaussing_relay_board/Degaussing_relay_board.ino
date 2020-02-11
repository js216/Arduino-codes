// pin configuration
#define R1 _BV(PD6)
#define R2 _BV(PD5)
#define R3 _BV(PD4)
#define R4 _BV(PD3)

const int relay_delay = 10;
int switch_state = 0;

void setup() {
  Serial.begin(9600);

  DDRB = R1 | R2 | R3 | R4;
  PORTD = 0;
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '0':
        PORTD = 0;
        break;
      case '1':
        PORTD = 0;
        delay(relay_delay);
        PORTD = R1;
        break;
      case '2':
        PORTD = 0;
        delay(relay_delay);
        PORTD = R2;
        break;
      case '3':
        PORTD = 0;
        delay(relay_delay);
        PORTD = R3;
        break;
      case '4':
        PORTD = 0;
        delay(relay_delay);
        PORTD = R4;
        break;
      case '?':
        Serial.write("Degaussing relay board ready.\n");
        break;
    }
  }
}

void loop() {
}
