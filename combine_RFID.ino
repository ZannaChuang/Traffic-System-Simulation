#include <analogWrite.h>
#include<WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <string.h>
const char ssid[] = "xxxx"; //修改WiFi網路名稱
const char pwd[] = "xxxxx"; //修改WiFi密碼
String url = "https://api.thingspeak.com/update?api_key=xxxxxxxx";//key 要自己換

// 變燈
#define fsr_pin_L 34
#define fsr_pin_LL 35
#define fsr_pin_R 36
#define fsr_pin_RR 39

int fsr_value_L;
int fsr_value_R;

//L=2 
int Red_L = 26;
int Yellow_L = 17; //////
int Green_L = 16;

//R=1
int Red_R = 14; //
int Yellow_R = 0;
int Green_R = 32;

int G = 0;
int N = 0;

//車流
#define OR1Pin 15
#define OL1Pin 22
#define OR2Pin 13
#define OL2Pin 21
int OR1 = HIGH;  // HIGH 表示沒檔
int OL1 = HIGH;
int OR2 = HIGH;
int OL2 = HIGH;
int carR1 = 0; //車子數量
int carL1 = 0;
int carR2 = 0;
int carL2 = 0;
int c_L = 0;
int c_H = 0;
int c_L2 = 0;
int c_H2 = 0;

// RFID
#include <SPI.h>
#include <MFRC522.h>
const int sendPin = 2; //發送方GPIO腳位
const int responsePin = 33; // 回應方GPIO脚位
#define SS_PIN1 5
#define RST_PIN1 27
MFRC522 mfrc522_1(SS_PIN1, RST_PIN1); // Create MFRC522 instance for first RFID reader
bool isCardDetected1 = false; // flag to indicate if card is detected by the first RFID reader




//上傳
void UploadData() {

  //建立一個網頁
  HTTPClient http;

  //把數值寫入網址
  String url1 = url + "&field1=" + (int)carR1 + "&field2=" + (int)carR2 + "&field3=" + (int)carL1 + "&field4=" + (int)carL2 ;

  //提交網址
  http.begin(url1);

  //讀取網頁內容
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.print("網頁內容=");
    Serial.println(payload);
  }
  else {
    Serial.println("網路傳送失敗");
  }

  //關閉網頁
  http.end();
}


void setup() {

  //變燈
  Serial.begin(9600);

  pinMode(Red_L, OUTPUT);
  pinMode(Green_L, OUTPUT);
  pinMode(Yellow_L, OUTPUT);
  digitalWrite(Red_L, LOW);
  digitalWrite(Green_L, LOW);
  digitalWrite(Yellow_L, LOW);

  pinMode(Red_R, OUTPUT);
  pinMode(Green_R, OUTPUT);
  pinMode(Yellow_R, OUTPUT);
  digitalWrite(Red_R, LOW);
  digitalWrite(Green_R, LOW);
  digitalWrite(Yellow_R, LOW);

  //RFID
  pinMode(sendPin, OUTPUT);
  pinMode(responsePin, INPUT);
  Serial.begin(9600);
  SPI.begin();        // Initialize SPI bus
  mfrc522_1.PCD_DumpVersionToSerial(); // 顯示讀卡設備的版本
  mfrc522_1.PCD_Init(); // Initialize first RFID reader


  //上傳
  Serial.begin(9600);
  WiFi.mode(WIFI_STA); //設置WiFi模式
  WiFi.begin(ssid, pwd);

  Serial.print("WiFi connecting");

  //當WiFi連線時會回傳WL_CONNECTED，因此跳出迴圈時代表已成功連線
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP位址:");
  Serial.println(WiFi.localIP()); //讀取IP位址
  Serial.print("WiFi RSSI:");
  Serial.println(WiFi.RSSI()); //讀取WiFi強度

}

void loop() {

  G = 0;
  N = 0;
  c_L = 0;
  c_H = 0;
  c_L2 = 0;
  c_H2 = 0;
  digitalWrite(Green_L, LOW);
  digitalWrite(Yellow_L, HIGH);
  delay(2000);
  digitalWrite(Yellow_L, LOW);
  digitalWrite(Red_L, HIGH);
  UploadData();
  digitalWrite(Red_R, LOW);
  digitalWrite(Green_R, HIGH);
  //上傳
  
  carR1 = 0;
  carL1 = 0;
  carR2 = 0;
  carL2 = 0;

  for (G = 0; G <= 18; G++)
  {
    if (mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial())
    {
      RFID1();
      G = 0;
      N = 0;
      c_L = 0;
      c_H = 0;
      c_L2 = 0;
      c_H2 = 0;
      digitalWrite(Green_L, LOW);
      digitalWrite(Yellow_L, HIGH);
      delay(2000);
      digitalWrite(Yellow_L, LOW);
      digitalWrite(Red_L, HIGH);
      delay(1000);
      digitalWrite(Red_R, LOW);
      digitalWrite(Green_R, HIGH);

      continue;
    }


    fsr_value_L = analogRead(fsr_pin_L) + analogRead(fsr_pin_LL); // 讀取左FSR //多加了一快對面逆向同燈號的壓力板
    fsr_value_R = analogRead(fsr_pin_R) + analogRead(fsr_pin_RR); // 讀取右FSR
    Serial.print("fsr_value_R:");
    Serial.println(fsr_value_L);
    Serial.print("fsr_value_L:");
    Serial.println(fsr_value_R);
    Serial.print("特殊秒:");
    Serial.println(G);
    Serial.print("正常秒:");
    Serial.println(N);
    Serial.println("-------------");

    if (fsr_value_L > 100 && fsr_value_R < 100)
    {
      N++;//不正常秒G持續增加
    }
    else
    {
      G = 0;
      N++;
    }

    if (N >= 90)
    {
      break;
    }

    //車流
    OL1 = digitalRead(OL1Pin);
    if (OL1 == LOW)
    {
      c_L++;
    }

    if (OL1 == HIGH)
    {
      c_H++;
    }

    if (c_L > 0 && c_H > 0)
    {
      carL1++;
      c_L = 0;
      c_H = 0;
    }
    c_H = 0;

    OL2 = digitalRead(OL2Pin);
    if (OL2 == LOW)
    {
      c_L2++;
    }

    if (OL2 == HIGH)
    {
      c_H2++;
    }

    if (c_L2 > 0 && c_H2 > 0)
    {
      carL2++;
      c_L2 = 0;
      c_H2 = 0;
    }
    c_H2 = 0;

    Serial.println("************");
    Serial.print("車子數量L1:");
    Serial.println(carL1);
    Serial.print("車子數量L2:");
    Serial.println(carL2);

    Serial.println("-------------");
    Serial.println("-------------");
    delay(1);

  }



  G = 0;
  N = 0;
  c_L = 0;
  c_H = 0;
  digitalWrite(Green_R, LOW);
  digitalWrite(Yellow_R, HIGH);
  delay(2000);
  digitalWrite(Yellow_R, LOW);
  digitalWrite(Red_R, HIGH);
  delay(3000);
  digitalWrite(Red_L, LOW);
  digitalWrite(Green_L, HIGH);

  for (G = 0; G <= 18; G++)
  {
    if (mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial())
    {
      RFID2();
      continue;
    }
    fsr_value_L = analogRead(fsr_pin_L) + analogRead(fsr_pin_LL); // 讀取左FSR //多加了一快對面逆向同燈號的壓力板
    fsr_value_R = analogRead(fsr_pin_R) + analogRead(fsr_pin_RR); // 讀取右FSR
    Serial.print("*****fsr_value_R:");
    Serial.println(fsr_value_L);
    Serial.print("*****fsr_value_L:");
    Serial.println(fsr_value_R);
    Serial.print("特殊秒:");
    Serial.println(G);
    Serial.print("正常秒:");
    Serial.println(N);
    Serial.println("-------------");
    if (fsr_value_R > 100 && fsr_value_L < 100)
    {
      N++;//不正常秒G持續增加
    }
    else
    {
      G = 0;
      N++;
    }
    if (N >= 90)
    {
      break;
    }


    OR1 = digitalRead(OR1Pin);
    if (OR1 == LOW)
    {
      c_L++;
    }
    if (OR1 == HIGH)
    {
      c_H++;
    }
    if (c_L > 0 && c_H > 0)
    {
      carR1++;
      c_L = 0;
      c_H = 0;
    }
    c_H = 0;


    OR2 = digitalRead(OR2Pin);
    if (OR2 == LOW)
    {
      c_L2++;
    }

    if (OR2 == HIGH)
    {
      c_H2++;
    }

    if (c_L2 > 0 && c_H2 > 0)
    {
      carR2++;
      c_L2 = 0;
      c_H2 = 0;
    }
    c_H2 = 0;
    Serial.println("************");
    Serial.print("車子數量R1:");
    Serial.println(carR1);
    Serial.print("車子數量R2:");
    Serial.println(carR2);

    Serial.println("-------------");
    Serial.println("-------------");
    delay(1);
  }

}

int RFID1()
{

  Serial.println("Card detected by Reader 1");
  isCardDetected1 = true;      // Set flag to indicate card detection
  digitalWrite(sendPin, HIGH); // 發送高訊號
  digitalWrite(Green_R, LOW);
  digitalWrite(Yellow_R, HIGH);
  delay(2000);
  digitalWrite(Yellow_R, LOW);
  digitalWrite(Red_R, HIGH);
  digitalWrite(Red_L, LOW);
  digitalWrite(Green_L, HIGH);
  delay(1000); // 等待
  digitalWrite(sendPin, LOW);
  for (int zz = 0; zz >= 0; zz++)
  {
    if (digitalRead(responsePin) == HIGH) { // 接收到高訊號
      Serial.println("access"); // 
      zz=0;
      break;
    } // 
  }
}
int RFID2()
{

  Serial.println("Card detected by Reader 1");
  isCardDetected1 = true;      // Set flag to indicate card detection
  digitalWrite(sendPin, HIGH); // 发送高电平信号
  delay(1000); // 等待一段时间
  digitalWrite(sendPin, LOW);
  for (int zz = 0; zz >= 0; zz++)
  {
    if (digitalRead(responsePin) == HIGH) { // 接收到高訊號
      Serial.println("access"); // 
      zz=0;
      break;
    } // 
  }
}


//delay 單位為毫秒 1s=1000ms
//150~=11.30s(25秒)
//30 ~= 2.85s(在這是5秒)
//6=1s
