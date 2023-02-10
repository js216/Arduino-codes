#include <sps30.h>

void setup() {
  int16_t ret;
  const uint8_t auto_clean_days = 4;

  Serial.begin(9600);
  delay(2000);

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

  // start hte measurement
  ret = sps30_start_measurement();
  if (ret < 0) {
    Serial.print("error starting measurement\n");
  }

  delay(1000);
}

void loop() {
  struct sps30_measurement m;
  char serial[SPS30_MAX_SERIAL_LEN];
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
  if (ret < 0) {
    Serial.print("error reading measurement\n");
  } else {
    // print out the mass concentrations
    Serial.print(m.mc_1p0);
    Serial.print(",");
    Serial.print(m.mc_2p5);
    Serial.print(",");
    Serial.print(m.mc_4p0);
    Serial.print(",");
    Serial.println(m.mc_10p0);

    // print out the number concentrations
//    Serial.print(",");
//    Serial.print(m.nc_0p5);
//    Serial.print(",");
//    Serial.print(m.nc_1p0);
//    Serial.print(",");
//    Serial.print(m.nc_2p5);
//    Serial.print(",");
//    Serial.print(m.nc_4p0);
//    Serial.print(",");
//    Serial.println(m.nc_10p0);
  }

  delay(1000);
}
