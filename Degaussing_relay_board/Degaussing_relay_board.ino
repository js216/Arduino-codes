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

void enable_relay(char r)
{
  if (r == '1')
    digitalWrite(R[0], HIGH);
  else if (r == '2')
    digitalWrite(R[1], HIGH);
  else if (r == '3')
    digitalWrite(R[2], HIGH);
  else if (r == '4')
    digitalWrite(R[3], HIGH);
  else if (r == '5')
    digitalWrite(R[4], HIGH);
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
      case '?':
        Serial.write("Degaussing relay board v1.3 ready.\n");
        break;
      default:
        enable_relay(c);
        break;
    }
  }
}
