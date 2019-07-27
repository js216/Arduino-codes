byte tableB[] = {16, 17, 19, 18, 0, 1, 32, 25};
byte tableC[] = {15, 22, 23, 9, 10, 13, 11, 12, 28, 27, 29, 30};
byte tableD[] = {2, 14, 7, 8, 6, 20, 21, 5};

#include <SPI.h>

const int CS_pin = 7;

unsigned int result[4];

voidspi setup()
{
  SPI.begin();

  pinMode(CS_pin, OUTPUT);
  digitalWrite(CS_pin, HIGH);
}

void loop()
{
}

void read_register(char reg)
{
  // create the read register instruction
  char instr = reg | B10000000;
  
  // do the SPI data transfer
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_pin, LOW);
  SPI.transfer(instr);
  for (int i=0; i<4; i++)
    result[i] = SPI.transfer(0x00);
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}
