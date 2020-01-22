// pin definitions
int PinA = 3;
int PinB = 4;

// state variables
int pos = 0;
int PinALast = LOW;
int n = LOW;

void setup() {
  pinMode (PinA, INPUT_PULLUP);
  pinMode (PinB, INPUT_PULLUP);
  Serial.begin (9600);
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();
    switch (cmd) {
      case '?':
        Serial.println(pos, DEC);
        break;
      case '0':
        pos = 0;
        break;
    }
  }
}

void loop() {
  n = digitalRead(PinA);
  if ((PinALast == LOW) && (n == HIGH)) {
    if (digitalRead(PinB) == LOW)
      pos--;
    else
      pos++;
  }
  PinALast = n;
}
