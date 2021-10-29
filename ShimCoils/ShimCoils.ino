#include <SPI.h>

// pin configuration
const int CS_DAC = 10;

// for SPI communication
unsigned char DAC_data[3];

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration
  pinMode(CS_DAC, OUTPUT);
  digitalWrite(CS_DAC, HIGH);
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.write("Shim coils v1.1 ready.\n");
        break;

      case 's':
      {
        const unsigned int ch = Serial.parseInt();
        const unsigned int val = Serial.parseInt();
        set_DAC(ch, 1000*val);
        break;
      }

      case 'r':
        read_DAC(Serial.parseInt());
        break;
    }
  }
}

void loop() {
}

/////////////////////////////////
/// DAC FUNTIONS ///////////////
/////////////////////////////////


void set_DAC(const unsigned int ch, const unsigned int val)
{
  // write and update DAC register ch
  DAC_data[0] = (B0011 << 4) + ch;
  DAC_data[1] = (val >> 8);
  DAC_data[2] = val;
  DAC_SPI();
}

void read_DAC(const unsigned int ch)
{
  Serial.println(0);
  // request data from register ch
  DAC_data[0] = (B1001 << 4) + ch;
  DAC_data[1] = 0;
  DAC_data[2] = 0;
  DAC_SPI();

  // read back the returned data
  DAC_data[0] = 0;
  DAC_data[1] = 0;
  DAC_data[2] = 0;
  DAC_SPI();

  // print to serial
  const unsigned int val = (DAC_data[1] << 8) + DAC_data[2];
  Serial.println(val);
  }

void DAC_SPI()
{
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(CS_DAC, LOW);
  SPI.transfer(DAC_data, 3);
  digitalWrite(CS_DAC, HIGH);
  SPI.endTransaction();
}
