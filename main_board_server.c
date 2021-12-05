//출력하는 쪽
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <string.h>

#define SERVICE_UUID        "28abcba1-7164-4c84-95f0-880f4a52b3c7"
#define CHARACTERISTIC_UUID "07ff6a94-bc77-49c7-b8f6-33bcee6db341"
#define LED_GPIO 15

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      Serial.println("connected. Push button to turn on Led");
      
      if(value[0] == '1'){
        digitalWrite(LED_GPIO, HIGH);
        delay(10);
        //digitalWrite(LED_GPIO, LOW);
      }
      else{
        digitalWrite(LED_GPIO, LOW);
        delay(10);
      }
      delay(10);
    }
};

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);
  
  BLEDevice::init("ESP1"); //디바이스 ID 설정
  BLEServer *pServer = BLEDevice::createServer();   //디바이스에 서버 생성

  BLEService *pService = pServer->createService(SERVICE_UUID);  //서버의 서비스 생성
  
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(  //서비스 기능 생성
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Hello World");
  pService->start();
  pServer->getAdvertising()->addServiceUUID(SERVICE_UUID);
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  
  

}

void loop() {
  
}