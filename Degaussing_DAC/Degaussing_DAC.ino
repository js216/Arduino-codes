#include <SPI.h>

// pin configuration
const int CSB = 2;

// signal generation parameters
int T = 650;
int amp = 5;

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

      case 'a':
        amp = Serial.parseInt();
        break;

      case 't':
        T = Serial.parseInt();
        break;

      case 'g':
        set_gain(Serial.parseInt());
        break;

      case 'w':
        write_SPI(Serial.parseInt());
        break;
    }
  }
}

void loop() {
//  for (int i=0; i<T; i++) {
//    long n = 0x8000 * (sin(2*3.14159*i/T)+1) / amp;
//    write_SPI(n);
//  }
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
