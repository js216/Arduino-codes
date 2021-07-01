#include <SPI.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// pin configuration
const int ramp_trig  = 2;
const int CS_DAC    = 8;
const int CS_ADC    = 10;
const int MISO_pin  = 12;
const int MOSI_pin  = 11;
const int SCK_pin   = 13;

// state variables
unsigned int ADC_data[2];
unsigned int DAC_data[3];
volatile bool triggered = false;

// peak detection constants
const int peak_thr = 3100;

// PID variables
bool PID_enabled=0, printing_enabled=1;
int Ki = 1;
int SP = 75;
int accumulator = 14500;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  SPI.setMOSI(MISO_pin);
  SPI.setMISO(MOSI_pin);
  SPI.setSCK(SCK_pin);

  // pin configuration
  pinMode(CS_DAC, OUTPUT);
  digitalWrite(CS_DAC, HIGH);
  pinMode(CS_ADC, OUTPUT);
  digitalWrite(CS_ADC, HIGH);
  pinMode(ramp_trig, INPUT);

  configure_ADC();
  configure_DAC();

  attachInterrupt(digitalPinToInterrupt(ramp_trig), trig_ISR, RISING);

  set_ramp(25000, 65000, 4000);
}

void trig_ISR()
{
  triggered = true;
}

bool toggle_flag;

void loop() {
  if (PID_enabled && triggered) {
    // clear triggered flag
    cli();
    triggered = false;
    sei();

    // reset state variables
    int P1_up=0, P1_down=0, P2_up=0, P2_down=0;

    // enumerate datapoints
    for (int i=0; i<65000; i++) {
      // check if triggered again
      if (triggered)
        break;

      // get a new datapoint
      read_ADC();
      
      // record time of Peak 1 rising
      if ((!P1_up) && (ADC_data[0] > peak_thr))
        P1_up = i;

      // record time of Peak 1 falling
      if (P1_up && (!P1_down) && (ADC_data[0] < peak_thr))
        P1_down = i;

      // record time of Peak 2 rising
      if (P1_down && (ADC_data[0] > peak_thr))
        P2_up = i;

      // record time of Peak 2 falling
      if (P2_up && (!P2_down) && (ADC_data[0] < peak_thr))
        P2_down = i;

      // finished the locking loop
      if (P2_down) {
        const int PV = P2_down - P1_down;
        const int err = PV - SP;
        accumulator += err;

        // limit the accumulator to positive values
        if (accumulator > 0)
          set_slave(accumulator * Ki);
        else
          accumulator = 0;

        // printout PID parameters
        if (printing_enabled) {
          Serial.print(PV);
          Serial.print('\t');
          Serial.print(SP);
          Serial.print('\t');
          Serial.println(accumulator);
        }
        break;
      }
    }
  }
}


void serialEvent() {
  while (Serial.available()) {
    // read user input
    char c = (char)Serial.read();

    // decide what to do with it
    switch (c) {
      case '?':
        Serial.write("Lock board v2.0 ready.\n");
        break;

      case 'p':
        print_ADC();
        break;

      case 'r':
        set_ramp(Serial.parseInt(), Serial.parseInt(), Serial.parseInt());
        break;
        
      case 's':
        set_slave(Serial.parseInt() * 1000);
        break;

      // PID control

      case 'e':
        PID_enabled = Serial.parseInt();
        accumulator = 5000;
        break;

      case 'S':
        SP = Serial.parseInt();
        break;

      case 'i':
        Ki = Serial.parseInt();
        break;

      case 'P':
        printing_enabled = Serial.parseInt();
        break;
    }
  }
}

/////////////////////////////////
/// ADC FUNTIONS ///////////////
/////////////////////////////////

void print_ADC()
{
  read_ADC();
  Serial.print(ADC_data[0]);
  Serial.print(", ");
  Serial.println(ADC_data[1]);
}

void read_ADC()
{
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_ADC, LOW);
  for (int i=0; i<2; i++)
    ADC_data[i] = SPI.transfer16(0);
  digitalWrite(CS_ADC, HIGH);
  SPI.endTransaction();
}

void configure_ADC()
{
  // make data packet (see AD4683 datasheet, p. 28)
  int config2 = (1 << 15); // write to ...
  config2    |= (1 << 13); // .. the register CONFIGURATION2
  config2    |= (1 << 8);  // enable 1-wire mode
  
  // transmit data
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_ADC, LOW);
  SPI.transfer16(config2);
  digitalWrite(CS_ADC, HIGH);
  SPI.endTransaction();
}

/////////////////////////////////
/// DAC FUNTIONS ///////////////
/////////////////////////////////

void set_ramp(const unsigned int ramp_min, const unsigned int ramp_max,
              const unsigned int ramp_slope)
{
  // Circuit design limitation: in order to get positive slope from
  // an inverting opamp with single positive supply, ramp_slope is subtracted
  // from ramp_min. This effectively gives the ramp_slop a negative sign,
  // resulting in a positive ramp. However, ramp_slope must be smaller than
  // ramp_min from which it's subtracted.
  if (ramp_min < ramp_slope)
   return;

  // write values to DAC
  write_DAC(0, ramp_max);
  write_DAC(2, ramp_min);
  write_DAC(3, ramp_slope);
}


void set_slave(const unsigned int val)
{
  write_DAC(1, val);
}

void write_DAC(const unsigned int ch, const unsigned int val)
{
  // write to and update DAC channel ch (see AD5664R datasheet, p. 21)
  DAC_data[0] = (3 << 3) | ch;
  DAC_data[1] = (val >> 8);
  DAC_data[2] = val;

  // transmit data to the DAC
  DAC_SPI();
}

void configure_DAC()
{
  // enable internal reference (see AD5664R datasheet, p. 23)
  DAC_data[0] = (7 << 3);
  DAC_data[1] = 0;
  DAC_data[2] = 1;

  // transmit data to the DAC
  DAC_SPI();

  // zero all outputs
  write_DAC(7, 0);
}

void DAC_SPI()
{
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_DAC, LOW);
  for (int i=0; i<3; i++)
    SPI.transfer(DAC_data[i]);
  digitalWrite(CS_DAC, HIGH);
  SPI.endTransaction();
}
