 #include <SPI.h>

// pin configuration
const int CS_ADC = 3;
const int CS_DAC = 4;
const int relay_ctl = 2;
const int polarity_control = 5;
const int HV_enable = 6;

// internal state
byte data[4];
bool printing=false;
int ch=0;
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

  if (printing) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= update_interval) {
      previousMillis = currentMillis;
      values[ch] = read_ADC(ch);
      ch = (ch + 1) % num_ch;
  
      if ((ch%num_ch) == 0)
        printout_ADC();
    }
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
        Serial.println("HV_control v1.2 ready.");
        break;
        
      case 'p':
        digitalWrite(polarity_control, Serial.parseInt());
        break;

      case 'h':
        digitalWrite(HV_enable, Serial.parseInt());
        break;
        
      case 'r':
        digitalWrite(relay_ctl, Serial.parseInt());
        break;
        
     case 'a':
        printing = Serial.parseInt();
        break;

     case 'A':
        read_ADC(Serial.parseInt());
        break;
        
     case 'd':
        set_DAC(Serial.parseInt());
        break;
    }
  }
}

/***********************************************************
 * DATA FUNCTIONS
 ***********************************************************/


void printout_ADC()
{
  for (int i=0; i<num_ch-1; i++) {
    Serial.print(values[i]);
    Serial.print(",");
  }
  Serial.println(values[num_ch-1]);
}

/***********************************************************
 * ADC/DAC FUNCTIONS
 ***********************************************************/

unsigned int read_ADC(int ch) // MAX1300BEUG+
{
  // select channel (see MAX1300 datasheet, Table 3)
  data[0] = 0b10000000 + (ch << 4);
  data[1] = 0;
  data[2] = 0;
  data[3] = 0;
  write_SPI_ADC();

  // interpret the results
  return (data[2] << 8) + data[3];
}

void set_DAC(int val) // MAX5216BGUA+
{
  data[0] = (val >> 10) & (0b01111111) | (0b01000000);
  data[1] = val >> 2;
  data[2] = val << 6;
  write_SPI_DAC();
}

/***********************************************************
 * LOW-LEVEL FUNCTIONS
 ***********************************************************/

void write_SPI_ADC()
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_ADC, LOW);

  // send or receive data
  SPI.transfer(data, 4);

  // end transaction
  digitalWrite(CS_ADC, HIGH);
  SPI.endTransaction();
}

void write_SPI_DAC()
{
  // begin transaction
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(CS_DAC, LOW);

  // send or receive data
  SPI.transfer(data, 3);

  // end transaction
  digitalWrite(CS_DAC, HIGH);
  SPI.endTransaction();
}
