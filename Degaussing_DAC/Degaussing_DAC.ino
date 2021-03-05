#include <SPI.h>

// pin configuration
const int CSB = 10;

// signal generation parameters
bool trig = false;
int di=5, dt=0;
float t_rise = 1;
float t_decay = 5;
float amp_f = 0.1;
int pulse_mode = 0;
bool step_forward = false;

// internal parameters
const unsigned int amp_max = 65535;
int amp, inc;
float div_f;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration
  pinMode(CSB, OUTPUT);
  digitalWrite(CSB, HIGH);

  // configure the DAC
  set_DAC();
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.println("Degaussing DAC board v2.0 ready.");
        break;

      case '0':
        trig = false;
        amp = 0;
        break;

      case 's':
        step_forward = true;
        break;

      case 'm':
        pulse_mode = Serial.parseInt();
        break;

      case 'f':
        di = Serial.parseInt();
        break;

      case 'F':
        dt = Serial.parseFloat();
        break;

      case 'D':
        div_f = Serial.parseFloat();
        break;

      case 'a':
        amp_f = Serial.parseFloat();
        break;

      case 'w':
        write_SPI(Serial.parseInt());
        break;

      case 'r':
        t_rise = Serial.parseFloat();
        break;

      case 'd':
        t_decay = Serial.parseFloat();
        break;

      case 'R':
        set_DAC();
        break;

      case 'c':
        readback_control();
        break;
        
      case 't':
        float freq = di * 4.44;
        inc = amp_max / freq / t_rise;
        div_f = exp(1./(freq * t_decay));
        trig = true;
        if (pulse_mode == 2)
          amp = amp_max;
        break;
    }
  }
}

void loop() {
  // EXPONENTIAL DECAY MODE
  if (pulse_mode == 0) {
    // calculate amplitude
    if (trig && (amp < amp_max-inc))
      amp += inc;
    else {
      trig = false;
      amp /= div_f;
    }
  
    // write one cycle
    for (int i=0; i<amp_max; i+=di)
      write_SPI( amp_f*amp*sin(2*3.14159*i/amp_max)/2 + amp_max/2 );

  // SINGLE-PULSE MODE
  } else if (pulse_mode == 1) {
    if (trig) {
      for (int i=0; i<amp_max; i+=di)
        write_SPI( amp_f*amp_max*sin(2*3.14159*i/amp_max)/2 + amp_max/2 );
        trig = false;
    }

  // STEPPED EXP DECAY MODE
  } else if (pulse_mode == 2) {
    if (step_forward) {
      amp /= div_f;
      for (int i=0; i<amp_max; i+=di) {
        write_SPI( amp_f*amp*sin(2*3.14159*i/amp_max)/2 + amp_max/2 );
        delayMicroseconds(dt);
      }
      step_forward = false;
    }
  }
}

void set_DAC()
{
  // compose data packet
  byte data[3];
  data[0] = B0100;     // write to control register
  data[1] = 0;         // 5% overrange disabled
  data[1] |= B00 << 1; // CLEAR selects zero scale
  data[2] = B000;      // output range = −10 V to +10 V
  data[2] |= B00 << 3; // power-up voltage = zero scale
  data[2] |= B0 << 5;  // disable thermal shutdown
  data[2] |= B0 << 6;  // DAC input for bipolar output range is straight binary coded

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
  data[0] = B11;
  data[1] = (val >> 8) & 0xff;
  data[2] = val & 0xff;

  // begin transaction
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE1));
  digitalWrite(CSB, LOW);

  // transfer data
  SPI.transfer(data, 3);

  // end transaction
  digitalWrite(CSB, HIGH);
  SPI.endTransaction();
}

void readback_control()
{
  // compose data packet
  byte data[3];
  data[0] = B1100;     // read from control register
  data[1] = 0;
  data[2] = 0;

  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(CSB, LOW);

  // transfer data
  for (int i=0; i<3; i++)
    SPI.transfer(data[i]);

  // end transaction
  digitalWrite(CSB, HIGH);
  SPI.endTransaction();

  // interpret the received data
  if ((data[1] >> 2) & 1)
    Serial.println("Brownout condition detected.");
  if ((data[1] >> 3) & 1)
    Serial.println("Short-circuit condition detected.");
}
