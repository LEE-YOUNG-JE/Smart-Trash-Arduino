/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/
#include <Adafruit_GFX.h> 
#include <Adafruit_IS31FL3731.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <string.h>
#include <SparkFunBME280.h>

#define BME280_ADDR 0x77

Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731();
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define _LED_R_GPIO 12  //CRIME_PREVENTION
#define _LED_G_GPIO 13  //OPEN
#define _LED_B_GPIO 14  //CLOSE

#define SERVICE_UUID        "b3241fb1-50e4-4b58-b6c7-78366fd47a57"
#define CHARACTERISTIC_UUID "35c68121-8050-4acc-87bb-5807e2589a21"
#define _LED_GPIO 15
#define OPEN 1
#define CLOSE 0
#define CRIME_PREVENTION 2

#define _MIC_36 36
BME280 myBME280;
void matrix_print(int n);
void state_print();
int flag_t = 0;
char t_String[8];

std::string PW = "1106";
std::string close_ = "close";
std::string prevention = "prevention";
std::string state_ = "state";
std::string temp_ = "temp";
std::string amount_ = "amount";
std::string reuse_ = "reuse";
std::string smell_ = "smell";
std::string mise_ = "mise";

int board_status = CLOSE;
float thi, T, Tw, rh; //불쾌지수

static const uint8_t PROGMEM initial[5][9]={
 {B01110000,
  B01001000,
  B01110000,
  B01000000,
  B01000000,
  B01010101,
  B00010101,
  B00010101,
  B00001010},     //pw 0
 
  {B00000000, 
 B01000010, 
 B10100101, 
 B00000000, 
 B01000010, 
 B00111100, 
 B00000000},     //^^ 1

 
{ B00000000, 
 B10000001, 
 B01000010, 
 B00000000, 
 B00000000, 
 B00111100, 
 B01000010},   //;( 2

  
  { B00000000, 
 B01100110, 
 B00000000, 
 B00000000, 
 B00000000, 
 B00111100, 
 B00000000},  //-_- 3
  
  { B11111111,
  B11000111,
  B10111111,
  B10111111,
  B11000111,
  B11111011,
  B11111011,
  B11000111,
  B11111111}   //S
 };
const char text[3]={'X','O','C'};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
       // Serial.println("*********");
       // Serial.print("New value: ");
        /*for (int i = 0; i < value.length(); i++)
        {            
          Serial.print(value[i]);
        } */
        if(board_status == CLOSE){
  digitalWrite(_LED_GPIO,LOW);  //digitalWrite(_LED_R_GPIO, HIGH);
          if(value == PW){
            Serial.println("1106");
            board_status = OPEN;
            digitalWrite(_LED_GPIO, HIGH);
            matrix_print(OPEN);
            delay(1000);
          }else{
            matrix_print(CLOSE);
            delay(2000);
            matrix_print(78);
          }
        }
        else if(board_status == OPEN){
           digitalWrite(_LED_GPIO,HIGH); 
          if(value == prevention) {  //방범모드 
            matrix_print(CRIME_PREVENTION);
            board_status = CRIME_PREVENTION;
          }
          if(value == close_) {  //close
            matrix_print(78);
            board_status = CLOSE;
            Serial.println("close");
          }
          if(value == state_){
            state_print(); 
          }
          if(value == temp_){
            Serial.println("a");
          }
          if(value == amount_){
            Serial.println("b");
          }
          if(value == reuse_){
            Serial.println("c");
          }
          if(value == smell_){
            Serial.println("d");
          }
          if(value == mise_){
            Serial.println("mise");
          }
        }
        else if(board_status == CRIME_PREVENTION){
          if(value == PW){
            board_status = OPEN;
            digitalWrite(_LED_GPIO, HIGH);
            ledcWrite(0, 0);
            matrix_print(OPEN);
            delay(1000);
          }
        }
       
      //  Serial.println();
      // Serial.println("*********");
      }
    }
};

int freq = 1046; //1318
void setup() {
ledcSetup(0, 1046, 8);
ledcAttachPin(25, 0);
pinMode(2, OUTPUT);
digitalWrite(2, HIGH);
  
myBME280.setI2CAddress(BME280_ADDR);
myBME280.setReferencePressure(101170);
myBME280.beginI2C();

   pinMode(_LED_R_GPIO,OUTPUT);
  digitalWrite(_LED_R_GPIO,LOW);
  pinMode(_LED_G_GPIO,OUTPUT);
  digitalWrite(_LED_G_GPIO,LOW);
  pinMode(_LED_B_GPIO,OUTPUT);
  digitalWrite(_LED_B_GPIO,LOW);
  
  matrix.begin();
  pinMode(_LED_GPIO,OUTPUT);
  digitalWrite(_LED_GPIO,LOW);  //digitalWrite(_LED_R_GPIO, HIGH);
  
  Serial.begin(115200);
  BLEDevice::init("MyESP32_5");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
 
  
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  matrix_print(78);
}

void matrix_print(int n){
 if(n == 78) {
  matrix.clear();
  matrix.drawBitmap(0,0,initial[0], 8,9, 128);  //
 }
 else if(n == 10){
  matrix.clear();
  matrix.drawBitmap(0,0,initial[1], 8,9, 128);  // ^^ 10
  delay(2000);
  matrix.clear();
  matrix.printf("%c", text[1]);
 }
 else if(n == 11){
  matrix.clear();
  matrix.drawBitmap(0,0,initial[2], 8,9, 128);  // ;( 11
  delay(2000);
  matrix.clear();
  matrix.printf("%c", text[1]);
 }
 else if(n == 12){
  matrix.clear();
  matrix.drawBitmap(0,0,initial[3], 8,9, 128);  // -_- 12
  delay(2000);
  matrix.clear();
  matrix.printf("%c", text[1]);
 }
 else{
    matrix.clear();
    matrix.setCursor(1,1);
    matrix.printf("%c", text[n]);
  }
}



void loop() {
  int i = 0 ;
  // put your main code here, to run repeatedly:
  if(board_status == OPEN){
     digitalWrite(_LED_G_GPIO,HIGH);
     digitalWrite(_LED_R_GPIO,LOW);
     digitalWrite(_LED_B_GPIO,LOW);
  }
  else if(board_status == CLOSE){
    digitalWrite(_LED_B_GPIO,HIGH);
    digitalWrite(_LED_R_GPIO,LOW);
    digitalWrite(_LED_G_GPIO,LOW);
  }
  else if(board_status == CRIME_PREVENTION){
    digitalWrite(_LED_R_GPIO,HIGH);
    digitalWrite(_LED_B_GPIO,LOW);
    digitalWrite(_LED_G_GPIO,LOW);
    //마이크 감지되면 경보음
    Serial.println(analogRead(_MIC_36));
    if(analogRead(_MIC_36) > 1000){
      Serial.println("w");
      ledcWrite(0, 255);
      for(i=0;i<3;i++){
        ledcWriteTone(0, 1318);
        delay(200);
        ledcWriteTone(0, 1046);
        delay(200);
      }
    }
  }
  delay(2);
}

void state_print(){
   rh = myBME280.readFloatHumidity(); //변수 rh에 상대습도 저장
   T = myBME280.readTempC(); //변수 T에 섭씨온도 저장
 
 
 Tw = T * atan(0.151977 * pow((rh*0.01 + 8.313659),(1/2))) + atan(T + rh*0.01) - atan(rh*0.01 - 
1.676331) + 0.00391838 *pow((rh*0.01),(3/2)) * atan(0.023101 * rh*0.01) - 4.686035; 
// https://perryweather.com/2020/04/01/what-is-wbgt-and-how-do-you-calculate-it/
//습구온도 계산 
 
 //thi= 0.72*(T+Tw)+40.6; 
 thi = 71;
 //불쾌지수=0.72*(기온+습구온도)+40.6
// https://m.blog.naver.com/kyoungin90/221342938432
 
 matrix.clear(); //Matrix 화면 초기화
 matrix.setCursor(1,1); //Matrix 커서 설정 
// https://web.kma.go.kr/weather/lifenindustry/li_asset/HELP/basic/help_01_05.jsp
//불쾌지수 단계 기준 reference 
 if(thi<70) matrix_print(10); //70미만일 때 낮음, 웃음 이모티콘 출력
 else if(thi>75) matrix_print(11); //75초과일 때 높음, 화남 이모티콘 출력
 else matrix_print(12); //70이상 75이하일 때 보통, 무표정 이모티콘 출력
 delay(1000); //한번의 출력 당 1초간 지연

}