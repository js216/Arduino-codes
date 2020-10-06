#include <SPI.h>
#include <avr/io.h>

///////////////////////////////////////////////
////    PIN ASSIGNMENT FOR AD9910   ///////////
///////////////////////////////////////////////

// SPI pins
const int CS_pin = 36;
const int IO_update_pin = 38;

// reset pins
const int reset_pin = 22;
const int powerdown_pin = 21;

// profiles pins
const int profile_pins[] = {14, 15, 16};
 
// parallel port pins
const int D_pins[] = {30,29,28,27,26,25,24,10,9,8,7,6,5,4,3,2};
const int F_pins[] = {32, 31};
const int PD = 18;
const int TEN = 17;

// digital ramp control pins
const int DRCTL = 33;
const int DRHOLD = 35;
const int DROVER = 34;

// other AD9910 pins
const int RSO = 19; // RAM_SWP_OVR
const int OSK = 37;
const int PLL = 20;
const int SYC = 39;

// external input pin
const int inputPin = A9;

// PID parameters
float Kp=0, Ki=0, setpoint=500;
long int accumulator=0;

///////////////////////////////////////////////
////    OTHER VARIABLES             ///////////
///////////////////////////////////////////////

// length of AD9910 internal registers
const int reg_len[] = {4,4,4,4,4,6,6,4,2,4,4,8,8,4,8,8,8,8,8,8,8,8,4};

// for reading SPI data into
char result[8];

// configuring the ramp
volatile bool digital_ramp_enable = false;
int ramp_time_step_us = 100;
int ramp_delay_ms = 10;

///////////////////////////////////////////////
////    DEVICE INITIALIZATION       ///////////
///////////////////////////////////////////////

void setup()
{
  // for AD9910 communication
  SPI.begin();

  // for computer communication
  Serial.begin(9600);

  // CS is active-low
  pinMode(CS_pin, OUTPUT);
  digitalWrite(CS_pin, LOW);

  pinMode(IO_update_pin, OUTPUT);
  digitalWrite(IO_update_pin, LOW);

  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, LOW);

  pinMode(powerdown_pin, OUTPUT);
  digitalWrite(powerdown_pin, LOW);

  pinMode(TEN, OUTPUT);
  digitalWrite(TEN, LOW);

  pinMode(PLL, INPUT);

  pinMode(inputPin, INPUT);

  for (int i=0; i<2; i++) {
    pinMode(F_pins[i], OUTPUT);
    digitalWrite(F_pins[i], LOW);
  }

  for (int i=0; i<16; i++) {
    pinMode(D_pins[i], OUTPUT);
    digitalWrite(D_pins[i], LOW);
  }

  for (int i=0; i<3; i++)
    pinMode(profile_pins[i], OUTPUT);

  pinMode(DRCTL, OUTPUT);
  pinMode(DRHOLD, OUTPUT);
  pinMode(DROVER, INPUT);

  // AD9910 defaults
  set_PLL(true);
  set_profile(0, 85000000, 1000000000, 100);
  enable_parallel_port(1);
  write_parallel(0, 16);
}

///////////////////////////////////////////////
////    SERIAL COMMUNICATION        ///////////
///////////////////////////////////////////////

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      
      // REGISTER-LEVEL AD9910 FUNCTIONS
      
      case '1':
        read_one_register( Serial.parseInt() );
        break;

      case '0':
        digitalWrite(reset_pin, HIGH);
        delay(100);
        digitalWrite(reset_pin, LOW);

        set_profile(0, 85000000, 1000000000, 100);
        set_PLL(true);
        Serial.print("Device reset complete.\n");
        break;

      case 'w':
        {
          int reg = Serial.parseInt();
          char data[reg_len[reg]];
          for (int i=0; i<reg_len[reg]; i++)
             data[i] = Serial.read();
          write_register(reg, data);
        }
        break;

      // HIGH-LEVEL AD9910 FUNCTIONS

      case 'a':
        activate_profile(Serial.parseInt());
        break;
      
      case 'p':
        set_profile(Serial.parseInt(), Serial.parseInt(), Serial.parseInt(), Serial.parseInt());
        break;

      case 'o':
        activate_profile(0);
        set_profile(0, Serial.parseInt()*1000000, 1000000000, 100);
        break;

      case 'P':
        enable_parallel_port(Serial.parseInt());
        break;

      case 'm':
        parallel_mode(Serial.parseInt());
        break;

      case 'A':
        enable_ASF(Serial.parseInt());
        break;

      case 'r':
        enable_refclk_divider(Serial.parseInt());
        break; 

      case 'N':
        set_PLL(Serial.parseInt());
        break;

      case 'n':
        if (read_PLL())
          Serial.println("PLL active.");
        else
          Serial.println("PLL not active.");
        break;

      // PID FUNCTIONS

      case 'd':
        write_parallel(Serial.parseInt(), 16);
        break;

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

      case '?':
        Serial.println(analogRead(inputPin));
        break;
    }
  }
}

///////////////////////////////////////////////
////    MAIN DEVICE & PID LOOP      ///////////
///////////////////////////////////////////////

void loop()
{
  // calculate error
  const int error = analogRead(inputPin) - setpoint;
  accumulator += error;

  // calculate output
  int dds_out = Kp*error + Ki*accumulator;

  // check output is positive; else make it zero
  if (dds_out < 0)
    dds_out = 0;

  // write to parallel port
  write_parallel(dds_out, 16);
}

///////////////////////////////////////////////
////    AD9910 SPI COMMUNICATION    ///////////
///////////////////////////////////////////////

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
  digitalWrite(CS_pin, HIGH);
  delay(5);
  digitalWrite(CS_pin, LOW);
  
  SPI.transfer(reg | B10000000);

  // read data
  for (int i=0; i<reg_len[reg]; i++)
    result[i] = SPI.transfer(0x00);

  // end transaction
  SPI.endTransaction();
}

void read_one_register(int reg)
{
  // Read out the contents of the register
  Serial.print("Reading from register ");
  Serial.print(reg, HEX);
  Serial.print(":   \n  ");
  read_register(reg);
  for (int i=0; i<reg_len[reg]; i++) {
    printBits(result[i]);
    Serial.print(",");
  }
  Serial.print("\n");
}

void write_register(int reg, char data[])
{
  // begin transaction
  SPI.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0));

  // flash the IO_RESET (as well as the CS) to clear potential unfinished transactions
  digitalWrite(CS_pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(CS_pin, LOW);

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

///////////////////////////////////////////////
////    AD9910 CONFIGURATION        ///////////
///////////////////////////////////////////////

void activate_profile(const int profile)
{
  for (int i=0; i<3; i++) {
    if ((profile>>i) & 1)
      digitalWrite(profile_pins[i], HIGH);
    else
      digitalWrite(profile_pins[i], LOW);
  }
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

  // flip the "Parallel data port enable" bit
  if (ENABLE == true) {
    digitalWrite(TEN, HIGH);
    data[3] |= B00010000;
  } else {
    digitalWrite(TEN, LOW);
    data[3] &= B11101111;
  }

  // write the new register values
  write_register(0x01, data);
}

void enable_ASF(bool ENABLE)
{
  // read register 0x01
  char data[reg_len[0x01]];
  read_register(0x01);
  for (int i=0; i<8; i++)
    data[i] = result[i];

  // flip the "Enable amplitude scale from single ton profiles" bit
  if (ENABLE == true) {
    data[0] |= B00000001;
  } else {
    data[0] &= B11111110;
  }

  // write the new register values
  write_register(0x01, data);
}

void enable_refclk_divider(bool ENABLE)
{
  // read register 0x02
  char data[reg_len[0x02]];
  read_register(0x02);
  for (int i=0; i<8; i++)
    data[i] = result[i];

  // flip the "REFCLK input divider bypass" bit
  if (ENABLE == true) {
    data[2] &= B01111111;
  } else {
    data[2] |= B10000000;
  }

  // write the new register values
  write_register(0x02, data);
}

void parallel_mode(int mode)
{
  if (mode == 0) { // amplitude modulation
    digitalWrite(F_pins[1], LOW);
    digitalWrite(F_pins[0], LOW);
  } else if (mode == 1) { // phase modulation
    digitalWrite(F_pins[1], LOW);
    digitalWrite(F_pins[0], HIGH);
  } else if (mode == 2) { // frequency modulation
    digitalWrite(F_pins[1], HIGH);
    digitalWrite(F_pins[0], LOW);
  } else if (mode == 3) { // amplitude & phase modulation
    digitalWrite(F_pins[1], HIGH);
    digitalWrite(F_pins[0], HIGH);
  } else
    Serial.println("ERROR: Invalid mode.");
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

bool read_PLL()
{
  if (digitalRead(PLL))
    return true;
  else
    return false;
}

///////////////////////////////////////////////
////    CONVENIENCE FUNCTIONS       ///////////
///////////////////////////////////////////////

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}

char demandSerialByte(const int N = 1000)
{
  for (int i=0; i<N; ++i) {
    if (Serial.available() > 0)
      return Serial.read();
    else
      delay(1);
  }

  return 0;
}
