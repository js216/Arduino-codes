#define IMXRT_GPIO6_DIRECT  (*(volatile uint32_t *)0x42000000)

const int D_pins[] = {19, 18, 14, 15, 40, 41, 17, 16, 22, 23, 20, 21, 38, 39, 26, 27};

void setup() {
  Serial.begin(9600);

  for (int i=0; i<16; i++) {
    pinMode(D_pins[i], OUTPUT);
    digitalWrite(D_pins[i], LOW);
  }
}

void loop() {
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      case '?':
        Serial.println(IMXRT_GPIO6_DIRECT, BIN);
        break;

      case 'w':
        IMXRT_GPIO6_DIRECT = Serial.parseInt() << 16;
        break;

      case 'v':
        for (uint32_t i=0; i<=65535; i++) {
          IMXRT_GPIO6_DIRECT = i << 16;
          uint32_t num = (IMXRT_GPIO6_DIRECT >> 16);
//          Serial.println(num);
          if (num != i) {
            Serial.print(i);
            Serial.print(" != ");
            Serial.println(num);
            break;
          }
        }
        break;
    }
  }
}
