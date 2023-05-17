#include <SPI.h>

// choose which board we're programming
#define BOARD_A

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
bool reading=true;

// for sine wave output
bool sine=false;
long int t=0;
int f0=1;
int amp=1000;
int offs=5000;

// constants
const int update_interval = 200;
const int num_ch = 5;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  pinMode(CS_ADC, OUTPUT);
  pinMode(CS_DAC, OUTPUT);
  pinMode(relay_ctl, OUTPUT);
  pinMode(HV_enable, OUTPUT);
  pinMode(polarity_control, OUTPUT);

  digitalWrite(CS_ADC, HIGH);
  digitalWrite(CS_DAC, HIGH);
  digitalWrite(relay_ctl, HIGH);
  digitalWrite(HV_enable, HIGH);
  #ifdef BOARD_A
     digitalWrite(polarity_control, HIGH);
  #endif
  #ifdef BOARD_B
     digitalWrite(polarity_control, LOW);
  #endif
}

void loop() {
  check_serial();

  if (reading) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= update_interval) {
      previousMillis = currentMillis;
      values[ch] = read_ADC(ch);
      ch = (ch + 1) % num_ch;
    }
  }

  if (sine) {
    set_DAC( int(amp*sin(f0*t/100000.0) + offs) );
    t++;
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
        #ifdef BOARD_A
           Serial.println("HV_control v1.3 ready. Board A.");
        #endif
        #ifdef BOARD_B
           Serial.println("HV_control v1.3 ready. Board B.");
        #endif
        break;
        
      case 'p':
        digitalWrite(polarity_control, Serial.parseInt());
        break;

      case 'P':
        Serial.println(digitalRead(polarity_control));
        break;

      case 'h':
        digitalWrite(HV_enable, Serial.parseInt());
        break;

      case 'H':
        Serial.println(digitalRead(HV_enable));
        break;
        
//      case 'r':
//        digitalWrite(relay_ctl, Serial.parseInt());
//        break;
        
     case 'a':
        printout_ADC();
        break;

//     case 'A':
//       reading = Serial.parseInt();
//       break;
//
//     case 'S':
//       sine = Serial.parseInt();
//       f0 = Serial.parseInt();
//       amp = Serial.parseInt();
//       offs = Serial.parseInt();
//       break;

     case 'd':
        #ifdef BOARD_A
           set_DAC(0.961*Serial.parseInt());
        #endif
        #ifdef BOARD_B
           set_DAC(0.726*Serial.parseInt());
        #endif
        break;
    }
  }
}

/***********************************************************
 * DATA FUNCTIONS
 ***********************************************************/

float ADC_to_volts(const long ADC_val)
{
  return ((float)ADC_val - 32768) / 65535 * (2 * 3 * 4.096);
}

void printout_ADC()
{
  for (int i=0; i<num_ch-1; i++) {
    Serial.print((values[i]));
    Serial.print(",");
  }
  Serial.println((values[num_ch-1]));
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
