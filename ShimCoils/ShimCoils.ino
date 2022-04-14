#include <SPI.h>

// pin configuration
const int CS_DAC = 10;

// for SPI communication
unsigned char DAC_data[3];

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration
  pinMode(CS_DAC, OUTPUT);
  digitalWrite(CS_DAC, HIGH);
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.write("Shim coils v1.3a, board B, ready.\n");
        break;

      case 's':
      {
        const unsigned int ch = Serial.parseInt();
        const unsigned int val = Serial.parseInt();
        set_DAC(ch, val);
        break;
      }
    }
  }
}

void loop() {
}

/////////////////////////////////
/// DAC FUNTIONS ///////////////
/////////////////////////////////


void set_DAC(unsigned int ch, const unsigned int val)
{
  // decide which DAC to address
  int invert_CS;
  if (ch < 8)
    invert_CS = 1;
  else {
    invert_CS = 0;
    ch -= 8;
  }
  
  // write and update DAC register ch
  DAC_data[0] = (B0011 << 4) + ch;
  DAC_data[1] = (val >> 8);
  DAC_data[2] = val;
  DAC_SPI(invert_CS);
}

void DAC_SPI(int invert_CS)
{
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE2));

  if (invert_CS)
    digitalWrite(CS_DAC, HIGH);
  else
    digitalWrite(CS_DAC, LOW);
    
  SPI.transfer(DAC_data, 3);

  if (invert_CS)
    digitalWrite(CS_DAC, LOW);
  else
    digitalWrite(CS_DAC, HIGH);

  SPI.endTransaction();
}
