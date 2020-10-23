#include <SPI.h>

// ADS8681 pins
const int CS_ADC = 38;

// AD9910 pins
const int CS_DDS = 10;
const int IO_update_pin = 34;
const int reset_pin = 22;
const int powerdown_pin = 21;
const int profile_pins[] = {40, 41};
const int D_pins[] = {30, 29, 28, 25, 24, 9, 8, 7, 6, 5, 4, 15, 16, 17, 3, 2};
const int F_pins[] = {32, 31};
const int TEN = 36;
const int PLL = 20;

// length of AD9910 internal registers
const int reg_len[] = {4,4,4,4,4,6,6,4,2,4,4,8,8,4,8,8,8,8,8,8,8,8,4};

// for reading SPI data into
char result[8];

// PID parameters
int setpoint=40000, loop_delay=0;
float Kp=0, Ki=0;
int error, dds_out, accumulator;

void setup() {
  Serial.begin(115200);

  // SPI for talking to the ADC
  pinMode(CS_ADC, OUTPUT);
  digitalWrite(CS_ADC, HIGH);
  SPI1.setMISO(39);
  SPI1.begin();

  // SPI for talking to the DDS
  pinMode(CS_DDS, OUTPUT);
  digitalWrite(CS_DDS, HIGH);
  SPI.begin();

  // other DDS pins
  pinMode(IO_update_pin, OUTPUT);
  digitalWrite(IO_update_pin, LOW);

  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, LOW);

  pinMode(powerdown_pin, OUTPUT);
  digitalWrite(powerdown_pin, LOW);

  pinMode(TEN, OUTPUT);
  digitalWrite(TEN, LOW);

  pinMode(PLL, INPUT);

  for (int i=0; i<2; i++) {
    pinMode(F_pins[i], OUTPUT);
    digitalWrite(F_pins[i], LOW);
  }

  for (int i=0; i<16; i++) {
    pinMode(D_pins[i], OUTPUT);
    digitalWrite(D_pins[i], LOW);
  }

  for (int i=0; i<2; i++)
    pinMode(profile_pins[i], OUTPUT);

  // AD9910 defaults
  reset_DDS();
}

void loop() {
//  // calculate error
//  error = read_ADC() - setpoint;
//  accumulator += error;
//
//  // calculate output
//  dds_out = Kp*error + Ki*accumulator;
//
//  // check output is positive; else make it zero
//  if (dds_out < 0)
//    dds_out = 0;
//
//  // write to parallel port
//  write_parallel(dds_out);

  // loop delay
  delayMicroseconds(loop_delay);
  write_parallel(Kp*read_ADC());
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      case '?':
        Serial.print("Kp=");
        Serial.print(Kp);
        Serial.print(", Ki=");
        Serial.print(Ki);
        Serial.print(", SP=");
        Serial.print(setpoint);
        Serial.print(", loop_delay=");
        Serial.println(loop_delay);
        break;

      // ADC commands

      case 'r':
        Serial.println(read_ADC());
        break;

      case 'R':
        set_ADC_range(Serial.parseInt());
        break;

      // DDS commands
      
      case '0':
        reset_DDS();
        break;

      case 'n':
        if (read_PLL())
          Serial.println("PLL active.");
        else
          Serial.println("PLL not active.");
        break;

      case 'N':
        set_PLL(Serial.parseInt());
        break;

      case 'P':
        enable_parallel_port(Serial.parseInt());
        break;

      case 'd':
        write_parallel(Serial.parseInt());
        break;

      // PID commands

      case 'k':
        accumulator = 0;
        Kp = Serial.parseFloat();
        break;

      case 'K':
        accumulator = 0;
        Ki = Serial.parseFloat();
        break;

      case 's':
        accumulator = 0;
        setpoint = Serial.parseFloat();
        break;
        
      case 'l':
        loop_delay = Serial.parseInt();
        break;
    }
  }
}

/* ============================
 * DDS FUNCTIONS
   ============================ */

void reset_DDS()
{
  digitalWrite(reset_pin, HIGH);
  delay(100);
  digitalWrite(reset_pin, LOW);

  set_profile(0, 85000000, 1000000000, 100);
  set_PLL(true);
  enable_parallel_port(1);
}

void write_parallel(int n)
{
  // disable parallel port
  digitalWrite(TEN, LOW);

  // write data to the parallel pins
  for (int i=0; i<16; i++) {
    if ((n>>i)&1)
      digitalWrite(D_pins[i], HIGH);
    else
      digitalWrite(D_pins[i], LOW);
  }

  // enable parallel port
  digitalWrite(TEN, HIGH);
}

void read_register(int reg)
{
  // register sanity check
  if (reg >= 23) {
    Serial.print("Invalid register: ");
    Serial.print(reg, HEX);
    Serial.print("\n");
    return;
  }
  
  // begin transaction
  SPI.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0));

  // flash the IO_RESET (as well as the CS) to clear potential unfinished transactions
  digitalWrite(CS_DDS, HIGH);
  delay(5);
  digitalWrite(CS_DDS, LOW);
  
  SPI.transfer(reg | B10000000);

  // read data
  for (int i=0; i<reg_len[reg]; i++)
    result[i] = SPI.transfer(0x00);

  // end transaction
  SPI.endTransaction();
}

void write_register(int reg, char data[])
{
  // begin transaction
  SPI.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0));

  // flash the IO_RESET (as well as the CS) to clear potential unfinished transactions
  digitalWrite(CS_DDS, HIGH);
  delayMicroseconds(5);
  digitalWrite(CS_DDS, LOW);

  // transfer data
  SPI.transfer(reg & B01111111);
  for (int i=0; i<reg_len[reg]; i++) {
    // do the SPI transfer
    result[i] = SPI.transfer(data[i]);
    
    // transfer data from IO buffers to internal registers
    digitalWrite(IO_update_pin, HIGH);
    delayMicroseconds(5);
    digitalWrite(IO_update_pin, LOW);
  }

  // end transaction
  SPI.endTransaction();
}

bool read_PLL()
{
  if (digitalRead(PLL))
    return true;
  else
    return false;
}

void set_profile(const int profile, const int Fout, const int Fsysclk, const int ASF_percent)
{
  // turn off PLL
  set_PLL(false);
  
  // find the profile register
  const int profile_reg = profile + 14;
  
  // calculate the frequency tuning word
  int FTW = (pow(2,32) * Fout) / Fsysclk;
  unsigned char FTW_bytes[4];
  for (int i=0; i<4; i++)  
    FTW_bytes[i] = (FTW >> (3-i)*8) & 0xFF;

  // calculate amplitude scale factor
  int ASF = int(ASF_percent * (pow(2,14)-1) / 100);
  unsigned char ASF_bytes[4];
  for (int i=0; i<2; i++)
    ASF_bytes[i] = (ASF >> (1-i)*8) & 0xFF;

  // write to the profile register
  char data[reg_len[profile_reg]];
  for (int i=0; i<2; i++)
    data[i] = ASF_bytes[i];
  for (int i=2; i<4; i++)
    data[i] = 0;
  for (int i=4; i<8; i++)
    data[i] = FTW_bytes[i-4];
  write_register(profile_reg, data);

  // turn PLL back on
  set_PLL(true);
}

void enable_parallel_port(bool ENABLE)
{
  // read register 0x01
  char data[reg_len[0x01]];
  read_register(0x01);
  for (int i=0; i<8; i++)
    data[i] = result[i];

  // flip the "Parallel data port enable" and "Data assembler hold last value" bits
  if (ENABLE == true) {
    digitalWrite(TEN, HIGH);
    data[3] |= B01010000;
  } else {
    digitalWrite(TEN, LOW);
    data[3] &= B10101111;
  }

  // write the new register values
  write_register(0x01, data);
}

void set_PLL(bool ENABLE)
{
  // read register 0x02
  char data[reg_len[0x02]];
  read_register(0x02);
  for (int i=0; i<reg_len[0x02]; i++)
    data[i] = result[i];

  if (ENABLE) {
    // select VCO_SEL = VCO5 (for operation at 920-1030 MHz)
    data[0] &= B11111000;
    data[0] |= B00000101;

    // write N=25 value (for 40MHz * 25 = 1 GHz clock)
    data[3] = 25 << 1;

    // set the "PLL enable" bit
    data[2] |= B00000001;
    
  } else {
    data[2] &= B11111110; // clear the "PLL enable" bit
    data[0] |= B00000111; // set VCO SEL bits to "PLL bypass"
    data[3] = 0;          // set N = 0
  }

  // write the new register values
  write_register(0x02, data);
}


/* ============================
 * ADC FUNCTIONS
   ============================ */

unsigned short read_ADC()
{
  SPI1.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_ADC, LOW);
  const unsigned short data = SPI1.transfer16(0);
  digitalWrite(CS_ADC, HIGH);
  SPI1.endTransaction();
  return data;
}

void set_ADC_range(byte range)
{
  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_ADC, LOW);
  SPI1.transfer16(0xD014);
  SPI1.transfer16(range);
  digitalWrite(CS_ADC, HIGH);
  SPI1.endTransaction();
}
