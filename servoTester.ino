#include <LiquidCrystal.h>
#include <Servo.h>

LiquidCrystal lcd(1, 0, 12, 13, 5, 11);

int pulseWidthAr[] = {500, 544, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2100, 2200, 2300, 2400, 2500, 2600};
int pulseWidthArSize;
int pulseWidthMinIndex = 1;
int pulseWidthMaxIndex = 20;

int tactPinL = 6;
int togglePinL = 16;
int tactPinR = 7;
int togglePinR = 17;
int tactPinC = 8;
int potenPinL = 0;
int potenPinR = 1;
int servoPinL = 9;
int servoPinR = 10;

int mode = 0; //0:通常, 1:MS指定, 2:パルス幅下限, 3:パルス幅上限
boolean lastTactC = false;
boolean lastTactL = false;
boolean lastTactR = false;

int lastL = 0;
int lastR = 0;
int fixL = 0; //0:固定しない, 1:固定するがツマミを操作されたら固定解除, 2:固定するがトグルスイッチを操作されたら固定解除
int fixR = 0;

int lastMSL = 0;
int lastMSR = 0;

Servo servoL;
Servo servoR;

int cnt = 0;

String padding4(int val){
  String str = String(val);
  
  if(10 > val){
    str = "   " + str;
  } else if (100 > val){
    str = "  " + str;
  } else if (1000 > val){
    str = " " + str;
  }

  return str;
}

void lcdShowValue(int val, int line, int charCode, String caption)
{
  String str = padding4(val);
  
  lcd.setCursor(0, line);
  lcd.print( caption );
  lcd.print( str );
  lcd.write( charCode );
  lcd.write( 32 );
}

void lcdShowMS(int val, int line, String caption)
{
  String str = padding4(val);
  
  lcd.setCursor(0, line);
  lcd.print( caption );
  lcd.print( str );
}

void lcdShowRange(int minValIndex, int maxValIndex, int current)
{
  int minVal = pulseWidthAr[minValIndex];
  int maxVal = pulseWidthAr[maxValIndex];

  String str = padding4(minVal);
  
  lcd.setCursor(0, 0);
  lcd.print("Min");
  if (current == 0){
    lcd.print("*");
  } else {
    lcd.print(" ");
  }
  lcd.print(str);

  str = padding4(maxVal);
  
  lcd.setCursor(0, 1);
  lcd.print("Max");
  if (current == 1){
    lcd.print("*");
  } else {
    lcd.print(" ");
  }
  lcd.print(str);
}

void changeRange(int addMin, int addMax){
  pulseWidthMinIndex = pulseWidthMinIndex + addMin;
  pulseWidthMaxIndex = pulseWidthMaxIndex + addMax;
  
  if (pulseWidthAr[pulseWidthMinIndex] >= 1052) {
    pulseWidthMinIndex = pulseWidthMinIndex - 1;
  }
  if (pulseWidthMinIndex < 0) {
   pulseWidthMinIndex = 0;
  }
  if (pulseWidthAr[pulseWidthMaxIndex] <= 1892) {
    pulseWidthMaxIndex = pulseWidthMaxIndex + 1;
  }
  if (pulseWidthMaxIndex >= pulseWidthArSize ) {
    pulseWidthMaxIndex = pulseWidthArSize - 1;
  }

  servoL.detach();
  servoR.detach();
  servoL.attach(servoPinL, pulseWidthAr[pulseWidthMinIndex], pulseWidthAr[pulseWidthMaxIndex]);
  servoR.attach(servoPinR, pulseWidthAr[pulseWidthMinIndex], pulseWidthAr[pulseWidthMaxIndex]);
}

int readRadian(int pin)
{
  int r = analogRead(pin);
  r = constrain(r, 0, 1023);
  return map(r, 0, 1023, 0, 180);
}

int readMS(int pin)
{
  int r = analogRead(pin);
  r = constrain(r, 0, 1023);
  return map(r, 0, 1023, pulseWidthAr[0], pulseWidthAr[pulseWidthArSize - 1]);
}


void setup() {
  pinMode(tactPinL, INPUT_PULLUP);
  pinMode(togglePinL, INPUT_PULLUP);
  pinMode(tactPinR, INPUT_PULLUP);
  pinMode(togglePinR, INPUT_PULLUP);
  pinMode(tactPinC, INPUT_PULLUP);

  pulseWidthArSize = sizeof(pulseWidthAr) / sizeof(pulseWidthAr[0]);
  
  lcd.begin(8, 2);
  servoL.write(90);
  servoR.write(90);
  servoL.attach(servoPinL);
  servoR.attach(servoPinR);
}

void loop() {

  //モード切替
  if (digitalRead(tactPinC) == LOW && lastTactC == false ){
    delay(20);
    mode = mode + 1;
    if (mode == 4){
      mode = 0;
    } else {
      servoL.write(180 * (mode - 2));
      servoR.write(180 * (mode - 2));
      fixL = 0; 
      fixR = 0; 
    }
    //選択中モード表示
    lcd.clear();
    switch (mode){
      case 0:
        lcd.print("Angle");
        lastL = -1;
        lastR = -1;
        break;
      case 1:
        lcd.print("MicroSec");
        lastMSL = -1;
        lastMSR = -1;
        break;
      case 2:
        lcd.print("Min");
        servoL.write(0);
        servoR.write(0);
        break;
      case 3:
        lcd.print("Max");
        servoL.write(180);
        servoR.write(180);
        break;
    }
    delay(500);
    lcd.clear();
    if (mode == 2 || mode == 3){      
        lcdShowRange(pulseWidthMinIndex, pulseWidthMaxIndex, mode - 2);
    }
    lastTactC == true;
  } else if (digitalRead(tactPinC) == HIGH){
    lastTactC == false;
  }

  //テストモード
  if (mode == 0 ){
    //サーボL
    if (digitalRead(tactPinL) == LOW){
      servoL.write(90);
      lcdShowValue(90, 0, 124, "L");
      fixL = 1;
      
    } else if (digitalRead(togglePinL) == HIGH){
          
      int r = readRadian(potenPinL);
      if(fixL == 2){
        fixL = 0;
      }
  
      if ((fixL == 0 || fixL == 1) && (abs(lastL - r) > 2 || r == 0 || r == 180)){
        servoL.write(r);
        lcdShowValue(r, 0, 32, "L");  
      
        lastL = r;
        fixL = 0;
      }
  
    } else {
      lcd.setCursor(6, 0);
      lcd.print("*");
      fixL = 2;
    }
    
    //サーボR
    if (digitalRead(tactPinR) == LOW){
      servoR.write(90);
      lcdShowValue(90, 1, 124, "R");
      fixR = 1;
      
    } else if (digitalRead(togglePinR) == HIGH){
      
      int r = readRadian(potenPinR);
      if(fixR == 2){
        fixR = 0;
      }
  
      if ((fixR == 0 || fixR == 1) && (abs(lastR - r) > 2 || r == 0 || r == 180)){
        servoR.write(r);
        lcdShowValue(r, 1, 32, "R");  
      
        lastR = r;
        fixR = 0;
        
      } 
  
    } else {
      lcd.setCursor(6, 1);
      lcd.print("*");
      fixR = 2;
    }

  } else if (mode == 1) {
    
      int msL = readMS(potenPinL);
      if (abs(lastMSL - msL) > 10 || msL == pulseWidthAr[0] || msL == pulseWidthAr[pulseWidthArSize - 1]){
        servoL.writeMicroseconds(msL);
        lcdShowMS(msL, 0, "ms-L");
      }
      
      int msR = readMS(potenPinR);
      if (abs(lastMSR - msR) > 10 || msR == pulseWidthAr[0] || msR == pulseWidthAr[pulseWidthArSize - 1]){
        servoR.writeMicroseconds(msR);
        lcdShowMS(msR, 1, "ms-R");
      }
      
  } else {
    //パルス幅 範囲設定モード
    //減らす
    if (digitalRead(tactPinL) == LOW && lastTactL == false ){
      delay(50);
      if (mode == 2){
        changeRange(-1, 0);
        servoL.write(0);
        servoR.write(0);
      } else {
        changeRange(0, -1);
        servoL.write(180);
        servoR.write(180);
      }
      lcdShowRange(pulseWidthMinIndex, pulseWidthMaxIndex, mode - 2);
      lastTactL = true;
    } else if (digitalRead(tactPinL) == HIGH){
      lastTactL = false;
    }

    //増やす
    if (digitalRead(tactPinR) == LOW && lastTactR == false ){
      delay(50);
      if (mode == 2){
        changeRange(1, 0);
        servoL.write(0);
        servoR.write(0);
      } else {
        changeRange(0, 1);
        servoL.write(180);
        servoR.write(180);
      }
      lcdShowRange(pulseWidthMinIndex, pulseWidthMaxIndex, mode - 2);
      lastTactR = true;
    } else if (digitalRead(tactPinR) == HIGH){
      lastTactR = false;
    }
  }
}

