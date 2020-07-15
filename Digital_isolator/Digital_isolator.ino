// pin configuration
const int in_C = 2;
const int in_D = 3;
const int enable_C = 4;
const int out_C = 5;
const int enable_D = 6;
const int out_D = 7;

void setup() {
  Serial.begin(9600);

  // pin configuration
  pinMode(in_C, INPUT);
  pinMode(in_D, INPUT);
  pinMode(enable_C, INPUT);
  pinMode(enable_D, INPUT);
  pinMode(out_C, INPUT);
  pinMode(out_D, INPUT);
}

void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.println("Digital isolator board v1.1 ready.");
        break;

      case 'e':
        enable(enable_C, out_C);
        break;

      case 'E':
        enable(enable_D, out_D);
        break;

      case 'd':
        disable(enable_C);
        break;

      case 'D':
        disable(enable_D);
        break;

      case 'w':
        write_pin(enable_C, out_C);
        break;

      case 'W':
        write_pin(enable_D, out_D);
        break;

      case 'c':
        clear_pin(enable_C, out_C);
        break;

      case 'C':
        clear_pin(enable_D, out_D);
        break;
    }
  }
}

void enable(const int en_pin, const int out_pin)
{
  // make sure we're not writing to the pin before enabling external control
  pinMode(out_pin, INPUT);

  // enable external control
  pinMode(en_pin, OUTPUT);
  digitalWrite(en_pin, HIGH);
}

void disable(const int en_pin)
{
  // disable external control
  pinMode(en_pin, OUTPUT);
  digitalWrite(en_pin, LOW);
}

void write_pin(const int en_pin, const int out_pin)
{
  // make sure external control is disabled before writing to the pin
  disable(en_pin);

  // write to the pin
  pinMode(out_pin, OUTPUT);
  digitalWrite(out_pin, HIGH);
}

void clear_pin(const int en_pin, const int out_pin)
{
  // make sure external control is disabled before writing to the pin
  disable(en_pin);

  // write to the pin
  pinMode(out_pin, OUTPUT);
  digitalWrite(out_pin, LOW);
}

void loop() {
}
