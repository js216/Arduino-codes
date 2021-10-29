#include <SPI.h>

// pin configuration
const int CS_ADC    = 10;
const int MISO_pin  = 12;
const int MOSI_pin  = 11;
const int SCK_pin   = 13;

// state variables
byte ADC_data[5];

void setup() {
  Serial.begin(9600);
  SPI.begin();
  SPI.setMOSI(MISO_pin);
  SPI.setMISO(MOSI_pin);
  SPI.setSCK(SCK_pin);

  // pin configuration
  pinMode(CS_ADC, OUTPUT);
  digitalWrite(CS_ADC, HIGH);

  configure_ADC();
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.write("DecaPSU to ADC v2.0 ready.\n");
        break;

      case 's':
        set_ADC_ch(Serial.parseInt(), Serial.parseInt());
        break;

      case 'r':
        print_ADC_data();
        break;
    }
  }
}

void loop() {
}


/////////////////////////////////
/// ADC FUNTIONS ///////////////
/////////////////////////////////

void write_ADC(unsigned int len)
{
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_ADC, LOW);
  SPI.transfer(ADC_data, len);
  digitalWrite(CS_ADC, HIGH);
  SPI.endTransaction();
}

void print_ADC_ID()
{
  // read from the ID register
  ADC_data[0] = (1 << 6) + 0x07;
  
  write_ADC(3);

  Serial.print("0x");
  Serial.print((ADC_data[1]<<4) + (ADC_data[2]>>4), HEX);
  Serial.println('X');
}

void print_ADC_register(byte reg, unsigned int len)
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

void set_ADC_filter()
{
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

void configure_ADC()
{
  // disable all channels (seems ch0 is active by default nonetheless)
  for (int i=0; i<16; i++)
    set_ADC_ch(i,0);
  
  set_ADC_setup();
  set_ADC_filter();
  set_ADC_mode();
  set_ADC_interface();
}

void print_ADC_data()
{
  // read from the data register
  ADC_data[0] = (1 << 6) + 0x04;
  write_ADC(5);

  // determine the channel
  unsigned int ch = ADC_data[4] & 15;

  // combine three bytes into one variable
  int val_i = (ADC_data[1] << 16) + (ADC_data[2] << 8) + ADC_data[3];

  // convert to voltage
  float val_f = 25 * (val_i/pow(2,23) - 1);

  Serial.print(ch);
  Serial.print(" : ");
  Serial.println(val_f);
}
