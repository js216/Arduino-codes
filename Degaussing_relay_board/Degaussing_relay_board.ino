const int relay_delay = 10;

const int R1=2, R2=3, R3=4, R4=5, R5=6;

void setup() {
  Serial.begin(9600);

  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);
  pinMode(R4, OUTPUT);
  pinMode(R5, OUTPUT);

  clear_all();
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '0':
        clear_all();
        break;
      case '1':
        clear_all();
        delay(relay_delay);
        digitalWrite(R5, HIGH);
        break;
      case '2':
        clear_all();
        delay(relay_delay);
        digitalWrite(R5, HIGH);
        break;
      case '3':
        clear_all();
        delay(relay_delay);
        digitalWrite(R5, HIGH);
        break;
      case '4':
        clear_all();
        delay(relay_delay);
        digitalWrite(R5, HIGH);
        break;
      case '5':
        clear_all();
        delay(relay_delay);
        digitalWrite(R5, HIGH);
        break;
      case '?':
        Serial.write("Degaussing relay board v1.3 ready.\n");
        break;
    }
  }
}

void clear_all()
{
  digitalWrite(R1, LOW);
  digitalWrite(R2, LOW);
  digitalWrite(R3, LOW);
  digitalWrite(R4, LOW);
  digitalWrite(R5, LOW);
}

void loop() {
}
