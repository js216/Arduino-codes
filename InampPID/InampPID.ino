#include <SPI.h>
#include <Encoder.h>
const int CSB = 10;
const int SW_DIP = 2;
const int R_POT = A7;

long int setpoint = 0;
byte data[3];

void setup() {
  Serial.begin(9600);
  SPI.begin();
  pinMode(SW_DIP, INPUT);
  pinMode(R_POT, INPUT);
  pinMode(CSB, OUTPUT);
  digitalWrite(CSB, HIGH);
  DAC_gain(HIGH);
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.println("InampPID v2.2 board ready");
        break;

      case 'w':
        setpoint = Serial.parseInt();
        Serial.println(setpoint);
        DAC_write(setpoint);
        break;

      case 'g':
        DAC_gain(Serial.parseInt());
        break;
    }
  }
}

void loop() {
  if (digitalRead(SW_DIP))
    DAC_write(analogRead(R_POT) << 6);
}

/************************************
 * DAC FUNCTIONs
 ***********************************/
void DAC_SPI()
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CSB, LOW);

  // transfer data
  SPI.transfer(data, 3);

  // end transaction
  digitalWrite(CSB, HIGH);
  SPI.endTransaction();
}

void DAC_write(const unsigned int val)
{
  data[0] = 020 | ((val >> 12) & 0xff);
  data[1] = (val >> 4) & 0xff;
  data[2] = (val << 4) & 0xff;
  DAC_SPI();
}

void DAC_gain(const bool gain)
{
  data[0] = 0x40;
  data[1] = gain << 7;
  data[2] = 0x00;
  DAC_SPI();
}
