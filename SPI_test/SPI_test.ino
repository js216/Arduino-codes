#include <SPI.h>

// SS = Select Device = pin 10
// SCK = Clock = pin 13
// MOSI = Data Output = pin 11
// MISO = Data Input = pin 12
const int CS_pin = 16;
const int IO_update_pin = 24;

unsigned int result[4];

void setup()
{
  SPI.begin();
  Serial.begin(9600);

  // CS is active-low
  pinMode(CS_pin, OUTPUT);
  digitalWrite(CS_pin, HIGH);
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      // read register
      case 'r':
        read_register(0x00);
        for (int i=0; i<4; i++) {
          Serial.print(result[i], BIN);
          Serial.print(",");
        }
        Serial.print("\n");
        break;
    }
  }
}

void loop()
{
}

void read_register(char reg)
{
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_pin, LOW);
  SPI.transfer(reg | B10000000);
  for (int i=0; i<4; i++)
    result[i] = SPI.transfer(0x00);
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}
