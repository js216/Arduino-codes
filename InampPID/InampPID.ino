#include <SPI.h>
#include <Encoder.h>

// pin configuration
const int CSB = 10;
const int enc_A = 2;
const int enc_B = 3;
const int button = 4;
const int LED[] = {0, 5, 6, 7, 8};

// for rotary encoder and its button
Encoder Enc(enc_B, enc_A);
const long inc[] = {0, 500, 50, 5, 1};
int btn_cur=HIGH, btn_prev=HIGH;
long btn_t = 0;

// state variables
int state = 0;
long st_val[] = {0, 0, 0, 0, 0};
long setpoint = 0;

// ADC readout
int ADC_ch_val[] = {0, 0};

// for SPI communication
byte data[3];

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // set pin modes and default outputs
  pinMode(CSB, OUTPUT);
  digitalWrite(CSB, HIGH);
  pinMode(button, INPUT);
  digitalWrite(button, HIGH);
  for (int i=1; i<=4; i++)
    pinMode(LED[i], OUTPUT);

  // DAC configuration
  DAC_gain(HIGH);

  // ADC configuration
  ADC_config(1, 0);           // enable internal reference
  ADC_channels(1, 1, 0, 0);   // select channels 0 and 1
  ADC_auto_mode();
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.println("InampPID v2.3 board ready");
        break;

      // DAC FUNCTIONS

      case 'w':
        setpoint = Serial.parseInt();
        Serial.println(setpoint);
        DAC_write(setpoint);
        break;

      case 'g':
        DAC_gain(Serial.parseInt());
        break;

      case 'R':
        for (long i=0; i<65535; i++) {
          if (i % 10 == 0) {
            DAC_write(i);
          }
        }
        Serial.println("Ramp done.");
        break;

      // ADC FUNCTIONS
        
      case 'r':
        print_ADC();
        break;

      case 'c':
        ADC_config(1, 0);           // enable internal reference
        ADC_channels(1, 1, 0, 0);   // select all channels
        ADC_auto_mode();
        break;
    }
  }
}

void loop() {
  // check for button press
  btn_cur = digitalRead(button);
  if (btn_cur != btn_prev) {
    if (btn_cur == LOW) {
      digitalWrite(LED[state], LOW);
      state = (state + 1) % 5;
      digitalWrite(LED[state], HIGH);
    }
    btn_prev = btn_cur;
    delay(50);
  }

  // check if encoder changed
  if (state && Enc.read())
    increment_setpoint();
}

/************************************
 * ENCODER FUNCTIONS
 ***********************************/

void increment_setpoint()
{
  // prevent encoder state from changing
  noInterrupts();

  // read and zero the encoder
  int enc_val = Enc.read();
  Enc.write(0);

  // increment the relevant decade
  st_val[state] += enc_val/abs(enc_val);

  // calculate new setpoint
  long SP_new = 0;
  for (int i=1; i<=4; i++)
    SP_new += inc[i] * st_val[i];

  // verify new setpoint is in range [0, 65535]
  if ((SP_new >= 0) && (SP_new <= 65535))
    setpoint = SP_new;
  else
    st_val[state] -= enc_val/abs(enc_val);

  // write new setpoint to the DA
  DAC_write(setpoint);

  // resume encoder operation
  interrupts();
}
  
/************************************
 * DAC FUNCTIONS
 ***********************************/

void DAC_SPI()
{
  // begin transaction
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(CSB, HIGH);
  digitalWrite(CSB, LOW);

  // transfer data
  SPI.transfer(data, 3);

  // end transaction
  SPI.endTransaction();
  digitalWrite(CSB, HIGH);
}

void DAC_write(long val)
{
  // argument sanity check
  if (val < 0)
    val = 0;
    
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
 * ADC FUNCTIONS
 ***********************************/

void ADC_SPI()
{
  // begin transaction
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE3));
  digitalWrite(CSB, HIGH);

  // transfer data
  SPI.transfer(data, 2);

  // end transaction
  SPI.endTransaction();
  digitalWrite(CSB, LOW);
}

void ADC_config(const bool int_Vref_enable, const bool temp_enable)
{
  data[0] = 0x06 << 1;
  data[1] = (int_Vref_enable << 2) | (temp_enable << 1);
  ADC_SPI();
}

void ADC_channels(const bool ch0_en, const bool ch1_en,
                  const bool ch2_en, const bool ch3_en)
{
  data[0] = 0x0C << 1;
  data[1] = (ch3_en << 1) | (ch2_en << 3) | (ch1_en << 5) | (ch0_en << 7);
  ADC_SPI();
}

void ADC_range(const int ch, const int range)
{
  /* Possible range values (ADS8634 datasheet, page 56):
   * 0 = Reserved; do not use this setting
   * 1 = Range is set to ±10V
   * 2 = Range is set to ±5V
   * 3 = Range is set to ±2.5V
   * 4 = Reserved; do not use this setting
   * 5 = Range is set to 0V to 10V
   * 6 = Range is set to 0V to 5V
   * 7 = Reserved; do not use this setting
   */
  data[0] = (0x10 + ch) << 1;
  data[1] = range << 4;
  ADC_SPI();
}

void ADC_auto_mode()
{
  data[0] = 0x05 << 1;
  data[1] = 0;
  ADC_SPI();
}

void read_ADC()
{
  // get data from ADC
  data[0] = 0;
  data[1] = 0;
  ADC_SPI();

  // interpret it
  int ch = data[0] >> 5;
  int val = data[1] | ((data[0] & B00001111) << 8);

  // sanity check channel id
  if ((ch > 1) || (ch < 0))
    return;

  // store values
  ADC_ch_val[ch] = val;
}

void print_ADC()
{
  // get data from ADC
  for (int i=0; i<2; i++)
    read_ADC();

  // print data to serial
  Serial.print(setpoint);
  Serial.print(",");
  Serial.print(ADC_ch_val[0]);
  Serial.print(",");
  Serial.println(ADC_ch_val[1]);
}
