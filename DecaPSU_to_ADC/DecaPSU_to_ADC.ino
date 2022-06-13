#include <SPI.h>

// pin configuration
const int sens_to_ch[] = {1, 2, 3, 13, 14, 0, 10, 12, 11, 8, 7, 9, 4, 6, 5};
const int ch_to_sens[] = {5, 0, 1, 2, 12, 14, 13, 10, 9, 11, 6, 8, 7, 3, 4};
const int CS_ADC    = 10;
const int MISO_pin  = 12;
const int MISO_pin2 = 9;
const int MOSI_pin  = 11;
const int SCK_pin   = 13;

// state variables
byte ADC_data[5];
int readings[15];
bool readings_rdy[15];
bool print_enable[15];
int offs[15];
int dt = -1;
elapsedMillis last_printed, t0;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  SPI.setMOSI(MISO_pin);
  SPI.setMISO(MOSI_pin);
  SPI.setSCK(SCK_pin);

  // pin configuration
  pinMode(CS_ADC, OUTPUT);
  pinMode(MISO_pin2, INPUT);
  digitalWrite(CS_ADC, HIGH);

  disable_all_sensors();
  set_ADC_setup();
  set_ADC_mode();
  set_ADC_filter(100);
  set_ADC_interface();
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.write("DecaPSU to ADC (");
        print_ADC_ID();
        Serial.write(") v2.0 ready.\n");
        break;

      case 'd':
        disable_all_sensors();
        break;

      case 's':
        set_ADC_ch(sens_to_ch[Serial.parseInt()], Serial.parseInt());
        break;

      case 'o':
        for (int i=0; i<15; i++)
          offs[i] = readings[i];
        break;

      case 'f':
        set_ADC_filter(Serial.parseInt());
        break;

      case 'p':
        dt = Serial.parseInt();
        break;
    }
  }
}

void loop() {
  readout_ADC();

  if (dt >= 0) {
    if (last_printed >= dt) {
      last_printed -= dt;

      // check all required values are ready
      for (int i=0; i<15; i++)
        if ((print_enable[i]) && (!readings_rdy[i]))
          return;
      
      // printout the values
      Serial.print(t0);
      for (int i=0; i<15; i++)
        if (print_enable[i]) {
          Serial.write(", ");
          Serial.print(readings[i] - offs[i]);
          readings_rdy[i] = false;
        }
      Serial.write("\n");
    }
  } else {
    t0 = 0;
    last_printed = 0;
  }
}


/////////////////////////////////
/// ADC FUNTIONS ///////////////
/////////////////////////////////

void print_ADC_ID()
{
  // read from the ID register
  ADC_data[0] = (1 << 6) + 0x07;
  
  write_ADC(3);

  Serial.print("0x");
  Serial.print((ADC_data[1]<<4) + (ADC_data[2]>>4), HEX);
  Serial.print('X');
}

void print_ADC_register(byte reg, int len)
{
  ADC_data[0] = (1 << 6) + reg;
  
  write_ADC(len);

  for (int i=0; i<len-1; i++) {
    Serial.print(ADC_data[i], BIN);
    Serial.print(", ");
  }
  Serial.println(ADC_data[len-1], BIN);
}

void set_ADC_ch(unsigned int ch, bool ch_enable)
{
  // write to the channel register ch
  ADC_data[0] = 0x10 + ch;

  // enable/disable selected channel
  ADC_data[1] = ch_enable << 7;

  // connect the right input pair to this channel
  ADC_data[1] += ch >> 3;
  ADC_data[2] = 0x10 + (ch << 5);

  write_ADC(3);

  // enable/disable printing of the channel data
  print_enable[ch_to_sens[ch]] = ch_enable;
}

void disable_all_sensors()
{
  // disable all sensors
  for (int i=0; i<15; i++)
    set_ADC_ch(i, 0);
}

void set_ADC_setup()
{
  // write to the setup register 0
  ADC_data[0] = 0x20;

  // set output coding to bipolar
  ADC_data[1] = 1 << 4;

  // enable the input buffer
  ADC_data[1] += 3;

  // use internal reference
  ADC_data[2] = 2 << 4;

  write_ADC(3);
}

void set_ADC_filter(const int ODR)
{
  // write to the filter register 0
  ADC_data[0] = 0x28;

  // set output data rate
  ADC_data[1] = 0;
  if (ODR == 125000)
    ADC_data[2] = B00000;
  else if (ODR == 62500)
    ADC_data[2] = B00010;
  else if (ODR == 31250)
    ADC_data[2] = B00100;
  else if (ODR == 25000)
    ADC_data[2] = B00101;
  else if (ODR == 15625)
    ADC_data[2] = B00110;
  else if (ODR == 10390)
    ADC_data[2] = B00111;
  else if (ODR == 4994)
    ADC_data[2] = B01000;
  else if (ODR == 2498)
    ADC_data[2] = B01001;
  else if (ODR == 1000)
    ADC_data[2] = B01010;
  else if (ODR == 500)
    ADC_data[2] = B01011;
  else if (ODR == 395)
    ADC_data[2] = B01100;
  else if (ODR == 200)
    ADC_data[2] = B01101;
  else if (ODR == 100)
    ADC_data[2] = B01110;
  else if (ODR == 60)
    ADC_data[2] = B01111;
  else if (ODR == 50)
    ADC_data[2] = B10000;
  else if (ODR == 20)
    ADC_data[2] = B10001;
  else if (ODR == 16)
    ADC_data[2] = B10010;
  else if (ODR == 10)
    ADC_data[2] = B10011;
  else if (ODR == 5)
    ADC_data[2] = B10100;
  else if (ODR == 2)
    ADC_data[2] = B10101;

  write_ADC(3);
}

void set_ADC_mode()
{
  // write to the ADC mode register
  ADC_data[0] = 0x01;

  // enable internal reference
  ADC_data[1] = 1 << 7;
  ADC_data[2] = 0;

  write_ADC(3);
}

void set_ADC_interface()
{
  // write to the ADC interface mode register
  ADC_data[0] = 0x02;

  // enable DATA_STAT
  ADC_data[1] = 0;
  ADC_data[2] = 1 << 6;

  write_ADC(3);
}

void readout_ADC()
{
  // check new data is ready
  digitalWrite(CS_ADC, LOW);
  delayMicroseconds(1);
  if (digitalRead(MISO_pin2))
    return;
  digitalWrite(CS_ADC, HIGH);
  
  // read from the data register
  ADC_data[0] = (1 << 6) + 0x04;
  write_ADC(5);

  // determine the channel from the status register
  const unsigned int ch = ADC_data[4] & 15;

  // combine three bytes into one variable
  const int val_i = (ADC_data[1] << 16) + (ADC_data[2] << 8) + ADC_data[3];

  // store into array for later use
  readings[ch_to_sens[ch]] = val_i - pow(2,23); // 25/pow(2,23) volts

  // indicate data is now ready
  readings_rdy[ch_to_sens[ch]] = true;
}

void write_ADC(unsigned int len)
{
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_ADC, LOW);
  SPI.transfer(ADC_data, len);
  digitalWrite(CS_ADC, HIGH);
  SPI.endTransaction();
}
