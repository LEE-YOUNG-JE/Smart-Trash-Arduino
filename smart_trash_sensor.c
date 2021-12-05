/*----------------BLE client-------*/
#include "BLEDevice.h"
#include "BLEUtils.h"        //사용할 헤더파일 선언
#include <stdlib.h>
#define Button_GPIO 2       //I/O 큐브(버튼 모듈)에 연결한 디지털 핀

static BLEUUID serviceUUID("28abcba1-7164-4c84-95f0-880f4a52b3c7");  //연결하고자 할 server의service UUID

static BLEUUID    charUUID("07ff6a94-bc77-49c7-b8f6-33bcee6db341"); //연결하고자 할 server의 characteristic UUID

static BLEAddress *pServerAddress;
static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLECharacteristic *pCharacteristic;
BLEClient * pClient;


bool connectToServer(BLEAddress pAddress) { //서버에 연결할 때 사용할 함수
    pClient  = BLEDevice::createClient();   //client 생성
    pClient->connect(pAddress);     //pAddress주소에 클라이언트 연결
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID); //서버의 service UUID 받아옴
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID); //서버의 characteristic UUID 받아옴
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks { //advertising된 신호를 감지했을 때 사용하기 위한 클래스
 void onResult(BLEAdvertisedDevice advertisedDevice) {
    if(advertisedDevice.haveServiceUUID()&& advertisedDevice.getServiceUUID().equals(serviceUUID)) { //advertised된 신호가 서비스를 가지고 있으며 찾고자 하는 service UUID와 같은 신호일 때
      advertisedDevice.getScan()->stop();   //advertise하는 신호의 스캔 중지
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //서버 주소 받아옴
      doConnect = true;       //연결됨을 나타내는 변수 doConnect 토글
    } 
 } 
}; 
/*------------------------------*/
#include <Adafruit_GFX.h> 
#include <Adafruit_IS31FL3731.h>


#define _D0_BUTTON_GPIO 2          //BUTTON
#define _D1_LED_GPIO 15            //LED
#define _A0_LIGHT_ADC_GPIO 36      //조도센서
#define _A1_R_GPIO 39              //가변저항
#define _TOUCH_LEFT 27             //왼쪽 터치센서
#define _TOUCH_RIGHT 4             //오른쪽 터치센서
#define plastic 0
#define can 1
#define glass 2



Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731(); //led 메트릭스 제어
const char text[5]={'P','G','C'};
static const uint8_t PROGMEM initial[5][9]={
 {B11111111,
  B11111111,
  B11111111,
  B00011000,
  B11111111,
  B11111111,
  B11000111,
  B11111111,
  B11111111},     //-_-
 
 {B11111111,
  B11111111,
  B10111101,
  B01011010,
  B11111111,
  B11111111,
  B11011011,
  B11100111,
  B11111111},     //^^
 
 {B11111111,
  B01111110,
  B10111101,
  B11011011,
  B11111111,
  B11111111,
  B11000011,
  B10111101,
  B11111111},   //;(
  
  {B11111111,
  B10111101,
  B10111101,
  B10000001,
  B10111101,
  B10111101,
  B10111101,
  B11111111,
  B11111111},   //H
  
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
 
int R=0;
int light=0;  //조도 값
int sel_trash=0;
int button_status=0;
int val = 0;
int old_val = 0;
int state = 0;    
int trash_count = 0;
int led_val = 1;
int led_old_val = 1;
int led_state = 0;            //사용할 변수 선언
char int_to_string[10];
void setup() {
/*------------------sensor setup---------------------*/  
  pinMode(_D0_BUTTON_GPIO, INPUT);  // BUTTON 입력설정
  pinMode(_D1_LED_GPIO, OUTPUT);    // LED 출력설정
  digitalWrite(_D1_LED_GPIO, LOW);
  light=analogRead(_A0_LIGHT_ADC_GPIO); // 초기 조도값 설정

  matrix.begin();
/*---------------------------------------------------*/ 

/*-----------------ble client--------------------*/
Serial.begin(9600);
  pinMode(Button_GPIO, INPUT);      //2번 핀을 INPUT핀으로 설정
    
  BLEDevice::init("");              //client 이름 지정
  BLEScan* pBLEScan = BLEDevice::getScan();   //스캔 시작
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //스캔된 신호의 callback 
  pBLEScan->setActiveScan(true);    //scan을 Activescan으로 설정
  pBLEScan->start(100);     //100초동안 스캔 시작
/*---------------------------------------------*/
}
void loop() {
  /*---------------------터치센서를 이용하여 led 메트릭스 제어------------------------------*/
  matrix.setTextSize(1);
  matrix.setTextColor(100);
  if(button_status == 0){
  if(touchRead(_TOUCH_LEFT)<11 ){
     sel_trash++;
     if(sel_trash>2) sel_trash=0;
  }
  else if(touchRead(_TOUCH_RIGHT)<17 ){
     sel_trash--;
     if(sel_trash<0) sel_trash=2;
  }
     matrix.clear();
     matrix.setCursor(1,1);
     matrix.printf("%c", text[sel_trash]);
  }
 /*-----------------------------------------------------------------------------------*/
delay(500);
 if(!digitalRead(_D0_BUTTON_GPIO) && button_status == 0) button_status = 1;  //상태 변화
 
 if(button_status == 1) {

     matrix.clear();
     matrix.setCursor(1,1);
     matrix.drawBitmap(0,0,initial[0], 8,9, 128);
 /*------------------sensor works---------------------*/
 if(analogRead(_A0_LIGHT_ADC_GPIO) < 500){
 digitalWrite(_D1_LED_GPIO, LOW);
 led_val = digitalRead(_D1_LED_GPIO); //LED의 입력값 저장
Serial.println("led val");
Serial.println(led_val);
Serial.println("led old_val");
Serial.println(led_old_val);
    delay(1500);       //LED 입력 감지를 위한 시간 여유
    led_old_val = led_val;    //old_val에 val값 저장(버튼 눌림 감지는 매우 빠르게 일어나기 때문에 이 값은 거의 LOW로 항상 저장된다.)
 }      
 else if(analogRead(_A0_LIGHT_ADC_GPIO) > 500){   //쓰레기 감지  //수정필요
    R = analogRead(_A1_R_GPIO);  //무게 저장

/* 수도코드
 * if(sel_trash == plastic)인데 플라스틱이 드러왔다? -> 행복 표시  
                               플라스틱이 아닌게 들어왔다?-> 화난표시
*/    
    //블루투스와 wifi로 정보 전송
    digitalWrite(_D1_LED_GPIO, HIGH);
    delay(1500);
    /*--------------------new yj-----count trash-----------------------------*/
led_val = digitalRead(_D1_LED_GPIO); //LED의 입력값 저장
Serial.println("trash_led val");
Serial.println(led_val);
Serial.println("trash_led old_val");
Serial.println(led_old_val);
    delay(100);       //LED 입력 감지를 위한 시간 여유
    if((led_val == HIGH) && (led_old_val == LOW)){  //LED켜지면 쓰레기 들어간것임, 이전값이 LOW일 때(즉, 버튼이 눌렸을 때)
      //led_state = 1 - led_state;      //state값 토글
      Serial.println("pushed_success");
      delay(100);
      trash_count = trash_count + 1;
      Serial.println("trash_count :");
      Serial.println(trash_count);
    }
    led_old_val = led_val;    //old_val에 val값 저장(버튼 눌림 감지는 매우 빠르게 일어나기 때문에 이 값은 거의 LOW로 항상 저장된다.)
    //if(led_state == 1){ //state 1상태일 때(Blink LED가 꺼져있을 때)
      
      //led_state = 0;
    //}    
/*----------------------------------------------------------------------------*/
 }

 /*---------------------------------------------------*/
 }// button_status == 1

 /*--------------------------ble client-------------*/
 if(!digitalRead(_D0_BUTTON_GPIO) && button_status == 1){
if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      connected = true;   //서버와 연결 성공했을 때 connected 변수 토글
    } else {
      Serial.println("We have failed to connect to the server");
    }
    doConnect = false;
  }
    
  while (connected) {  
    val = !digitalRead(_D0_BUTTON_GPIO); //버튼의 입력값 저장
    delay(1000);       //버튼 입력 감지를 위한 시간 여유
    if((val == HIGH)){  //버튼이 눌렸고, 이전값이 LOW일 때(즉, 버튼이 눌렸을 때)
      Serial.println("pushed");
      //delay(1000);
    }
      itoa(trash_count, int_to_string, 10);
      Serial.println("trash int: ");
      Serial.println(trash_count);
      Serial.println("trash string : ");
      Serial.println(int_to_string);
      pRemoteCharacteristic->writeValue(int_to_string[0]); //서버의 charactersitic값을 1로 변경(LED ON)
      trash_count = 0;
      int_to_string[0] = '0';
      delay(100); //여기는 반복되는지 확인용도
      pRemoteCharacteristic->writeValue("0"); //반복되는지 확인용도
      break;
  }  
  button_status = 0;
 }

 /*----------------------------------------*/
delay(500);
}