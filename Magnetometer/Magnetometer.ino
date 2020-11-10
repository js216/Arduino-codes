#include <SPI.h>

// pin configuration
const int CSB = 10;

// internal state
byte data[4];
long result[2];
bool rdy[2];
bool enable_printing;

void setup() {
  Serial.begin(115200);

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
               B000); // gain (000 = gain 1, 111 = gain 128)

  filt_config(0,       // filter id
              0,       // filter type = sinc^4
              0,       // reject 60 Hz
              3,       // post filter
              1,       // SINGLE_CYCLE
              384);    // FS

  channel_config(1,       // enable
                 0,       // channel 0
                 0,       // setup 0
                 B00100,  // AINP
                 B00101); // AINM;

  channel_config(1,       // enable
                 1,       // channel 1
                 0,       // setup 0
                 B00011,  // AINP
                 B00010); // AINM;
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.println("Magnetometer v3.0 board ready.");
        break;
        
      case 'e':
        enable_printing = false;
        break;

      case 'E':
        enable_printing = true;
        break;
        
      case 'r':
        print_ADC();
        break;
              
      case 'f':
        filt_config(
          0,   // filter id
          0,   // filter type = sinc^4
          0,   // don't reject 60 Hz
          3,   // post filter (doesn't matter, not enabled)
          0,   // SINGLE_CYCLE = false
          Serial.parseInt()); // FS
        break;

      case 'g':
        setup_config(0,     // setup_id
                     1,     // bipolar (1) or unipolar (0)
                     0x00,  // burnout current source off
                     0,     // no buffer enable on REFINx(+)
                     0,     // no buffer enable on REFINx(-)
                     0,     // no buffer enable on AINP
                     0,     // no buffer enable on AINM
                     B10,   // use internal reference
                     Serial.parseInt()); // gain (0 = gain 1, 7 = gain 128)
        break;
    }
  }
}

void loop() {
  read_ADC();

  if (enable_printing)
    print_ADC();
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
  if (!((ch==0) || (ch==1)))
    return;

  // decode and store received value
  result[ch] = (((long)data[0] << 16) | ((long)data[1] << 8) | (long)data[2]);
  rdy[ch] = true;
}

void print_ADC()
{
  // check data is ready
  for (int i=0; i<2; i++)
    if (rdy[i] != true)
      return;
  
  // print out the data
  Serial.print(result[0]);
  Serial.print(',');
  Serial.println(result[1]);

  // clear "data ready" flags
  for (int i=0; i<2; i++)
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
