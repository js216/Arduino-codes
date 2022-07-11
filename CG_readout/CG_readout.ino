#include <SPI.h>

// pin configuration
const int CSB = 2;

// internal state
byte data[4];
long result[4];
bool rdy[4];
bool enable_printing;
int gain = 0;

void setup() {
  Serial.begin(9600);

  SPI.begin();
  pinMode(CSB, OUTPUT);
  digitalWrite(CSB, HIGH);
  
  ADC_config(0,     // DOUT_RDY_DEL
             0,     // CONT_READ
             1,     // DATA_STATUS
             0,     // CS_EN
             1,     // REF_EN
             B11,   // POWER_MODE (11 = full power)
             B0000, // MODE (0000 = continuous conversion mode)
             B00);  // CLK_SEL (internal 614.4 kHz clock)

  setup_config(0,     // setup_id
               1,     // bipolar (1) or unipolar (0)
               0x00,  // burnout current source off
               0,     // no buffer enable on REFINx(+)
               0,     // no buffer enable on REFINx(-)
               0,     // no buffer enable on AINP
               0,     // no buffer enable on AINM
               B10,   // use internal reference
               gain); // gain (000 = gain 1, 111 = gain 128)

  filt_config(0,       // filter id
              0,       // filter type = sinc^4
              0,       // reject 60 Hz
              3,       // post filter
              1,       // SINGLE_CYCLE
              480);   // FS (from 1 = 19200 SPS to 2047 = 9.4 SPS)

  channel_config(1,       // enable
                 0,       // channel 0
                 0,       // setup 0
                 B00001,  // AINP
                 B00000); // AINM;

  channel_config(1,       // enable
                 1,       // channel 1
                 0,       // setup 0
                 B00011,  // AINP
                 B00010); // AINM;

  channel_config(1,       // enable
                 2,       // channel 2
                 0,       // setup 0
                 B00101,  // AINP
                 B00111); // AINM;
                 
  channel_config(1,       // enable
                 3,       // channel 3
                 0,       // setup 0
                 B00100,  // AINP
                 B00110); // AINM;
}

void loop() {
  read_ADC();

  if (enable_printing)
    print_ADC(1);

  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        // should read 0x04
        Serial.print("CG_readout v1.0 board ready (0x0");
        identify_ADC();
        Serial.println(").");
        break;
        
      case 'e':
        enable_printing = false;
        break;

      case 'E':
        enable_printing = true;
        break;
        
      case 'r':
        print_ADC(1);
        break;

      case 'R':
        print_ADC(0);
        break;
              
      case 'f':
        filt_config(
          0,   // filter id
          0,   // filter type = sinc^4
          0,   // don't reject 60 Hz
          3,   // post filter (doesn't matter, not enabled)
          0,   // SINGLE_CYCLE = false
          Serial.parseInt()); // FS (from 1 to 2047)
        break;

      case 'g':
        gain = Serial.parseInt();
        setup_config(0,     // setup_id
                     1,     // bipolar (1) or unipolar (0)
                     0x00,  // burnout current source off
                     0,     // no buffer enable on REFINx(+)
                     0,     // no buffer enable on REFINx(-)
                     0,     // no buffer enable on AINP
                     0,     // no buffer enable on AINM
                     B10,   // use internal reference
                     gain); // gain (0 = gain 1, 7 = gain 128)
        break;

      case 'w':
      int ch = Serial.parseInt();
      int val = Serial.parseInt();
        write_DAC(ch, val);
    }
  }
}

/***********************************************************
 * DATA FUNCTIONS
 ***********************************************************/

void read_ADC()
{
  // read the status register
  write_SPI(0x40, // read
            0x00, // status register
            1);   // length in bytes

  // check conversion result is ready
  if (data[0] & B10000000)
    return;

  // read the data register
  write_SPI(0x40, // read
            0x02, // data register
            4);   // length in bytes
  
  // check conversion result is still ready
  if (data[3] & B10000000)
    return;

  // verify valid channel received
  int ch = data[3] & B00001111;
  if (!((ch==0) || (ch==1) || (ch==2) || (ch==3)))
    return;

  // decode and store received value
  result[ch] = (((long)data[0] << 16) | ((long)data[1] << 8) | (long)data[2]);
  rdy[ch] = true;
}

float ADC_to_V(const int ADC_val)
{
  return ADC_val;
  return 2.5 * ((ADC_val/pow(2,23)) - 1);
}

void print_ADC(bool print_voltage)
{
  // check data is ready
  for (int i=0; i<4; i++)
    if (rdy[i] != true)
      return;

  // convert ADC value to voltage
  if (print_voltage) {
    Serial.print(ADC_to_V(result[0]));
    Serial.print(',');
    Serial.print(ADC_to_V(result[1]));
    Serial.print(',');
    Serial.print(ADC_to_V(result[2]));
    Serial.print(',');
    Serial.println(ADC_to_V(result[3]));
  } else {
    Serial.print(result[0]);
    Serial.print(',');
    Serial.print(result[1]);
    Serial.print(',');
    Serial.print(result[2]);
    Serial.print(',');
    Serial.println(result[3]);
  }

  // clear "data ready" flags
  for (int i=0; i<4; i++)
    rdy[i] = false;
}

/***********************************************************
 * CONFIGURATION FUNCTIONS
 ***********************************************************/

void identify_ADC()
{
  write_SPI(0x40, // read
            0x05, // ID register
            1);   // length = 1 byte

  // should print 0x04 (for AD7124-4)
  Serial.print(data[0], HEX);
}

void channel_config(const byte enable, const unsigned int ch, const byte setup_id, const byte AINP, const byte AINM)
{
  // compose data packet
  data[0] = (enable << 7) | (setup_id << 4) | (AINP >> 3);
  data[1] = (AINP << 5) | AINM;
  
  write_SPI(0x00,    // write
            0x09+ch, // channel config register
            2);      // length = 2 bytes
}

void setup_config(const unsigned int setup_id, const byte bipolar, const byte burnout, const byte REF_BUFP, const byte REF_BUFM,
                  const byte AIN_BUFP, const byte AIN_BUFM, const byte REF_SEL, const byte PGA)
{
  // compose data packet
  data[0] = (bipolar << 3) | (burnout << 1) | REF_BUFP;
  data[1] = (REF_BUFM << 7) | (AIN_BUFP << 6) | (AIN_BUFM << 5) | (REF_SEL << 3) | PGA;
  
  write_SPI(0x00,          // write
            0x19+setup_id, // setup config register
            2);            // length = 2 bytes
}

void filt_config(const unsigned int filt_id, const byte filter_type, const byte REJ60, const byte POST_FILTER, const byte SINGLE_CYCLE, int FS)
{
  // compose data packet
  data[0] = (REJ60 << 4) | (POST_FILTER << 1) | SINGLE_CYCLE;
  data[1] = FS >> 8;
  data[3] = FS;
  
  write_SPI(0x00,          // write
            0x21+filt_id, // setup config register
            3);            // length = 3 bytes
}

void ADC_config(const byte DOUT_RDY_DEL, const byte CONT_READ, const byte DATA_STATUS, const byte CS_EN, const byte REF_EN,
                const byte POWER_MODE, const byte MODE, const byte CLK_SEL)
{
  // compose data packet
  data[0] = (DOUT_RDY_DEL << 4) | (CONT_READ << 3) | (DATA_STATUS << 2) | (CS_EN << 1) | REF_EN;
  data[1] = (POWER_MODE << 6) | (MODE << 2) | CLK_SEL;
  
  write_SPI(0x00, // write
            0x01, // setup ADC control register
            2);   // length = 2 bytes
}

/***********************************************************
 * LOW-LEVEL FUNCTIONS
 ***********************************************************/

void write_SPI(const byte operation, const byte reg, const unsigned int len)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(CSB, LOW);

  // send read/write command
  SPI.transfer(operation | reg);

  // send or receive data
  SPI.transfer(data, len);

  // end transaction
  digitalWrite(CSB, HIGH);
  SPI.endTransaction();
}

void write_DAC(const int ch, const int val)
{
  // compose data packet for AD5664
  data[0] = (B011 << 3) | ch;
  data[1] = val >> 8;
  data[2] = val;

  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(CSB, LOW);
  delay(10);
  digitalWrite(CSB, HIGH);

  // send data
  SPI.transfer(data, 3);

  // end transaction
  SPI.endTransaction();
}
