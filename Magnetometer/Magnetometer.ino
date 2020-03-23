#include <SPI.h>

// pin configuration
const int OR[] = {A0, 5, 7}; // x, y, z
const int ERR[] = {A1, 6, 8}; // x, y, z
const int SEL[] = {A2, 10, 9}; // bsel, rsel0, rsel1
const int CSB=A3, SYNC=A4;

// for data transfer
byte byte0=0x00, byte1=0x00;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration
  pinMode(CSB, OUTPUT);
  digitalWrite(CSB, HIGH);
  pinMode(SYNC, INPUT);
  for (int i=0; i<3; i++) {
    pinMode(OR[i], INPUT);
    pinMode(ERR[i], INPUT);
    pinMode(SEL[i], OUTPUT);
  }
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.write("Magnetometer v2.1 board ready.\n");
        break;
    }
  }
}

void loop() {
}


/* ============================
 * AUXILIARY FUNCTIONS
   ============================ */

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}

void read_SPI(const unsigned int address, const int num_reg, const bool printing)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(SPI_speed, MSBFIRST, SPI_MODE));
  digitalWrite(CS_pin, LOW);

  // command to read configuration registers
  SPI.transfer(0x03);

  // 16-bit register address
  SPI.transfer16(address);

  // read the returned data
  for (int i=0; i<num_reg; i++)
    T0[i] = SPI.transfer(0x00);

  // print out the returned value to serial
  if (printing) {
    for (int i=0; i<num_reg; i++) {
      printBits(T0[i]);
      Serial.print(" ");
    }
    Serial.print("\n");
  }

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}

void write_SPI(const unsigned int address, const int num_reg, const unsigned char data[])
{
  // begin transaction
  SPI.beginTransaction(SPISettings(SPI_speed, MSBFIRST, SPI_MODE));
  digitalWrite(CS_pin, LOW);

  // command to write to configuration registers
  SPI.transfer(0x02);

  // 16-bit register address
  SPI.transfer16(address);

  // write serial data to SPI
  for (int i=0; i<num_reg; i++) {
    SPI.transfer(data[i]);
    delay(1);
  }

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}
