#include <SPI.h>

// pin configuration
const int SEL[] = {A1, 4, 5}; // bsel, rsel0, rsel1
const int ERR[] = {6, 7, 10}; // x, y, z
const int OR[] = {A0, 8, 9}; // x, y, z
const int CS_pin=3, SYNC=2;

// for data transfer
byte data[3];

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration
  pinMode(CS_pin, OUTPUT);
  digitalWrite(CS_pin, HIGH);
  pinMode(SYNC, INPUT);
  for (int i=0; i<3; i++) {
    pinMode(OR[i], INPUT);
    pinMode(ERR[i], INPUT);
    pinMode(SEL[i], OUTPUT);
  }

  // DRV425 low bandwidth
  digitalWrite(SEL[0], LOW);

  // DRV425 select V_ref = 1.65 V
  digitalWrite(SEL[1], HIGH);
  digitalWrite(SEL[2], LOW);
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case 'r':
        printout_magnetometer();
        break;
        
      case '?':
        Serial.write("Magnetometer v2.2 board ready.\n");
        print_ID();
        break;
    }
  }
}

void printout_magnetometer()
{
  // for x, y, z
  for (int i=0; i<3; i++) {
    // check for error
    if (digitalRead(ERR[i]) == HIGH)
      Serial.write("ERR");

    // check for overrange
    else if (digitalRead(OR[i]) == HIGH)
      Serial.write("OR");

    // read magnetic field
    else
      Serial.write(get_B(i));

    // separator
    if (i < 2)
      Serial.write(", ");
    else
      Serial.write('\n');
  }
}

int get_B(int axis)
{
  
}

void print_ID()
{
  transfer_SPI(0x05, 1, 1);
  Serial.println(data[0], HEX);
}

void loop() {
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

void transfer_SPI(const unsigned int address, const int num_reg, const int read_bit)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(CS_pin, LOW);

  // command to access (either read or write) a given register
  SPI.transfer((read_bit << 6) | address);

  // read the returned data
  for (int i=0; i<num_reg; i++)
    data[i] = SPI.transfer(data[i]);

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}
