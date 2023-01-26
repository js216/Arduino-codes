 #include <SPI.h>

// pin configuration
const int CS_ADC = 3;
const int CS_DAC = 4;
const int relay_ctl = 2;
const int polarity_control = 5;
const int HV_enable = 6;

// internal state
byte data[3];
int next_ch=0, prev_ch=0;
long values[5];
unsigned long previousMillis = 0;

// constants
const int update_interval = 200;
const int num_ch = 5;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  pinMode(CS_ADC, OUTPUT);
  digitalWrite(CS_ADC, HIGH);
  pinMode(CS_DAC, OUTPUT);
  digitalWrite(CS_DAC, HIGH);
  pinMode(polarity_control, OUTPUT);
  digitalWrite(polarity_control, LOW);
  pinMode(HV_enable, OUTPUT);
  digitalWrite(HV_enable, LOW);
  pinMode(relay_ctl, OUTPUT);
  digitalWrite(relay_ctl, LOW);
}

void loop() {
  check_serial();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= update_interval) {
    previousMillis = currentMillis;
    values[prev_ch] = read_ADC(next_ch);
    prev_ch = next_ch;
    next_ch = (next_ch + 1) % num_ch;

    if ((next_ch)%num_ch == 0)
      printout_ADC();
  }
}

void check_serial()
{
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.println("HV_control v1.1 ready.");
        break;
        
      case 'p':
        digitalWrite(polarity_control, HIGH);
        break;

     case 'P':
        digitalWrite(polarity_control, LOW);
        break;
        
      case 'h':
        digitalWrite(HV_enable, LOW);
        break;
        
     case 'H':
        digitalWrite(HV_enable, HIGH);
        break;        
        
      case 'r':
        digitalWrite(relay_ctl, LOW);
        break;
        
     case 'R':
        digitalWrite(relay_ctl, HIGH);
        break;

     case 'a':
        printout_ADC();
        break;
        
     case 'd':
        set_DAC(Serial.parseInt()*100);
        break;
    }
  }
}

/***********************************************************
 * DATA FUNCTIONS
 ***********************************************************/

long read_ADC(int ch)
{
  // select channel (see LTC2496 datasheet, pp. 14-15)
  data[0] = 0b10100011 + ch;
  write_SPI_ADC();

  // interpret the measured voltage
  long val = (data[2] >> 4) + (data[1] << 4) + (((long)data[0] & 0b00001111) << 12);

  // interpret the status bits
  byte status_bits = data[0] >> 4;
  switch (status_bits) {
      case 0b0011: // positive out of range
        val = 99999;
        break;
        
      case 0b0010: // positive
        break;
        
      case 0b0001: // negative
        val = -val;
        break;

      case 0b0000: // negative out of range
        val = -99999;
        break;
  }

  return val;
}

void printout_ADC()
{
  for (int i=0; i<num_ch-1; i++) {
    Serial.print(values[i]);
    Serial.print(",");
  }
  Serial.println(values[num_ch-1]);
}

void set_DAC(int val)
{
  data[0] = val >> 8;
  data[1] = val;
  write_SPI_DAC();
}

/***********************************************************
 * CONFIGURATION FUNCTIONS
 ***********************************************************/



/***********************************************************
 * LOW-LEVEL FUNCTIONS
 ***********************************************************/

void write_SPI_ADC()
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_ADC, LOW);

  // send or receive data
  SPI.transfer(data, 3);

  // end transaction
  digitalWrite(CS_ADC, HIGH);
  SPI.endTransaction();
}

void write_SPI_DAC()
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_DAC, LOW);

  // send or receive data
  SPI.transfer(data, 2);

  // end transaction
  digitalWrite(CS_DAC, HIGH);
  SPI.endTransaction();
}
