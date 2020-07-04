#include <ArduinoBLE.h>
#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>
//#include <Arduino_LSM9DS1.h>

// for sensor data
int temperature;
unsigned int humidity;
unsigned int pressure;
//float B[3];

// for loop delay
unsigned long previousMillis = 0;
const int dt = 2000;

// BLE objects
BLEDevice conn;
BLEService envServ("181A");
BLEIntCharacteristic tempCh("2A6E", BLERead | BLENotify);
BLEUnsignedIntCharacteristic humCh("2A6F", BLERead | BLENotify);
BLEUnsignedIntCharacteristic presCh("2A6D", BLERead | BLENotify);
//BLECharacteristic magnCh("2AA1", BLERead | BLENotify, 12);

void setup()
{
  Serial.begin(9600);
  HTS.begin();
  BLE.begin();
  BARO.begin();
//  IMU.begin();

  // configure bluetooth
  BLE.setLocalName("Nano33BLESENSE");
  BLE.setAdvertisedService(envServ);
  envServ.addCharacteristic(tempCh);
  envServ.addCharacteristic(humCh);
  envServ.addCharacteristic(presCh);
//  envServ.addCharacteristic(magnCh);
  BLE.addService(envServ);
//  tempCh.setValue(0);
//  humCh.setValue(0);
//  presCh.setValue(0);
  BLE.advertise();
}

void loop()
{
  if (conn = BLE.central()) {
    Serial.print("Connected to central MAC: ");
    Serial.println(conn.address());

    while (conn.connected()) {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= dt) {
        previousMillis = currentMillis;
        updateReadings();
      }
    }

    Serial.print("Disconnected from central MAC: ");
    Serial.println(conn.address());
  }
}

void updateReadings()
{
        // get data from sensors
        temperature = 100 * HTS.readTemperature();
        humidity = 100 * HTS.readHumidity();
        pressure = 10000 * BARO.readPressure();
//        if (IMU.magneticFieldAvailable())
//          IMU.readMagneticField(B[0], B[1], B[2]);

        // update BLE characteristics
        tempCh.setValue(temperature);
        humCh.setValue(humidity);
        presCh.setValue(pressure);
//        magnCh.setValue((byte *) &B, 12);
}
