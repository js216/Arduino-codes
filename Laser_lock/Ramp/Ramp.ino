#include <SPI.h>

// pin configuration
const int CSB = 2;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration
  pinMode(CSB, OUTPUT);
  digitalWrite(CSB, HIGH);
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.write("Ramp generator v1.0 board ready.\n");
        break;

      case 'e':
        digitalWrite(CSB, LOW);
        break;

      case 'd':
        digitalWrite(CSB, HIGH);
        break;

      case 'w':
        const int addr = Serial.parseInt();
        const int data = Serial.parseInt();
        Serial.println(addr);
        Serial.println(data);
        write_SPI(addr, data);
        break;
    }
  }
}

void loop() {
}


/* ============================
 * AUXILIARY FUNCTIONS
   ============================ */

void write_SPI(const unsigned int address, const unsigned int data)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CSB, LOW);

  // address
  SPI.transfer(address);

  // data
  SPI.transfer(data);

  // end transaction
  digitalWrite(CSB, HIGH);
  SPI.endTransaction();
}
