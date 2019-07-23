/*
This makes the Arduino into a serial instrument with the connection parameters:
instr.baud_rate = 9600
instr.data_bits = 8
instr.stop_bits = pyvisa.constants.StopBits.one
instr.parity = pyvisa.constants.Parity.none

Type '?' over serial to print out the usage guide.

Jakob Kastelic, Feb 8th, 2019
Updated: Apr 1st, 2019
 */

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// The nitrogen valve relay control is pin 3 of the Arduino
#define VALVE_BUS 3

float max_temp = 86; // maximum safe temperature

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int num_dev; // number of thermometers attached

int valve_state = -1; // whether valve is open or closed

void setup(void)
{
  Serial.begin(9600);
  sensors.begin();

  // set the valve pin in a definite state
  pinMode(VALVE_BUS, OUTPUT);
  digitalWrite(VALVE_BUS, LOW);
  valve_state = 0;

  // find out the number of thermometers attached
  num_dev = sensors.getDeviceCount();
  if (num_dev < 1)
    Serial.print("Error: No thermometers found\n");
}

// print to serial the device address
void print_address(DeviceAddress& dev, int i)
{
  if (! sensors.getAddress(dev, i) ) {
    Serial.print("Error: Cannot find address for Device");
    Serial.print(i);
  } else {
    for (int i = 0; i < 8; ++i) {
      if (dev[i] < 16)
        Serial.print("0");
      Serial.print(dev[i]);
    }
  }
}

void print_all_addresses() {
  DeviceAddress devices[num_dev];
  for (int i = 0; i < num_dev - 1; ++i) {
    print_address(devices[i], i);
    Serial.print(",");
  }
  print_address(devices[num_dev - 1], num_dev - 1);
  Serial.print("\n");
}

void print_all_temperatures()
{
  sensors.requestTemperatures();
  for (int i = 0; i < num_dev - 1; ++i) {
    Serial.print(sensors.getTempCByIndex(i));
    Serial.print(",");
  }
  Serial.print(sensors.getTempCByIndex(num_dev - 1));
  Serial.print("\n");
}

void serialEvent()
{
  while (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      case 'c':
        digitalWrite(VALVE_BUS, LOW);
        valve_state = 0;
        Serial.print("Valve closed.\n");
        break;

      case 'o':
        digitalWrite(VALVE_BUS, HIGH);
        valve_state = 1;
        Serial.print("Valve opened.\n");
        break;

      case 's':
        if (valve_state == 1) {
           Serial.print("Valve opened.\n");
        } else if (valve_state == 0) {
           Serial.print("Valve closed.\n");
        } else {
          Serial.print("Unknown valve state.\n");
        }
        break;

      case 'a':
        print_all_addresses();
        break;

      case 't':
        print_all_temperatures();
        break;

      case 'i':
        Serial.print("Beam Source Thermometer Arduino\n");
        break;

      case '?':
        Serial.print("i = query identification, a = print all addresses, t = print all temperatures,");
        Serial.print("c = close valve, o = open valve, s = valve status\n");
        break;
    }
  }
}

void loop(void)
{
  delay(10);
}
