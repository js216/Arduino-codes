#include <SPI.h>

// pin configuration
const int CS_arr[] = {2, 3, 4, 10};

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // pin configuration for SPI
  for (int i=0; i<4; i++) {
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
    int n;
    if (c == '1') n = 1;
    else if (c == '2') n = 2;
    else if (c == '3') n = 3;
    else if (c == '4') n = 4;
    else if (c == '?') {
      Serial.println("Board ready.");
      return;
    }
    else {
//      Serial.println("Unknown command.");
      return;
    }

    // measure temperature
    read_SPI(CS_arr[n-1]);

//    // print out bits for debugging
//    for (int i=0; i<4; i++)
//      Serial.println(data[i], BIN);

    // return reading
    Serial.print(n, DEC);
    Serial.print(", ");
    decode_data();
  }
}

void loop() {
}


void read_SPI(const int CS_pin)
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_pin, LOW);

  // read the data
  for (int i=0; i<4; i++)
    data[i] = SPI.transfer(0x00);

  // end transaction
  digitalWrite(CS_pin, HIGH);
  SPI.endTransaction();
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
    Serial.println("open circuit");
    return;
  } else if (data[3] & 0x02) {
    Serial.println("short to GND");
    return;
  } else if (data[3] & 0x04) {
    Serial.println("short to VCC");
    return;
  } else {
    // 14-bit thermocouple data
    sign = 1;
    if (data[0] & 0b10000000) {
      sign = -1;
      data[0] &= 0b01111111;
    }
    Serial.println(sign*((data[0]<<6) + (data[1]>>2))/4.0, 2);
  }
}
