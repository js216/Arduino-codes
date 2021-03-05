#include <SPI.h>

const int CS_ADC = 10;
long int setpoint = 0;
byte data[3];
int default_ch = 1;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  pinMode(CS_ADC, OUTPUT);
  digitalWrite(CS_ADC, LOW);
  ADC_DAC_init();
}

void ADC_DAC_init()
{
  // set DAC for high gain
  DAC_gain(HIGH);

  // configure the ADC
  ADC_range(0, 0, 2, 2);
  ADC_control(0);
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.write("InampPID v2.0 board ready SP = ");
        Serial.println(setpoint);
        break;

      case 'i':
          ADC_DAC_init();
          break;

      case 'w':
        setpoint = Serial.parseInt();
        Serial.println(setpoint);
        DAC_write(setpoint);
        break;

      case 'd':
        default_ch = Serial.parseInt();
        break;

      case 'g':
        DAC_gain(Serial.parseInt());
        break;

      case 'r':
        for (int i=0; i<4; i++) {
          Serial.print("ch");
          Serial.print(i);
          Serial.print(": ");
          ADC_read(i);
        }
        break;

      case 'R':
        ADC_range(Serial.parseInt(), Serial.parseInt(),
                  Serial.parseInt(), Serial.parseInt());
        break;
    }
  }
}

void loop() {
//  ADC_read(default_ch);
}

/************************************
 * DAC FUNCTIONs
 ***********************************/
void DAC_SPI()
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_ADC, HIGH);

  // transfer data
  SPI.transfer(data, 3);

  // end transaction
  digitalWrite(CS_ADC, LOW);
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

/************************************
 * ADC FUNCTIONs
 ***********************************/

void ADC_SPI(int SPI_MODE)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE));
  digitalWrite(CS_ADC, HIGH);
  delayMicroseconds(10);
  digitalWrite(CS_ADC, LOW);

  // transfer data
  SPI.transfer(data, 2);

  // end transaction
  SPI.endTransaction();
}

void ADC_read(int ch_sel)
{
  // read data from ADC
  ADC_control(ch_sel);
  data[0] = 0;
  data[1] = 0;
  ADC_SPI(SPI_MODE0);

  // interpret received data
  int ch_rec = data[0] >> 5;
  signed int val = (((data[0] & B00011111) << 11) | (data[1] << 3)) >> 3;

  // verify correct channel received
  if (ch_rec == ch_sel)
    Serial.println(val);
  else
    Serial.println("error");
}

void ADC_control(int ch)
{
  data[0]  = (1 << 7);   // write to control register
  data[0] |= (ch << 2);  // select analog input channel for next conversion
  data[1]  = (1 << 4);   // enable internal reference
  ADC_SPI(SPI_MODE1);
}

void ADC_range(char vin0, char vin1, char vin2, char vin3)
{
  // vin = 0 means an input range of +/- 10 V
  // vin = 1 means an input range of +/- 5 V
  // vin = 2 means an input range of +/- 2.5 V
  // vin = 3 means an input range of 0 to 10 V
  
  data[0]  = (1 << 7); // write ...
  data[0] |= (1 << 5); // ... to the range register;
  data[0] |= (vin0 << 3);
  data[0] |= (vin1 << 1);
  data[0] |= (vin2 >> 1);
  data[1]  = ((vin2 & 1) << 7);
  data[1] |= (vin3 << 5);
  ADC_SPI(SPI_MODE1);
}
