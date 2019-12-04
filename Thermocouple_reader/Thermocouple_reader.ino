#include <SPI.h>

// for SPI communication with LTC2983CLX#PBF devices
const int CS_pin = 9;
const int RESET = 8;
const int INTERRUPT = 24;
const int SPI_speed = 1000*1000;
const int SPI_MODE = SPI_MODE0;
unsigned char T0[] = {0x00, 0x00, 0x00, 0x00};

void setup() {
  Serial.begin(115200);
  SPI.begin();

  // initalize the data ready, chip select, fault, and relay pins
  pinMode(CS_pin, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(INTERRUPT, INPUT);

  // set the active-low pins to high
  digitalWrite(CS_pin, HIGH);
  digitalWrite(RESET, HIGH);

  // configure channels
  sensor_config();

  // set ideality factor
  ideality_factor(1010);
}

void loop() {
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      // read thermocouple temperature
      case 't':
        read_temp(Serial.parseInt());
        break;

      case 'i':
        ideality_factor(Serial.parseFloat());
        break;
    }
  }
}

/* ============================
 * THERMOMETER FUNCTIONS
   ============================ */

void ideality_factor(float eta)
{
  // configure CH8 for diode, single-ended, 3 conversion cycles, average enabled, low excitation current
  unsigned char CH8_config[4] = {0b11100111, 0b00000000, 0b000000000, 0b00000000};

  // convert eta to binary (see LTC2983 datasheet, page 24)
  long eta_long = (eta * 1048576) / 1000;
  CH8_config[3] |= (char)eta_long;
  CH8_config[2] |= (char)(eta_long >> 8);
  CH8_config[1] |= (char)(eta_long >> 16);

  // write the register
  write_SPI(0x21c, 4, CH8_config);
}

void sensor_config()
{
  // wait for instrument to be ready
  while (!digitalRead(INTERRUPT))
    delay(10);

  // set units to celsius
  const unsigned char unit_config[] = {0b00000000};
  write_SPI(0x0f0, 1, unit_config);
    
  // configure all channels for K-type thermocouple, single-ended, OC check high current, cold-junction compensation from CH8
  const unsigned char sensor_config[] = {0b00010010, 0b00111100, 0b00000000, 0b00000000};
  for (int ch=0; ch<20; ch++)
    if (ch != 7)
      write_SPI(0x200+4*ch, 4, sensor_config);
}

void read_temp(const int ch)
{
  // request temperature measurement on CH1
  unsigned char ch_b = (char)ch;
  unsigned char sensor_request[] = {0b10000000 + ch_b};
  write_SPI(0x00, 1, sensor_request);

  // wait for measurement to finish
  while (!digitalRead(INTERRUPT))
    delay(10);
  
  // read out the value
  read_SPI(0x10+4*(ch-1), 4, false);

  // convert binary value to float
  long signed T = T0[3] + (T0[2]<<8) + (T0[1]<<16);
  if (T0[1] && 0b10000000) // sign extend negative numbers
    T |= (long)0xff << 24;
  Serial.println((float)T/1024);
}

/* ============================
 * AUXILIARY FUNCTIONS
   ============================ */

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}

void read_SPI(const unsigned int address, const int num_reg, const bool printing)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(SPI_speed, MSBFIRST, SPI_MODE));
  digitalWrite(CS_pin, LOW);

  // command to read configuration registers
  SPI.transfer(0x03);

  // 16-bit register address
  SPI.transfer16(address);

  // read the returned data
  for (int i=0; i<num_reg; i++)
    T0[i] = SPI.transfer(0x00);

  // print out the returned value to serial
  if (printing) {
    for (int i=0; i<num_reg; i++) {
      printBits(T0[i]);
      Serial.print(" ");
    }
    Serial.print("\n");
  }

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}

void write_SPI(const unsigned int address, const int num_reg, const unsigned char data[])
{
  // begin transaction
  SPI.beginTransaction(SPISettings(SPI_speed, MSBFIRST, SPI_MODE));
  digitalWrite(CS_pin, LOW);

  // command to write to configuration registers
  SPI.transfer(0x02);

  // 16-bit register address
  SPI.transfer16(address);

  // write serial data to SPI
  for (int i=0; i<num_reg; i++) {
    SPI.transfer(data[i]);
    delay(1);
  }

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}
