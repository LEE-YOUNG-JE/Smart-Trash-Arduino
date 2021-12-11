#include <WiFi.h>
#include <PubSubClient.h>
#define HICLOUD_API_KEY "2c6edde6db100ad83a509a6cea13a75ef97c3826"
const char* ssid = "LAPTOPYJ";
const char* password = "01034801271";
const char* mqtt_server = "202.30.11.115";
float val = 0;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define Button_GPIO 2       //I/O 큐브(버튼 모듈)에 연결한 디지털 핀
/*------------------------------*/
#include <Adafruit_GFX.h> 
#include <Adafruit_IS31FL3731.h>


#define _D0_BUTTON_GPIO 2          //BUTTON
#define _D2_LED_GPIO 26            //LED
#define _A0_LIGHT_ADC_GPIO 36      //조도센서
#define _A1_R_GPIO 39              //가변저항
#define _TOUCH_LEFT 27             //왼쪽 터치센서
#define _TOUCH_RIGHT 4             //오른쪽 터치센서
#define plastic 0
#define can 1
#define glass 2
/*-------------------------------------------wifi function---------------------------*/
int all_state = 0; //0이면 처음 쓰레기 선택모드상태, 1이면 버튼 눌린상태 = 쓰레기 인식대기상태, 2이면 hicloud로 데이터 보내는 상태
int pa = 0; //amount of plastic amount
int ga = 0; //amount of glass
int ca = 0; //amount of can 
float R=0;
int light=0;  //조도 값
int sel_trash=0;
int PGC[3] = {'P', 'G', 'C'};
char choose_trash = '0';
char web_trash = '0';
int button_status=0;
int old_val = 0;
int state = 0;    
int shadow_val = 1;
int shadow_old_val = 1;
int led_state = 0;            //사용할 변수 선언
/*---------------------------------------------------------------------------*/
void Preprocessing(){
  if(sel_trash == 0) pa+=R;     //여기서 카운트도 바로 할 수 있을듯
  else if(sel_trash == 2) ca+=R ;   //R은 현재 찍히고 있는 쓰레기의 무게
  else if(sel_trash == 1) ga+=R;
  char CO2String[8];
  char saving_energyString[8];
  char EPRString[8];
  float CO2 = 12;   //좀더 찾아봐야될듯
  float saving_energy = 24;  // 좀더...
  float EPR = (1000*0.807-(float)ca)*134*1.3425*(1.15*134*(1000*0.807-(float)ca)+1) +  
              (1000*0.774-(float)pa)*172*1.3425*(1.15*172*(1000*0.774-(float)pa)+1) +
              (1000*0.711-(float)ga)*36*1.3425*(1.15*36*(1000*0.711-(float)ga)+1);//1톤 기준 재활용을 안하면 내야되는 벌금같은 거





  dtostrf(R, 1, 2, CO2String);
  dtostrf(saving_energy, 1, 2, saving_energyString);
  dtostrf(EPR, 1, 2, EPRString);
  client.publish("/component/b7d4ddb2-2e79-4a87-abde-9f31406bce7e/pub", CO2String); //cow
  delay(3000);
  client.publish("/component/3d532a74-ef24-4851-9d1b-2872708b489a/pub", saving_energyString); //saving_energy
  delay(3000);
  //client.publish("/component/b7057685-2240-45e3-b9dc-96d55d29ab75/pub", EPRString); //EPR
}
/*-------------------------------------------wifi function end--------------------------*/


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
 

void setup() {
  setup_wifi();
client.setServer(mqtt_server, 1883);
/*------------------sensor setup---------------------*/  
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
void loop() {
   if (!client.connected()) {
  reconnect();
  }
  client.loop();
  
  matrix.setTextSize(1);
  matrix.setTextColor(100);
    /*---------------------터치센서를 이용하여 led 메트릭스 제어-----------------------------------------------------------loop-------------*/
  if(all_state == 0){ //쓰레기 선택모드
      digitalWrite(15, LOW);
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


/*--------------------------------------------------------------------------------------------------------------button_status == 0 && button눌렸을때*/
 if(!digitalRead(_D0_BUTTON_GPIO) && all_state == 0) all_state = 1;  //상태 변화

/*button status ==1일때 무표정코드 추가해야됨 elseif로 밑에추가*/
/*---------------------------------------------------111111111111111111111111111111111---------------------------------------button_status == 1*/
 if(all_state == 1) { //조건이 node-red webcam에서 인식한 것과 같을 때로 바꿔야됨, button_status ==1일때는 인식하기 직전이므로 무표정 아래 표정도 웃는 표정으로 바꾸고 무표정은 따로옮기기
     matrix.clear();
     matrix.setCursor(1,1);
     matrix.drawBitmap(0,0,initial[0], 8,9, 128); //
 /*------------------sensor works------------------------22222222222222222222222222222222222---------------쓰레기 감지 안될때--*/
    char temp = Serial.read();
    if(temp == '0' || temp == '1' || temp =='P' || temp =='G' || temp =='C') web_trash = temp;
    Serial.println(web_trash);
    if(PGC[sel_trash] == web_trash){ //문법확인 분리수거 일치할때 *********
        all_state = 2;
        web_trash = '0';
    }
    else if(web_trash != '0' && web_trash != '1' && web_trash != NULL && web_trash != PGC[sel_trash]){ //분리수거 일치하지 않을때 부저울리고 표정 
      all_state = 0;
      matrix.clear();
      matrix.setCursor(1,1);
      matrix.drawBitmap(0,0,initial[2], 8,9, 128); //화남
      delay(1500);
      web_trash = '0';
    }
}


/*-------------------------------------------------------------------------------------------all_state == 2 */
if(all_state == 2){
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
                
                delay(100);
                
                if(sel_trash == 0) pa += 1; //new
                else if(sel_trash == 1) ga += 1; //new
                else if(sel_trash == 2) ca += 1; //new
      
                Serial.println("trash_count : P, G, C");
                Serial.println(pa);
                Serial.println(ga);
                Serial.println(ca);
                all_state = 0; // 상태원상복구 -> p,g,c선택화면
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
                Preprocessing();
            }
            shadow_old_val = shadow_val;    //old_val에 val값 저장(버튼 눌림 감지는 매우 빠르게 일어나기 때문에 이 값은 거의 LOW로 항상 저장된다.)
        }
}

delay(500);
}