#include <Wire.h>
#include <avr/wdt.h>
#include <arduino-timer.h>
#include <EEPROM.h>
#include <max6675.h>
#include <DallasTemperature.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SI2C.h>
#define ONE_WIRE_BUS 5
#define sel digitalRead(3) == 0
#define up digitalRead(4) == 0
#define down digitalRead(11) == 0
const int flameSensor = A0;
const int voltageSensor = A2;
const int motorAir = 4;
const int beginResistor = 2;
const int motorPellet = 6;
const int onOffTimmers = 7;
const int buzz = 13;
int ktcSO = 8;
int ktcCS = 9;
int ktcCLK = 10;
int volt = 0;
int status = 0;
int endTime = 2;
int cleanTime = 2;
int time = 0;
int timeForClose = 0;
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
float fireValue = 0;
float exhaustValue = 0;
float waterValue = 0;
float firePerCent;
unsigned long previousMillis[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  //number of onOfftimers
unsigned long previousTime = 0;
unsigned long pelletInDelayLastTime = 0;
unsigned long pelletInDelay = 10000;
unsigned long preTime = 0;
bool state[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
bool open = false;
bool manual = false;
bool menu = false;
bool error = false;
bool pellet = false;
bool MAX = true;
bool menuOn = true;
bool endOperation = false;
bool cleaned = false;

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  relay(0, 1, 1, 1);
  pinMode(motorAir, OUTPUT);
  pinMode(beginResistor, OUTPUT);
  pinMode(motorPellet, OUTPUT);
  pinMode(onOffTimmers, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(up, INPUT);
  pinMode(sel, INPUT);
  pinMode(down, INPUT);
  lcd.begin(16, 2);
  sensors.begin();
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  wdt_enable(WDTO_2S);
  // while (EEPROM.read(0) == 0) {  //need reset button
  //   errorOn();
  // }
}

void loop() {
  collectDataFromSensors();
  if (open) {
    mainMode();
  } else {
    closed();
  }
}

void mainMode() {
  if (!pellet && (exhaustValue < 35 || fireValue > 900)) {  // ktc.readCelsius() <= 25
    pelletIn();
  } else {
    started();
  }
}


void started() {
  if ((fireValue >= 500) && exhaustValue <= 25) {  // ktc.readCelsius() <= 25
    preFlameOperation();
  } else {
    unsigned long currentTime = millis();
    if (millis() - previousMillis[5] >= 1000) {  //i = 5
      time++;
      while (fireValue > 900) {
        errorOn();
      }
      if (exhaustValue >= 0 && exhaustValue <= 30) {  //ktc.readCelsius() >= 0 && ktc.readCelsius() <= 30
        lowFire();
      } else if (exhaustValue > 30 && exhaustValue < 65) {  //(ktc.readCelsius() > 30 && ktc.readCelsius() < 65)
        warmingUp();
      } else if (exhaustValue >= 65) {  //ktc.readCelsius() < 65
        mainWorking();
      }
      previousMillis[5] = currentTime;
      wdt_reset();
    }
  }
}

void lowFire() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("XAMILI ENAUSI");
  lcd.setCursor(0, 1);
  lcd.print("KAYSAERIA ");
  lcd.print(exhaustValue);  //ktc.readCelsius()
  lcd.print(" C");
  relay(0, 0, 1, 1);
}

void warmingUp() {
  time = 0;
  pellet = false;
  digitalWrite(onOffTimmers, HIGH);
  digitalWrite(motorAir, LOW);
  digitalWrite(beginResistor, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ZESTAMA ");
  lcd.setCursor(0, 1);
  lcd.print("KAYSAERIA ");
  lcd.print(exhaustValue);  //ktc.readCelsius()
  lcd.print(" C");
  endOperation = true;
}

void warmingUpTimer() {
  unsigned long currentMillis = millis();
  int i = 1;
  int onTime = 3000;
  int offTime = 10000;
  if ((state[i] == HIGH) && (currentMillis - previousMillis[i] >= onTime)) {
    state[i] = LOW;
    previousMillis[i] = currentMillis;
    digitalWrite(motorPellet, HIGH);
  } else if ((state[i] == LOW) && (currentMillis - previousMillis[i] >= offTime)) {
    state[i] = HIGH;
    previousMillis[i] = currentMillis;
    digitalWrite(motorPellet, LOW);
  }
}

void mainWorking() {
  if (!manual) {
    digitalWrite(onOffTimmers, HIGH);
    digitalWrite(motorAir, LOW);
    digitalWrite(beginResistor, HIGH);
    if (waterValue <= 51 && MAX == true) {
      highTimer();
    } else if (waterValue >= 45 && waterValue < 51) {
      midTimer();
      if (waterValue < 45) {
        MAX = true;
      }
    } else {
      lowTimer();
      MAX = false;
      if (waterValue < 45) {
        MAX = true;
      }
    }
  } else if (manual) {
    relay(0, 1, 1, 0);
  }
  endOperation = true;
  showDataOnLcd();
  wdt_reset();
}


void highTimer() {
  unsigned long currentMillis = millis();
  int i = 2;
  int onTime = 3000;
  int offTime = 10000;
  if ((state[i] == HIGH) && (currentMillis - previousMillis[i] >= onTime)) {
    state[i] = LOW;
    previousMillis[i] = currentMillis;
    digitalWrite(motorPellet, HIGH);
  } else if ((state[i] == LOW) && (currentMillis - previousMillis[i] >= offTime)) {
    state[i] = HIGH;
    previousMillis[i] = currentMillis;
    digitalWrite(motorPellet, LOW);
  }
}

void midTimer() {
  unsigned long currentMillis = millis();
  int i = 3;
  if ((state[i] == HIGH) && (currentMillis - previousMillis[i] >= 3000)) {
    state[i] = LOW;
    previousMillis[i] = currentMillis;
    digitalWrite(motorPellet, HIGH);
  } else if ((state[i] == LOW) && (currentMillis - previousMillis[i] >= 10000)) {
    state[i] = HIGH;
    previousMillis[i] = currentMillis;
    digitalWrite(motorPellet, LOW);
  }
}

void lowTimer() {
  int i = 4;
  unsigned long currentMillis = millis();
  if ((state[i] == HIGH) && (currentMillis - previousMillis[i] >= 3000)) {
    state[i] = LOW;
    previousMillis[i] = currentMillis;
    digitalWrite(motorPellet, HIGH);
  } else if ((state[i] == LOW) && (currentMillis - previousMillis[i] >= 10000)) {
    state[i] = HIGH;
    previousMillis[i] = currentMillis;
    digitalWrite(motorPellet, LOW);
  }
}

void showDataOnLcd() {
  unsigned long currentTime = millis();
  firePerCent = fireValue / 1023;
  firePerCent = 1 - firePerCent;
  firePerCent = firePerCent * 100;
  String operation;
  if (!manual) {
    operation = "KANONIKH LEIT. A";
  } else {
    operation = "KANONIKH LEIT. M";
  }
  if (currentTime - previousMillis[6] >= 1000) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(operation);
    lcd.setCursor(0, 1);
    lcd.print("FOTIA");
    lcd.setCursor(6, 1);
    lcd.print(firePerCent);
    lcd.print("%");
    previousMillis[6] = currentTime;  // i=6
  }
  if (currentTime - previousMillis[7] >= 2000) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(operation);
    lcd.setCursor(0, 1);
    lcd.print("KAYSAERIA");
    lcd.setCursor(11, 1);
    lcd.print(exhaustValue);
    previousMillis[7] = currentTime;  // i=7
  }
  if (currentTime - previousMillis[8] >= 3000) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(operation);
    lcd.setCursor(0, 1);
    lcd.print("NERO");
    lcd.setCursor(11, 1);
    lcd.print(waterValue);
    previousMillis[8] = currentTime;  // i=8
  }
}

void preFlameOperation() {
  pellet = true;
  endOperation = false;
  unsigned long currentTime = millis();
  if (millis() - previousTime >= 1000) {
    time++;
    if (time >= 14 && time <= 15) {
      relay(0, 0, 1, 1);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("AERAS ANOIXTOS 1");
    } else if (time >= 18 && time <= 20) {
      relay(0, 0, 1, 1);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("AERAS ANOIXTOS 2");
    } else if (time >= 25 && time <= 30) {
      relay(0, 0, 1, 1);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("AERAS ANOIXTOS");
    } else if (time > 40) {
      relay(1, 1, 1, 1);
      errorOn();
    } else {
      relay(1, 0, 1, 1);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ANAFLEKSI ");
      lcd.print(time);
      lcd.print(" sec");
      lcd.setCursor(0, 1);
      lcd.print("FOTIA ");
      lcd.print(fireValue);
    }
    previousTime = currentTime;
    wdt_reset();
  }
}

void autoManual() {
  if (digitalRead(12)) {
    manual = true;
  } else {
    manual = false;
  }
}

void errorOn() {
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("PROVLIMA");  //ERROR
  digitalWrite(buzz, HIGH);
  delay(500);
  digitalWrite(buzz, LOW);
  delay(500);
}

void pelletIn() {
  while (endTime >= 0) {
    relay(1, 1, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("EISAGWGI PELLET");
    lcd.setCursor(0, 1);
    lcd.print(endTime);
    lcd.print(" sec");
    endTime--;
    EEPROM.write(0, 0);
    delay(1000);
    wdt_reset();
  }
  pellet = true;
  EEPROM.write(0, 1);
}

void closed() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis[9] >= 1000) {
    if (endOperation) {
      if (!cleaned) {
        clean();
        cleaned = true;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("KLEISTO");
        lcd.setCursor(0, 1);
        lcd.print(exhaustValue);
        relay(1, 1, 1, 1);
      }
    } else {
      if (timeForClose < 5) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("KLEISIMO");
        lcd.setCursor(9, 0);
        lcd.print(timeForClose);
        lcd.setCursor(0, 1);
        lcd.print(exhaustValue);
        relay(0, 1, 1, 1);
      } else {
        endOperation = true;
        cleaned = true;
      }
      timeForClose++;
    }
    wdt_reset();
    previousMillis[9] = currentMillis;
  }
}

void clean() {
  while (cleanTime >= 0) {
    relay(0, 1, 1, 1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("KATHARISMOS");
    lcd.setCursor(0, 1);
    lcd.print(cleanTime);
    lcd.print(" sec");
    cleanTime--;
    EEPROM.write(0, 0);
    delay(1000);
    wdt_reset();
  }
  EEPROM.write(0, 1);
}

void menuMode() {
}

void collectDataFromSensors() {
  unsigned long currentTime = millis();
  if (millis() - preTime >= 1000) {
    sensors.requestTemperatures();
    waterValue = analogRead(A3);    // sensors.getTempCByIndex(0);
    exhaustValue = analogRead(A1);  // tha mpei o  aisthtiras kausaeriwn
    fireValue = analogRead(flameSensor);
    volt = analogRead(voltageSensor);
    vOUT = (volt * 5.0) / 1024.0;
    vIN = vOUT / (R2 / (R1 + R2));
    if (vIN > 12) {
      open = true;
    } else {
      open = false;
    }
    preTime = currentTime;
    autoManual();
    wdt_reset();
  }
}

void relay(bool air, bool resist, bool motorpellet, bool onofftimers) {
  digitalWrite(motorAir, air);
  digitalWrite(beginResistor, resist);
  digitalWrite(motorPellet, motorpellet);
  digitalWrite(onOffTimmers, onofftimers);
}

void receiveEvent(int howMany) {
  // while (0 < Wire.available()) {
  //   char c = Wire.read();
  //   Serial.print(c);
  // }
  // Serial.println();
}

void requestEvent() {
  sendDataToMaster(waterValue, fireValue, exhaustValue, status);
}

void sendDataToMaster(int valueW, int valueF, int valueE, int valueS) {
  int valueOfIntW = round(valueW);
  int valueOfIntF = round(valueF);
  int valueOfIntE = round(valueE);
  byte highW = highByte(valueOfIntW);
  byte lowW = lowByte(valueOfIntW);
  byte highF = highByte(valueOfIntF);
  byte lowF = lowByte(valueOfIntF);
  byte highE = highByte(valueOfIntE);
  byte lowE = lowByte(valueOfIntE);
  byte buffer[12];
  buffer[0] = highW;
  buffer[1] = lowW;
  buffer[2] = "w";
  buffer[3] = highF;
  buffer[4] = lowF;
  buffer[5] = "f";
  buffer[6] = highE;
  buffer[7] = lowE;
  buffer[8] = "e";
  buffer[9] = valueS;
  buffer[10] = "s";
  buffer[11] = 1;
  Wire.write(buffer, 12);
}