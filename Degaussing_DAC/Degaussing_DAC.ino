#include <SPI.h>

// pin configuration
const int CSB = 2;

// signal generation parameters
bool trig = false;
float freq = 10;
float t_rise = 1;
float t_decay = 5;
const unsigned int amp_max = 65535;

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
        Serial.println("Degaussing DAC board ready.");
        break;

      case 't':
        trig = true;
        break;

      case 'f':
        freq = Serial.parseInt();
        break;

      case 'r':
        t_rise = Serial.parseInt();
        break;

      case 'd':
        t_decay = Serial.parseInt();
        break;
    }
  }
}

void loop() {
  if (trig) {
    // initialize
    unsigned int amp = 1;
    const int dt = 1000 / 2 / freq;
    const int inc = amp_max / freq / t_rise;
    const float div_f = exp(1./(freq * t_decay));
    
    while (amp > 0) {
      // calculate amplitude
      if (trig && (amp < amp_max-inc))
        amp += inc;
      else {
        trig = false;
        amp /= div_f;
      }
      Serial.println(amp);

      // write one cycle
      delay(dt);
      write_SPI(amp);
      delay(dt);
      write_SPI(0);
    }
  }
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
