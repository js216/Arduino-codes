#include <sps30.h>
#include <Adafruit_BME280.h>

// connection for BME280
#define BME_CS A0
Adafruit_BME280 bme(BME_CS);

// internal state
struct sps30_measurement m;
unsigned long previousMillis = 0;
bool printing_enable = false;

// constants
const int update_interval = 1000;

void setup() {
  int16_t ret;
  const uint8_t auto_clean_days = 4;

  Serial.begin(9600);
  while(!Serial);
  delay(2000);

  // check for BME280 sensor
  unsigned status;
  status = bme.begin();  
  if (!status) {
    Serial.println("BME280 error");
    while (1) delay(10);
  }

  sensirion_i2c_init();

  // probe the sensor to verify communication
  while (sps30_probe() != 0) {
    Serial.print("SPS sensor probing failed\n");
    delay(500);
  }

  // set auto-cleaning period
  ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
  if (ret) {
    Serial.print("error setting the auto-clean interval: ");
    Serial.println(ret);
  }

  // start the measurement
  ret = sps30_start_measurement();
  if (ret < 0) {
    Serial.print("error starting measurement\n");
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
        Serial.println("SPS30_mounting v1.0 ready.");
        break;

       case 'r':
        printout_measurements();
        break;
    }
  }
}

void loop() {
  check_serial();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= update_interval) {
    previousMillis = currentMillis;
    read_sensor();

    if (printing_enable)
      printout_measurements();
  }
}

void read_sensor()
{
  uint16_t data_ready;
  int16_t ret;

  // check if data is ready, wait if not
  do {
    ret = sps30_read_data_ready(&data_ready);
    if (ret < 0) {
      Serial.print("error reading data-ready flag: ");
      Serial.println(ret);
    } else if (!data_ready)
      Serial.print("data not ready, no new measurement available\n");
    else
      break;
    delay(100); /* retry in 100ms */
  } while (1);

  // read the measurement
  ret = sps30_read_measurement(&m);
  if (ret < 0)
    Serial.print("error reading measurement\n");
}

void printout_measurements()
{
    Serial.print(m.mc_1p0);
    Serial.print(",");
    Serial.print(m.mc_2p5);
    Serial.print(",");
    Serial.print(m.mc_4p0);
    Serial.print(",");
    Serial.print(m.mc_10p0);

    Serial.print(",");
    Serial.print(m.nc_0p5);
    Serial.print(",");
    Serial.print(m.nc_1p0);
    Serial.print(",");
    Serial.print(m.nc_2p5);
    Serial.print(",");
    Serial.print(m.nc_4p0);
    Serial.print(",");
    Serial.print(m.nc_10p0);

    Serial.print(",");
    Serial.print(bme.readTemperature());       // °C
    Serial.print(',');
    Serial.print(bme.readPressure() / 100.0F); // hPa
    Serial.print(',');
    Serial.print(bme.readHumidity());          // %
    Serial.println();   
}
