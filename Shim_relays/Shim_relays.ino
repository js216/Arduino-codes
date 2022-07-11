int ch;
//               1   2   3   4   5   6   7   8   9  10 11 12 13 14 15 16
const int R[] = {A6, A7, A4, A5, A2, A3, A0, A1, 3, 2, 5, 4, 7, 6, 9, 8};

void setup() {
  Serial.begin(9600);
//  while (!Serial) {} // wait for serial port to connect

  for (int i=0; i<16; i++)
    pinMode(R[i], OUTPUT);

  disable_all();
}

void disable_all()
{
  for (int i=0; i<16; i++) {
    digitalWrite(R[i], LOW);
  }
}

void enable_all()
{
  for (int i=0; i<16; i++) {
    digitalWrite(R[i], HIGH);
  }
}

void loop() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '0':
        disable_all();
        break;
      case '1':
        enable_all();
        break;
      case 'e':
        digitalWrite(R[Serial.parseInt()-1], HIGH);
        break;
      case 'd':
        digitalWrite(R[Serial.parseInt()-1], LOW);
        break;
      case 't':
        test_relays();
        break;
      case '?':
        Serial.print("Shim_relays v1.0 ready. Board C.\n");
        break;
    } 
  }
}

void test_relays()
{
  delay(500);
  for (int i=0; i<16; i++) {
    digitalWrite(R[i], HIGH);
    delay(100);
    digitalWrite(R[i], LOW);
    delay(100);
  }
}
