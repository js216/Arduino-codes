#include <ArduinoBLE.h>
#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>
//#include <Arduino_LSM9DS1.h>

// for sensor data
int temperature;
int humidity;
int pressure;
//float Ax,Ay,Az, Gx,Gy,Gz, Mx,My,Mz;

// for loop delay
unsigned long previousMillis = 0;
const int dt = 2000;

// BLE objects
BLEDevice conn;
BLEService envServ("181A");
BLEIntCharacteristic tempCh("2A6E", BLERead | BLENotify);
BLEUnsignedIntCharacteristic humCh("2A6F", BLERead | BLENotify);
BLEUnsignedIntCharacteristic presCh("2AA3", BLERead | BLENotify);
//BLEUnsignedIntCharacteristic magnCh("2AA1", BLERead | BLENotify);

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
  tempCh.setValue(0);
  humCh.setValue(0);
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
//        pressure = 100 * BARO.readPressure();
//        if (IMU.accelerationAvailable())
//          IMU.readAcceleration(Ax, Ay, Az);
//        if (IMU.gyroscopeAvailable())
//          IMU.readGyroscope(Gx, Gy, Gz);
//        if (IMU.magneticFieldAvailable())
//          IMU.readMagneticField(Mx, My, Mz);

        // write to serial
        Serial.print(temperature/100.);
        Serial.print(" degC, ");
        Serial.print(humidity/100.);
        Serial.print(" %, ");
        Serial.print(pressure/100.);
        Serial.println(" kPa");

        // update BLE characteristics
        tempCh.setValue(temperature);
        humCh.setValue(humidity);
//        presCh.setValue(pressure);
//        magnCh.setValue((int)100*Mx);
}
