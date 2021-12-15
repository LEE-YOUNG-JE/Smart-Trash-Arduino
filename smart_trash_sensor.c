#include <WiFi.h>
#include <PubSubClient.h>
#define HICLOUD_API_KEY "2c6edde6db100ad83a509a6cea13a75ef97c3826"
const char* ssid = "Bong";
const char* password = "";
const char* mqtt_server = "202.30.11.115";
float val = 0;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

/*-----------------temp----------------*/
#include "DHT.h"
#define DHTPIN 33     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11  
DHT dht(DHTPIN, DHTTYPE);
/*-------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#define Button_GPIO 2       //I/O 큐브(버튼 모듈)에 연결한 디지털 핀
/*------------------------------*/
#include <Adafruit_GFX.h> 
#include <Adafruit_IS31FL3731.h>


#define _D0_BUTTON_GPIO 2          //BUTTON
#define _D2_LED_GPIO 26            //LED
#define _A0_LIGHT_ADC_GPIO 36      //조도센서
#define _A1_R_GPIO 39              //가변저항
#define _TOUCH_LEFT 27             //왼쪽 터치센서
#define _TOUCH_RIGHT 4             //오른쪽 터치센서
#define _LED_R_GPIO 12  //R
#define _LED_G_GPIO 13  //G
#define _LED_B_GPIO 14  //B
#define plastic 0
#define can 1
#define glass 2
/*-------------------------------------------wifi function---------------------------*/
float a = 0.0;
int all_state = 0; //0이면 처음 쓰레기 선택모드상태, 1이면 버튼 눌린상태 = 쓰레기 인식대기상태, 2이면 hicloud로 데이터 보내는 상태
float pa = 0.0; //amount of plastic amount
float ga = 0.0; //amount of glass
float ca = 0.0; //amount of can 
float R=0.0;
int light=0;  //조도 값
int sel_trash=0; //Plastic이면 0, Glass이면 1 Can이면 2
int PGC[3] = {'P', 'G', 'C'};
char choose_trash = '0';
char web_trash = '0';
int button_status=0;
int old_val = 0;
int state = 0;    
int shadow_val = 1;
int shadow_old_val = 1;
int led_state = 0;            //사용할 변수 선언




Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731(); //led 메트릭스 제어
const char text[5]={'P','G','C'};
static const uint8_t PROGMEM initial[5][9]={
 {B00000000,
  B00000000,
  B00000000,
  B11100111,
  B00000000,
  B00000000,
  B00111000,
  B00000000,
  B00000000},     //-_-
 
 {B00000000,
  B00000000,
  B01000010,
  B10100101,
  B00000000,
  B00000000,
  B00100100,
  B00011000,
  B00000000},     //^^
 
 {B00000000,
  B10000001,
  B01000010,
  B00100100,
  B00000000,
  B00000000,
  B00111100,
  B01000010,
  B00000000},   //;(
  
  {B00000000,
  B01111110,
  B01000000,
  B01111110,
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B00000000},   //H
  
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
 

void setup() {
   dht.begin(); //온습도센서 begin
  setup_wifi(); //wifi setup
/*------------------sensor setup---------------------*/  
 pinMode(_LED_R_GPIO,OUTPUT);
  digitalWrite(_LED_R_GPIO,LOW); 
  pinMode(_LED_G_GPIO,OUTPUT);
  digitalWrite(_LED_G_GPIO,LOW);
  pinMode(_LED_B_GPIO,OUTPUT);
  digitalWrite(_LED_B_GPIO,LOW);
  
  ledcSetup(0, 1046, 8);
  ledcAttachPin(25, 0);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  Serial.begin(115200);
  pinMode(15,OUTPUT);//
 digitalWrite(15,LOW);// 쓰레기통 open이면 HIGH
  pinMode(_D0_BUTTON_GPIO, INPUT);  // BUTTON 입력설정
  pinMode(_D2_LED_GPIO, OUTPUT);    // LED 출력설정
  digitalWrite(_D2_LED_GPIO, LOW);
  light=analogRead(_A0_LIGHT_ADC_GPIO); // 초기 조도값 설정
  
  matrix.begin();
/*---------------------------------------------------*/ 
}
void setup_wifi() {
delay(10);
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println("");
Serial.println("[LOG] WiFi connected");
Serial.println("[LOG] IP address: ");
Serial.println(WiFi.localIP());
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("[LOG] Attempting MQTT connection...");
    if (client.connect("kepler", HICLOUD_API_KEY , "")) {
    Serial.println("[LOG] connected");
    client.publish("/device/42eeebb0-ec07-4de9-bcd6-fb682bd99437/heartbeat", "");
    } else {
    Serial.print("[LOG] failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    delay(5000);
    }
  }
}
/*---------------------------------------------------------------------------*/
void Preprocessing(){ //절약률 전처리하기 위한 전처리 함수
  char EPRString[8];
  float EPR = 0.0; //절약률

 //R은 현재 찍히고 있는 쓰레기의 무게
  if(sel_trash == 0) EPR = R*0.00015;     //Plastic의 절약률계산
  else if(sel_trash == 2) EPR = R*0.0002;  //Can의 절약률계산
  else if(sel_trash == 1) EPR = R*0.00025; //Glass의 절약률계산

  dtostrf(EPR, 6, 4, EPRString);
  delay(2000);
  client.publish("/component/b7d4ddb2-2e79-4a87-abde-9f31406bce7e/pub", EPRString); //EPR
  delay(5000);
  
}
int flag_e = 0; //flg_e=1이면 전처리함수 실행
float R_all = 0; //무게의 총 누적값을 저장하는 전역변수
/*-------------------------------------------wifi function end--------------------------*/
void loop() {
   if(flag_e == 1){ //flag_e가 1이면 전처리 함수 실행
    Preprocessing();
    flag_e = 0;
   }
  client.setServer(mqtt_server, 1883);
  matrix.setTextSize(1);
  matrix.setTextColor(100);
       if (!client.connected()) {
          reconnect();
       }
        client.loop();
    /*---------------------터치센서를 이용하여 led 메트릭스 제어-----------------------------------------------------------loop-------------*/
  if(all_state == 0){ //쓰레기 선택모드, 왼쪽 오른쪽 눌러서 p,g,c 선택
      digitalWrite(15, LOW); //digitalWrite
      if(touchRead(_TOUCH_LEFT)<10 ){
        sel_trash++;
        if(sel_trash>2) sel_trash=0;
      }
      else if(touchRead(_TOUCH_RIGHT)<16 ){
         sel_trash--;
        if(sel_trash<0) sel_trash=2;
      }
         matrix.clear();
         matrix.setCursor(1,1);
         matrix.printf("%c", text[sel_trash]);
  }


/*--------------------------------------------------------------------------------------------------------------button_status == 0 && button눌렸을때*/
 if(!digitalRead(_D0_BUTTON_GPIO) && all_state == 0) { // 선택화면에서 버튼을 누른상태
    float t = dht.readTemperature();
    char t_String[8];
    dtostrf(t, 6, 4, t_String);
    client.publish("/component/8ef61501-a4ac-4df4-a073-ae91b58b7437/pub", t_String); //온도를 cloud에 전송
    delay(3000);
    all_state = 1;  //상태 1로 변화
  }

 if(all_state == 1) { //1인상태는 선택화면에서 버튼을 누른 상태
     matrix.clear();
     matrix.setCursor(1,1);
     matrix.drawBitmap(0,0,initial[0], 8,9, 128); // 무표정으로 출력
 /*------------------sensor works------------------------22222222222222222222222222222222222---------------쓰레기 감지 안될때--*/
    char temp = Serial.read();
    if(temp == '0' || temp == '1' || temp =='P' || temp =='G' || temp =='C' || temp=='X') web_trash = temp; // 쓰레기값을 거르고 0,1,P,G,C,X만 취득
    Serial.println(web_trash);
    if(PGC[sel_trash] == web_trash){ //문법확인 분리수거 일치할때 *********
        all_state = 2; // 쓰레기 일치하면 상태 2로 변경
      matrix.clear();
      matrix.setCursor(1,1);
      matrix.drawBitmap(0,0,initial[1], 8,9, 128); //웃음
        web_trash = '0';
        ledcWrite(0, 255);
     ledcWriteTone(0, 1046); //딩동댕 출력
     delay(100);
     ledcWriteTone(0, 1318);
     delay(100);
     ledcWriteTone(0, 1567);
     delay(100);
     ledcWriteTone(0, 0);
     matrix.clear();
      matrix.setCursor(1,1);
      matrix.drawBitmap(0,0,initial[0], 8,9, 128); //무표정
    }
    else if(web_trash != '0' && web_trash != '1' && web_trash != NULL && web_trash != PGC[sel_trash]){ //분리수거 일치하지 않을때 부저울리고 화난 표정 
      all_state = 0;
      matrix.clear();
      matrix.setCursor(1,1);
      matrix.drawBitmap(0,0,initial[2], 8,9, 128); //화남
      ledcWrite(0, 255);
     ledcWriteTone(0, 1479);
     delay(200);
     ledcWriteTone(0, 0);
    delay(1000);
      web_trash = '0';
    }
}


/*-------------------------------------------------------------------------------------------all_state == 2 */
if(all_state == 2){ //쓰레기통이 열린상태 -> 쓰레기 넣기
  digitalWrite(15, HIGH); //쓰레기통 열림
        if(analogRead(_A0_LIGHT_ADC_GPIO) < 1500){ //쓰레기 감지 안되면
            
            shadow_val = LOW; //LED의 입력값 저장
            Serial.println("led val");
            Serial.println(shadow_val);
            Serial.println("led old_val");
            Serial.println(shadow_old_val);
            shadow_old_val = shadow_val;    //old_val에 val값 저장(버튼 눌림 감지는 매우 빠르게 일어나기 때문에 이 값은 거의 LOW로 항상 저장된다.)
        }
        else if(analogRead(_A0_LIGHT_ADC_GPIO) > 1500){   //쓰레기 감지= led 켜짐//수정필요
            R = analogRead(_A1_R_GPIO);  //무게 저장

          /*--------------------new yj-----count trash-----------------------------*/
            shadow_val = HIGH; //LED의 입력값 저장
            Serial.println("trash_led val");
            Serial.println(shadow_val);
            Serial.println("trash_led old_val");
            Serial.println(shadow_old_val);
           // delay(100);       //LED 입력 감지를 위한 시간 여유
            
            if((shadow_val == HIGH) && (shadow_old_val == LOW)){  //LED켜지면 쓰레기 들어간것임, 이전값이 LOW일 때(즉, 버튼이 눌렸을 때)
                Serial.println("pushed_success");

                all_state = 0; // 상태원상복구 -> p,g,c선택화면
                float R_float = 0.0;
                char R_String[8];
                R_float = R/100.0000;
                R_all = R_all + R_float;
                dtostrf(R_all, 6, 4, R_String);
                delay(1000);
                flag_e = 1;
                client.publish("/component/ef2e5862-8b1a-45f0-b221-b4ebab7083d8/pub", R_String); //string 무게
                delay(1000);
                flag_e = 1;
                digitalWrite(_D2_LED_GPIO, HIGH);
                delay(100);
                digitalWrite(_D2_LED_GPIO, LOW);
                delay(100);
                digitalWrite(_D2_LED_GPIO, HIGH);
                delay(100);
                digitalWrite(_D2_LED_GPIO, LOW);
                delay(100);
                digitalWrite(_D2_LED_GPIO, HIGH);
                delay(100);
                digitalWrite(_D2_LED_GPIO, LOW);
                delay(100);
                //Preprocessing();
                flag_e = 1;
            }
            shadow_old_val = shadow_val;    //old_val에 val값 저장(버튼 눌림 감지는 매우 빠르게 일어나기 때문에 이 값은 거의 LOW로 항상 저장된다.)
        }
  }



delay(300);
}