#include <SPI.h>

// pin configuration
const int CSB = 2;

void write_SPI(const unsigned int val)
{
  // compose data packet
  byte data[3];
  data[0] = 020 | ((val >> 12) & 0xff);
  data[1] = (val >> 4) & 0xff;
  data[2] = (val << 4) & 0xff;

  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(CSB, LOW);

  // transfer data
  for (int i=0; i<3; i++)
    SPI.transfer(data[i]);

  // end transaction
  digitalWrite(CSB, HIGH);
  SPI.endTransaction();
}

void set_gain(const bool gain)
{
  // compose data packet
  byte data[3];
  data[0] = 0x40;
  data[1] = gain << 7;
  data[2] = 0x00;

  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(CSB, LOW);

  // transfer data
  for (int i=0; i<3; i++)
    SPI.transfer(data[i]);

  // end transaction
  digitalWrite(CSB, HIGH);
  SPI.endTransaction();
}

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration
  pinMode(CSB, OUTPUT);
  digitalWrite(CSB, HIGH);

  // set DAC for high gain
  set_gain(HIGH);
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.write("Degaussing DAC board ready.\n");
        break;
    }
  }
}

void loop() {
  for (int i=0; i<0xffff; i++)
    write_SPI(0xffff*sin(i));
}
