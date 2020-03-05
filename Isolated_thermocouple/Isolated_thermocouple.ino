// pin configuration
const int CS_arr[] = {2, 3, A1, 10};
const int sck = 13;
const int miso = 12;

// for data transfer
byte byte0=0x00, byte1=0x00;

void setup() {
  Serial.begin(9600);

  // pin ocnfiguration for SPI
  pinMode(miso, INPUT);
  pinMode(sck, OUTPUT);
  for (int i=0; i<4; i++) {
    pinMode(CS_arr[i], HIGH);
    digitalWrite(CS_arr[i], HIGH);    
  }
}

byte state = 0x00;
byte state2 = 0x00;

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case 't':
        digitalWrite(CS_arr[1], state2);
        state2 ^= 0xff;
        break;
      case '?':
        Serial.write("Isolated thermocouple board ready.\n");
        break;
    }
  }
}

void loop() {
  digitalWrite(sck, state);
  state ^= 0xff;
  delay(200);
}


void read_SPI(const int CS_pin)
{
}
