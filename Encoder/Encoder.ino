#include <Encoder.h> // include the encoder library

Encoder enc(3, 4); // create an encoder object

void setup() {
  Serial.begin (9600);
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();
    switch (cmd) {
      case '?':
        Serial.println(enc.read(), DEC);
        break;
      case '0':
        enc.write(0);
        break;
    }
  }
}

void loop() {
}
