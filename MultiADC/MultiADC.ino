 #include <SPI.h>

// pin configuration
const int CS[] = {2, 4, 6, 3, 5, 7};

// internal state
byte data[4];
long result[6][4];
long offs[6][4];
bool rdy[6][4];
bool enable_printing;
int gain = 0;

void setup() {
  Serial.begin(9600);
  
  SPI.begin();
  
  for (int CS_i=0; CS_i<6; CS_i++) {
    pinMode(CS[CS_i], OUTPUT);
    digitalWrite(CS[CS_i], LOW);

    ADC_config(CS_i,
               0,      // DOUT_RDY_DEL
               0,      // CONT_READ
               1,      // DATA_STATUS
               0,      // CS_EN
               1,      // REF_EN
               0b11,   // POWER_MODE (11 = full power)
               0b0000, // MODE (0000 = continuous conversion mode)
               0b00);  // CLK_SEL (internal 614.4 kHz clock)
               
    setup_config(CS_i,
                 0,      // setup_id
                 1,      // bipolar (1) or unipolar (0)
                 0x00,   // burnout current source off
                 0,      // no buffer enable on REFINx(+)
                 0,      // no buffer enable on REFINx(-)
                 0,      // no buffer enable on AINP
                 0,      // no buffer enable on AINM
                 0b10,   // use internal reference
                 gain);  // gain (000 = gain 1, 111 = gain 128)
  
    filt_config(CS_i,
                0,       // filter id
                0,       // filter type = sinc^4
                0,       // reject 60 Hz
                3,       // post filter
                1,       // SINGLE_CYCLE
                1);      // FS (from 1 to 2047)
  
    channel_config(CS_i,
                   1,        // enable
                   0,        // channel 0
                   0,        // setup 0
                   0b00000,  // AINP
                   0b00001); // AINM;
  
    channel_config(CS_i,
                   1,        // enable
                   1,        // channel 1
                   0,        // setup 0
                   0b00010,  // AINP
                   0b00011); // AINM;
  
    channel_config(CS_i,
                   1,        // enable
                   2,        // channel 2
                   0,        // setup 0
                   0b00100,  // AINP
                   0b00101); // AINM;
                   
    channel_config(CS_i,
                   1,        // enable
                   3,        // channel 3
                   0,        // setup 0
                   0b00110,  // AINP
                   0b00111); // AINM;
                   
  }
}

void loop() {
  for (int CS_i=0; CS_i<6; CS_i++)
    read_ADC(CS_i);
    
  if (enable_printing)
    print_ADC(0);

  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.print("MultiADC v1.0 ready, board B (");
        for (int CS_i=0; CS_i<6; CS_i++) {
          Serial.print("0x0");
          identify_ADC(CS_i);
          Serial.print(", ");
        }
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

      case 'o':
        for (int CS_i=0; CS_i<6; CS_i++)
          for (int i=0; i<15; i++)
            offs[CS_i][i] = result[CS_i][i];
        break;
              
      case 'f':
        for (int CS_i=0; CS_i<6; CS_i++)
          filt_config(
            CS_i,
            0,   // filter id
            0,   // filter type = sinc^4
            0,   // don't reject 60 Hz
            3,   // post filter (doesn't matter, not enabled)
            0,   // SINGLE_CYCLE = false
            Serial.parseInt()); // FS (from 1 to 2047)
        break;

      case 'g':
        gain = Serial.parseInt();
        for (int CS_i=0; CS_i<6; CS_i++)
          setup_config(CS_i,
             0,      // setup_id
             1,      // bipolar (1) or unipolar (0)
             0x00,   // burnout current source off
             0,      // no buffer enable on REFINx(+)
             0,      // no buffer enable on REFINx(-)
             0,      // no buffer enable on AINP
             0,      // no buffer enable on AINM
             0b10,   // use internal reference
             gain);  // gain (0 = gain 1, 7 = gain 128)
        break;

    }
  }
}

/***********************************************************
 * DATA FUNCTIONS
 ***********************************************************/

void read_ADC(const int CS_i)
{
  // read the status register
  write_SPI(CS_i,
            0x40, // read
            0x00, // status register
            1);   // length in bytes

  // check conversion result is ready
  if (data[0] & 0b10000000)
    return;

  // read the data register
  write_SPI(CS_i,
            0x40, // read
            0x02, // data register
            4);   // length in bytes
  
  // check conversion result is still ready
  if (data[3] & 0b10000000)
    return;

  // verify valid channel received
  int ch = data[3] & 0b00001111;
  if (!((ch==0) || (ch==1) || (ch==2) || (ch==3)))
    return;

  // decode and store received value
  result[CS_i][ch] = (((long)data[0] << 16) | ((long)data[1] << 8) | (long)data[2]);
  rdy[CS_i][ch] = true;
}

void print_ADC(bool print_voltage)
{
  // check data is ready
  for (int CS_i=0; CS_i<6; CS_i++)
    for (int i=0; i<4; i++)
      if (rdy[CS_i][i] != true)
        return;
  
  // print out the data
  for (int CS_i=0; CS_i<6; CS_i++) {
    // convert ADC value to voltage
    if (print_voltage) {
    float ADC_to_V = 5 / (pow(2,24) * pow(2,gain));
      Serial.print(ADC_to_V * result[CS_i][0]);
      Serial.print(',');
      Serial.print(ADC_to_V * result[CS_i][1]);
      Serial.print(',');
      Serial.print(ADC_to_V * result[CS_i][2]);
      Serial.print(',');
      Serial.print(ADC_to_V * result[CS_i][3]);
    } else {
      Serial.print(result[CS_i][0]-offs[CS_i][0]);
      Serial.print(',');
      Serial.print(result[CS_i][1]-offs[CS_i][1]);
      Serial.print(',');
      Serial.print(result[CS_i][2]-offs[CS_i][2]);
      Serial.print(',');
      Serial.print(result[CS_i][3]-offs[CS_i][3]);
    }
  
    // print newline or comma
    if (CS_i == 5)
      Serial.print('\n');
    else
    Serial.print(',');
  
    // clear "data ready" flags
    for (int i=0; i<4; i++)
      rdy[CS_i][i] = false;
  }
}

/***********************************************************
 * CONFIGURATION FUNCTIONS
 ***********************************************************/

void identify_ADC(const int CS_i)
{
  write_SPI(CS_i,
            0x40, // read
            0x05, // ID register
            1);   // length = 1 byte

  // should print 0x04 (for AD7124-4)
  Serial.print(data[0], HEX);
}

void channel_config(const int CS_i, const byte enable, const unsigned int ch, const byte setup_id, const byte AINP, const byte AINM)
{
  // compose data packet
  data[0] = (enable << 7) | (setup_id << 4) | (AINP >> 3);
  data[1] = (AINP << 5) | AINM;
  
  write_SPI(CS_i,
            0x00,    // write
            0x09+ch, // channel config register
            2);      // length = 2 bytes
}

void setup_config(const int CS_i, const unsigned int setup_id, const byte bipolar, const byte burnout, const byte REF_BUFP, const byte REF_BUFM,
                  const byte AIN_BUFP, const byte AIN_BUFM, const byte REF_SEL, const byte PGA)
{
  // compose data packet
  data[0] = (bipolar << 3) | (burnout << 1) | REF_BUFP;
  data[1] = (REF_BUFM << 7) | (AIN_BUFP << 6) | (AIN_BUFM << 5) | (REF_SEL << 3) | PGA;
  
  write_SPI(CS_i,
            0x00,          // write
            0x19+setup_id, // setup config register
            2);            // length = 2 bytes
}

void filt_config(const int CS_i, const unsigned int filt_id, const byte filter_type, const byte REJ60, const byte POST_FILTER, const byte SINGLE_CYCLE, int FS)
{
  // compose data packet
  data[0] = (REJ60 << 4) | (POST_FILTER << 1) | SINGLE_CYCLE;
  data[1] = FS >> 8;
  data[3] = FS;
  
  write_SPI(CS_i,
            0x00,          // write
            0x21+filt_id, // setup config register
            3);            // length = 3 bytes
}

void ADC_config(const int CS_i, const byte DOUT_RDY_DEL, const byte CONT_READ, const byte DATA_STATUS, const byte CS_EN, const byte REF_EN,
                const byte POWER_MODE, const byte MODE, const byte CLK_SEL)
{
  // compose data packet
  data[0] = (DOUT_RDY_DEL << 4) | (CONT_READ << 3) | (DATA_STATUS << 2) | (CS_EN << 1) | REF_EN;
  data[1] = (POWER_MODE << 6) | (MODE << 2) | CLK_SEL;
  
  write_SPI(CS_i,
            0x00, // write
            0x01, // setup ADC control register
            2);   // length = 2 bytes
}

/***********************************************************
 * LOW-LEVEL FUNCTIONS
 ***********************************************************/

void write_SPI(const int CS_i, const byte operation, const byte reg, const unsigned int len)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(CS[CS_i], HIGH);
  delayMicroseconds(5); // needed because SI8380P-IUR has a 4us propagation delay

  // send read/write command
  SPI.transfer(operation | reg);

  // send or receive data
  SPI.transfer(data, len);

  // end transaction
  digitalWrite(CS[CS_i], LOW);
  SPI.endTransaction();
}
