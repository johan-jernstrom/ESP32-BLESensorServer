#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define BLE_DEVICE_NAME "ESP32 BLE Sensor Server"
#define BLE_SERVICE_UUID "f6fb45ea-8102-42c4-8cfb-ac5be07f5dbe"             // Generated by https://www.uuidgenerator.net/
#define BLE_TANK1CHARACTERISTIC_UUID "1e424618-a1f2-4a35-a6c3-3b65997badbe" // Generated by https://www.uuidgenerator.net/
// #define BLE_TANK1DESCRIPTOR_UUID "2ff8c54a-1d1f-4289-9885-2e9d0b6d4c53" // Generated by https://www.uuidgenerator.net/
#define BLE_TANK2CHARACTERISTIC_UUID "d167981d-6a98-4be3-adf9-d90a0dfc56b7" // Generated by https://www.uuidgenerator.net/
// #define BLE_TANK2DESCRIPTOR_UUID "1d499af2-deee-4d3a-a3be-f1df1633a875" // Generated by https://www.uuidgenerator.net/

uint8_t LED_PIN = 35;
bool deviceConnected = false;

BLEServer *pServer = NULL;

BLECharacteristic *pCharacteristicTank1 = NULL;
BLEDescriptor *pDescrTank1;
BLE2902 *pBLE2902Tank1;

BLECharacteristic *pCharacteristicTank2 = NULL;
BLEDescriptor *pDescrTank2;
BLE2902 *pBLE2902Tank2;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    digitalWrite(LED_PIN, LOW);
    Serial.println("Device connected!");
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Device disconnected");

    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Re-start advertising " + String(BLE_DEVICE_NAME) + "...");
  }
};

void setupBLE()
{
  Serial.println("Setting up BLE...");
  BLEDevice::init(BLE_DEVICE_NAME);
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(BLE_SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristicTank1 = pService->createCharacteristic(BLE_TANK1CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  // pDescrTank1 = new BLEDescriptor((uint16_t)0x2901);
  // pDescrTank1->setValue("Tank 1 Level in percentage");
  // pCharacteristicTank1->addDescriptor(pDescrTank1);
  pBLE2902Tank1 = new BLE2902();
  pBLE2902Tank1->setNotifications(true);
  pCharacteristicTank1->addDescriptor(pBLE2902Tank1);

  pCharacteristicTank2 = pService->createCharacteristic(BLE_TANK2CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  // pDescrTank2 = new BLEDescriptor((uint16_t)0x2901);
  // pDescrTank2->setValue("Tank 2 Level in percentage");
  // pCharacteristicTank2->addDescriptor(pDescrTank2);
  pBLE2902Tank2 = new BLE2902();
  pBLE2902Tank2->setNotifications(true);
  pCharacteristicTank2->addDescriptor(pBLE2902Tank2);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  // pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  // pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  digitalWrite(LED_PIN, HIGH);
  Serial.println("BLE Setup Done. Started advertising " + String(BLE_DEVICE_NAME) + "...");
}

void SendSensorValue(BLECharacteristic *pCharacteristic, double value)
{
  pCharacteristic->setValue(value);
  // pCharacteristic->notify();
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);
  Serial.println("Sending value: " + String(value) + " to client for characteristic: " + pCharacteristic->getUUID().toString().c_str());
}

void setup()
{
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  setupBLE();
}

// put your main code here, to run repeatedly:
void loop()
{
  if (deviceConnected)
  {
    // create random double value between 0 and 100 with two decimals
    double value1 = random(0, 10000) / 100.0;
    double value2 = random(0, 10000) / 100.0;
    SendSensorValue(pCharacteristicTank1, value1);
    SendSensorValue(pCharacteristicTank2, value2);
  }
  else
  {
    Serial.println("Waiting for connection...");
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
  }
  delay(3000); // bluetooth stack will go into congestion, if too many packets are sent
}