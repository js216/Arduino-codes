const int R[] {2, 3, 4, 5, 6};

void setup() {
  Serial.begin(9600);

  for (int i=0; i<5; i++)
    pinMode(R[i], OUTPUT);

  disable_all();
}

void disable_all()
{
  for (int i=0; i<5; i++)
    digitalWrite(R[i], LOW);
}

void enable_all()
{
  for (int i=0; i<5; i++)
    digitalWrite(R[i], HIGH);
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
      case '?':
        Serial.write("Degaussing relay board v1.3 ready.\n");
        break;
    }
  }
}
