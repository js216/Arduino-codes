#include <SPI.h>

// pin configuration
const int CSB = 10;

// for SPI communication
byte data[3];

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration
  pinMode(CSB, OUTPUT);
  digitalWrite(CSB, LOW);
  
  // configure the ADC
  ADC_config(0,     // DOUT_RDY_DEL
             0,     // CONT_READ
             0,     // DATA_STATUS
             0,     // CS_EN
             1,     // REF_EN
             B11,   // POWER_MODE (11 = full power)
             B0000, // MODE (0000 = continuous conversion mode)
             B00);  // CLK_SEL (internal 614.4 kHz clock)

  // configure one of the 8 available setups
  setup_config(0,     // setup_id
               0,     // bipolar (1) or unipolar (0)
               0x00,  // burnout current source off
               0,     // no buffer enable on REFINx(+)
               0,     // no buffer enable on REFINx(-)
               0,     // no buffer enable on AINP
               0,     // no buffer enable on AINM
               B10,   // use internal reference
               B111);  // gain (00 = gain 1)

  // enable one of the 16 ADC channels
  channel_config(1,       // enable
                 0,       // channel 0
                 0,       // setup 0
                 B00000,  // AINP
                 B00001); // AINM;
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.println("AD7124 test board ready.");
        break;
    }
  }
}

void loop() {
  Serial.println(read_mV(), 10);
  delay(100);
}

/***********************************************************
 * DATA FUNCTIONS
 ***********************************************************/

float read_mV()
{
  write_SPI(0x40, // read
            0x02, // data register
            3);   // length = 3 bytes

  long val = (((long)data[0] << 16) | ((long)data[1] << 8) | (long)data[2]);
  return (float)val/16777.216*2.50;
}

/***********************************************************
 * CONFIGURATION FUNCTIONS
 ***********************************************************/

void identify_ADC()
{
  write_SPI(0x40, // read
            0x05, // ID register
            1);   // length = 1 byte

  Serial.println(data[0], HEX);
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
