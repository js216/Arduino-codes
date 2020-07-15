#include <SPI.h>

// pin configuration
const int CSB = 2;

// signal generation parameters
const unsigned long int amp_init = 65535;
unsigned long int amp = 0;
float div_f = 1.00;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration
  pinMode(CSB, OUTPUT);
  digitalWrite(CSB, HIGH);

  // configure the DAC
  set_gain(LOW);
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.print("Degaussing DAC board ready; amp = ");
        Serial.print(amp);
        Serial.print(", div_f = ");
        Serial.print(div_f, 4);
        Serial.println(".");
        break;

      case 'r':
        amp = amp_init;
        break;

      case 'd':
        div_f = Serial.parseFloat();
        break;

      case 'w':
        amp = Serial.parseInt();
        break;
    }
  }
}

void loop() {
  write_SPI(amp);
  delay(50);
  write_SPI(0);
  amp /= div_f;
  delay(50);
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
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE1));
  digitalWrite(CSB, LOW);

  // transfer data
  SPI.transfer(data, 3);

  // end transaction
  digitalWrite(CSB, HIGH);
  SPI.endTransaction();
}
