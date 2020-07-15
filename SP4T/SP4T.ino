// pin configuration
const int TRIG_in = 1;
const int EN_pin = 7;
const int A0_pin = 8;
const int A1_pin = 9;

void setup() {
  Serial.begin(9600);

  // pin configuration
  pinMode(TRIG_in, INPUT);
  pinMode(EN_pin, OUTPUT);
  pinMode(A0_pin, OUTPUT);
  pinMode(A1_pin, OUTPUT);
  digitalWrite(EN_pin, LOW);
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.println("SP4T BNC switch v1.1 ready.");
        break;

      case 'e':
        digitalWrite(EN_pin, HIGH);
        break;

      case 'd':
        digitalWrite(EN_pin, LOW);
        break;

      case '1':
        digitalWrite(EN_pin, LOW);
        digitalWrite(A0_pin, LOW);
        digitalWrite(A1_pin, LOW);
        digitalWrite(EN_pin, HIGH);
        break;

      case '2':
        digitalWrite(EN_pin, LOW);
        digitalWrite(A0_pin, HIGH);
        digitalWrite(A1_pin, LOW);
        digitalWrite(EN_pin, HIGH);
        break;

      case '3':
        digitalWrite(EN_pin, LOW);
        digitalWrite(A0_pin, LOW);
        digitalWrite(A1_pin, HIGH);
        digitalWrite(EN_pin, HIGH);
        break;

      case '4':
        digitalWrite(EN_pin, LOW);
        digitalWrite(A0_pin, HIGH);
        digitalWrite(A1_pin, HIGH);
        digitalWrite(EN_pin, HIGH);
        break;
    }
  }
}

void loop() {
}
