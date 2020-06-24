#include <SPI.h>

// pin configuration
const int CSB_seed1 = 23;
const int CSB_seed2 = 15;
const int CSB_ramp  = 14;
const int trig_in   = 16;
const int peak1     = 3;
const int peak2     = 9;
const int peak3     = 10;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration
  pinMode(CSB_seed1, OUTPUT);
  digitalWrite(CSB_seed1, HIGH);
  pinMode(CSB_seed2, OUTPUT);
  digitalWrite(CSB_seed2, HIGH);
  pinMode(CSB_ramp, OUTPUT);
  digitalWrite(CSB_ramp, HIGH);
  pinMode(trig_in, INPUT);
//  pinMode(peak1, INPUT);
//  pinMode(peak2, INPUT);
//  pinMode(peak3, INPUT);

  // set DACs for high gain
  set_gain(CSB_seed1, HIGH);
  set_gain(CSB_seed2, HIGH);

  // set ramp
  ramp_offset(120);
  ramp_R1(120);
  ramp_R3(120);
}

void loop() {
  if (digitalRead(peak1) == HIGH)
    set_seed(CSB_seed1, 100000);
  else
    set_seed(CSB_seed1, 10000);
}


void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.write("Lock board ready.\n");
        break;

      case '1':
        set_seed(CSB_seed1, Serial.parseInt());
        break;

      case '2':
        set_seed(CSB_seed2, Serial.parseInt());
        break;

      case 'o':
        ramp_offset(Serial.parseInt());
        break;

      case 'a':
        ramp_R1(Serial.parseInt());
        break;

      case 'b':
        ramp_R3(Serial.parseInt());
        break;
    }
  }
}

/////////////////////////////////
/// RAMP FUNTIONS ///////////////
/////////////////////////////////

void ramp_offset(const unsigned int val)
{
  ramp_SPI(1, val);
}

void ramp_R1(const unsigned int val)
{
  ramp_SPI(2, val);
}

void ramp_R3(const unsigned int val)
{
  ramp_SPI(0, val);
}

void ramp_SPI(const unsigned int address, const unsigned int data)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CSB_ramp, LOW);

  // address
  SPI.transfer(address);

  // data
  SPI.transfer(data);

  // end transaction
  digitalWrite(CSB_ramp, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////
/// SEED OUTPUT FUNCTIONS ///////
/////////////////////////////////

void set_seed(const int CSB, const unsigned int val)
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

void set_gain(const int CSB, const bool gain)
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
