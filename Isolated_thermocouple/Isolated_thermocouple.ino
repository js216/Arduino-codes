#include <SPI.h>

// pin configuration
const int CS_arr[] = {2,3,4,5,6,7,8,9};

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration for SPI
  for (int i=0; i<8; i++) {
    pinMode(CS_arr[i], OUTPUT);
    digitalWrite(CS_arr[i], HIGH);    
  }
}

byte data[4];

void serialEvent() {
  while (Serial.available()) {
    // read user input
    int c = (char)Serial.read();

    // parse input
    if      (c == '1') print_one(1);
    else if (c == '2') print_one(2);
    else if (c == '3') print_one(3);
    else if (c == '4') print_one(4);
    else if (c == '5') print_one(5);
    else if (c == '6') print_one(6);
    else if (c == '7') print_one(7);
    else if (c == '8') print_one(8);
    else if (c == 'a') print_all();
    else if (c == '?') Serial.println("Isolated thermocouple v2.0. Board No. 1.");
  }
}

void loop() {
}

void print_one(int n)
{
    // measure temperature
    read_SPI(CS_arr[n-1]);

    // return reading
    Serial.print(n, DEC);
    Serial.print(", ");
    decode_data();
    Serial.print('\n');
}

void print_all()
{
  for (int n=0; n<8; n++) {
    read_SPI(CS_arr[n]);
    decode_data();
    if (n<3)
      Serial.print(", ");
  }
  Serial.print('\n');
}

void decode_data()
{
  // 12-bit internal temperature data
  int sign = 1;
  if (data[2] & 0b10000000) {
    sign = -1;
    data[2] &= 0b01111111;
  }
  Serial.print(sign*((data[2]<<4) + (data[3]>>4))/16.0, 3);

  // separator
  Serial.write(", ");

  // check for error conditions
  if (data[3] & 0x01) {
    Serial.print("open circuit");
    return;
  } else if (data[3] & 0x02) {
    Serial.print("short to GND");
    return;
  } else if (data[3] & 0x04) {
    Serial.print("short to VCC");
    return;
  } else {
    // 14-bit thermocouple data
    sign = 1;
    if (data[0] & 0b10000000) {
      sign = -1;
      data[0] &= 0b01111111;
    }
    Serial.print(sign*((data[0]<<6) + (data[1]>>2))/4.0, 2);
  }
}

void read_SPI(const int CS_pin)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_pin, LOW);

  // read the data
  for (int i=0; i<4; i++)
    data[i] = SPI.transfer(0x00);

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
}
