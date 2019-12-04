#include <SPI.h>

// for SPI communication with MAX31856 devices
const int CS0 = 16;
const int CS1 = 17;
const int DRDY0 = 22;
const int DRDY1 = 20;
const int FAULT0 = 23;
const int FAULT1 = 21;
const int SPI_speed = 5000*1000;
const int SPI_MODE = SPI_MODE3;

// for controlling the CPC40055ST relays
const int RELAY0 = 2;
const int RELAY1 = 3;

void setup() {
  Serial.begin(115200);
  SPI.begin();

  // initalize the data ready, chip select, fault, and relay pins
  pinMode(CS0, OUTPUT);
  pinMode(CS1, OUTPUT);
  pinMode(DRDY0, INPUT);
  pinMode(DRDY1, INPUT);
  pinMode(FAULT0, INPUT);
  pinMode(FAULT1, INPUT);
  pinMode(RELAY0, OUTPUT);
  pinMode(RELAY1, OUTPUT);

  // set the active-low chip-select pins to high
  digitalWrite(CS0, HIGH);
  digitalWrite(CS1, HIGH);

  // default relays to closed
  digitalWrite(RELAY0, LOW);
  digitalWrite(RELAY1, LOW);
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
        read_SPI(CS0, 0x0c, 3);
        break;
      case 'T':
        read_SPI(CS1, 0x0c, 3);
        break;

      // read cold junction temperature
      case 'j':
        read_SPI(CS0, 0x0a, 2);
        break;
      case 'J':
        read_SPI(CS1, 0x0a, 2);
        break;

      // read configuration registers
      case 'c':
        read_SPI(CS0, 0x00, 2);
        break;
      case 'C':
        read_SPI(CS1, 0x00, 2);
        break;

      // write configuration registers
      case 'w':
        write_SPI(CS0, 0x80, 2);
        break;
      case 'W':
        write_SPI(CS1, 0x80, 2);
        break;
        
      // check for fault conditions
      case 'f':
        fault_check();
        break;

      // relay on
      case 'r':
        digitalWrite(RELAY0, HIGH);
        break;
      case 'R':
        digitalWrite(RELAY1, HIGH);
        break;

      // relay off
      case 'q':
        digitalWrite(RELAY0, LOW);
        break;
      case 'Q':
        digitalWrite(RELAY1, LOW);
        break;
    }
  }
}

void fault_check()
{
  if (digitalRead(FAULT0) == HIGH)
    Serial.print("FAULT:0 ");
  else
    Serial.print("OK:0 ");
  if (digitalRead(FAULT1) == HIGH)
    Serial.print("FAULT:1");
  else
    Serial.print("OK:1");

  Serial.print("\n");
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

void read_SPI(int CS_pin, byte address, int num_reg)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(SPI_speed, MSBFIRST, SPI_MODE));
  digitalWrite(CS_pin, LOW);

  // command to read configuration registers
  SPI.transfer(address);

  // print the returned data to serial
  for (int i=0; i<num_reg; i++) {
    printBits(SPI.transfer(0x00));
    Serial.print(" ");
  }
  Serial.print("\n");

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}

void write_SPI(int CS_pin, byte address, int num_reg)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(SPI_speed, MSBFIRST, SPI_MODE3));
  digitalWrite(CS_pin, LOW);

  // command to read configuration registers
  SPI.transfer(address);

  // write serial data to SPI
  for (int i=0; i<num_reg; i++)
    SPI.transfer(Serial.read());

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}
