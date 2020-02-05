#include <SPI.h>

// pin mappings
const int DAC_RST = 9;
const int LDAC = 10;
const int CS = 11;
const int trig_in = 6;
const int trig_out = 7;

// for SPI data transfer
byte result[3];
byte data[3];

void setup() {
  // pin modes
  pinMode(CS, OUTPUT);
  pinMode(DAC_RST, OUTPUT);
  pinMode(LDAC, OUTPUT);
  pinMode(trig_out, OUTPUT);
  pinMode(trig_in, INPUT);

  // default pin values
  digitalWrite(CS, HIGH);
  digitalWrite(LDAC, HIGH);
  digitalWrite(DAC_RST, HIGH);

  // comm libraries
  Serial.begin(9600);
  SPI.begin();
}

void move_bits()
{
  // begin transaction
  digitalWrite(CS, LOW);
  SPI.beginTransaction(SPISettings(1000, MSBFIRST, SPI_MODE2));

  // transfer data
  for (int i=0; i<3; i++)
    result[i] = SPI.transfer(data[i]);

  // end transaction
  SPI.endTransaction();
  digitalWrite(CS, HIGH);
}

void write_DAC(unsigned int x)
{
  data[0] = 0x30 | ((x >> 12) & 0xff);
  data[1] = (x >> 4) & 0xff;
  data[2] = (x << 4)  & 0xff;
  move_bits();
  
  Serial.print("Should have written: ");
  for (int i=0; i<3; i++) {
    printBits(data[i]);
    Serial.print(' ');
  }
  Serial.print('\n');
}

void read_DAC()
{
  // send "read input register" command
  data[0] = 0x50;
  move_bits();

  // print result over serial
  for (int i=0; i<3; i++) {
    printBits(result[i]);
    Serial.print(' ');
  }
  Serial.print('\n');
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case 'w':
        write_DAC(Serial.parseInt());
        break;
      case 'r':
        read_DAC();
        break;
    }
  }
}

void loop() {
}

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}
