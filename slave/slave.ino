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
#define sel digitalRead(3) == 1
#define up digitalRead(11) == 1
#define down digitalRead(4) == 1
const int flameSensor = A0;
const int beginResistor = 2;
const int motorAir = 1;
const int onOffTimmers = 7;
const int motorPellet = 12;  //allagi se 6
const int voltageSensor = A2;
const int buzz = 8;
int ktcSO = 8;
int ktcCS = 9;
int ktcCLK = 10;
int volt = 0;
int status = 0;
int endTime = 2;
int cleanTime = 2;
int time = 0;
int timeForClose = 0;
int count = 0;
int standBy[4] = { 0, 0, 0, 0 };
int throwing[4] = { 0, 0, 0, 0 };
int initialPellet = 0;
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
float fireValue = 0;
float exhaustValue = 0;
float waterValue = 0;
float firePerCent;
unsigned long previousMillis[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  //number of onOfftimers
unsigned long previousTime = 0;
unsigned long pelletInDelayLastTime = 0;
unsigned long pelletInDelay = 10000;
unsigned long preTime = 0;
bool state[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
bool open = false;
bool manual = false;
bool error = false;
bool pellet = false;
bool MAX = true;
bool menuOn = false;
bool endOperation = false;
bool cleaned = false;
char levelTwo = 1;
char levelThree = 200;
unsigned long timePress = 0;
unsigned long timePressLimit = 0;
int clicks = 0;

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
  // resetEppromReads();
  setUpTimesForTimers();
  while (!EEPROM.read(0)) {  //need reset button
    errorOn();
  }
}

void loop() {
  if (menuOn) {
    menuMode();
  } else {
    menu(true);
    collectDataFromSensors();
    if (open) {
      mainMode();
    } else {
      closed();
    }
  }
}

void menu(bool state) {
  unsigned long currentTime = millis();
  if (currentTime - previousMillis[10] >= 1000 && sel) {
    if (count > 1) {
      manual = true;
      menuOn = state;
      delay(100);
    }
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(count);
    count++;
    if (menuOn == state) { count = 0; }
    previousMillis[10] = currentTime;
  }
  wdt_reset();
}

void mainMode() {
  if (!EEPROM.read(15)) {
    if (sel) {
      EEPROM.write(15, 1);
    }
    menu(true);
    showDataOnLcd(true);
    collectDataFromSensors();
    relay(0, 0, 1, 0);
    wdt_reset();
  } else {
    if (!pellet && (exhaustValue < 35 || fireValue > 900)) {
      pelletIn();
    } else {
      started();
    }
  }
}


void started() {
  if (fireValue >= 500 && exhaustValue <= 25) {  // ktc.readCelsius() <= 25
    preFlameOperation();
  } else {
    unsigned long currentTime = millis();
    if (millis() - previousMillis[5] >= 1000) {
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
  pellet = false;
  relay(0, 0, 1, 1);
}

void warmingUp() {
  time = 0;
  pellet = false;
  digitalWrite(onOffTimmers, HIGH);
  digitalWrite(motorAir, LOW);
  digitalWrite(beginResistor, HIGH);
  warmingUpTimer();
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
  int onTime = throwing[3] * 1000;
  int offTime = standBy[3] * 1000;
  if ((state[i] == HIGH) && (currentMillis - previousMillis[i] >= offTime)) {
    state[i] = LOW;
    digitalWrite(motorPellet, LOW);
    previousMillis[i] = currentMillis;
  } else if ((state[i] == LOW) && (currentMillis - previousMillis[i] >= onTime)) {
    state[i] = HIGH;
    digitalWrite(motorPellet, HIGH);
    previousMillis[i] = currentMillis;
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
      if (waterValue < 45) { MAX = true; }
    } else {
      lowTimer();
      MAX = false;
      if (waterValue < 45) { MAX = true; }
    }
  } else if (manual) {
    relay(0, 1, 1, 0);
  }
  endOperation = true;
  showDataOnLcd(false);
  wdt_reset();
}


void highTimer() {
  unsigned long currentMillis = millis();
  int i = 2;
  int onTime = throwing[0] * 1000;
  int offTime = standBy[0] * 1000;
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
  int onTime = throwing[1] * 1000;
  int offTime = standBy[1] * 1000;
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

void lowTimer() {
  int i = 4;
  int onTime = throwing[2] * 1000;
  int offTime = standBy[2] * 1000;
  unsigned long currentMillis = millis();
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

void showDataOnLcd(bool open) {
  unsigned long currentTime = millis();
  firePerCent = fireValue / 1023;
  firePerCent = 1 - firePerCent;
  firePerCent = firePerCent * 100;
  String operation;
  if (open) {
    operation = "RELAY ANOIXTA !!!";
  } else {
    if (!manual) {
      if (exhaustValue > 300) {
        operation = "KA.LEIT. HIGH TEMP A";
      } else {
        operation = "KANONIKH LEIT.A";
      }
    } else {
      operation = "KANONIKH LEIT. M";
    }
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
    lcd.setCursor(6, 1);
    lcd.print(waterValue);
    previousMillis[8] = currentTime;  // i=8
  }
}

void preFlameOperation() {  //allagi xxronius
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
  if (digitalRead(13)) {
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
  if (sel) {
    EEPROM.write(0, 1);
  }
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
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis[12] >= 200) {
    updown();
    levelMenu();
    previousMillis[12] = currentMillis;
  }
  wdt_reset();
}

void selected(int level, int number) {
  while (sel) {
    clicks++;
    lcd.clear();
    lcd.print(clicks);
    delay(450);
  }
  if (clicks >= 1 && clicks < 3) {
    if (level == 3) {
      levelTwo = 100;
      levelThree = number;
    } else if (level == 2) {
      levelThree = 100;
      levelTwo = number;
    }
    count = 0;
    clicks = 0;
  } else if (clicks >= 3) {
    levelTwo = 100;
    menuOn = false;
    count = 0;
    manual = false;
    clicks = 0;
    while (1) {}
  } else {
    return;
  }
}

void menuLcd(const String& first, const String& second, int arrow) {
  lcd.setCursor(0, 0);
  lcd.print(first);
  lcd.setCursor(13, arrow);
  lcd.print("<-");
  lcd.setCursor(0, 1);
  lcd.print(second);
}

void resetEppromReads() {
  EEPROM.write(1, 12);
  EEPROM.write(2, 3);
  EEPROM.write(3, 15);
  EEPROM.write(4, 3);
  EEPROM.write(5, 15);
  EEPROM.write(6, 2);
  EEPROM.write(7, 2);  //pellet Init
  EEPROM.write(8, 20);
  EEPROM.write(9, 3);
}


void setUpTimesForTimers() {
  standBy[0] = EEPROM.read(1);
  standBy[1] = EEPROM.read(3);
  standBy[2] = EEPROM.read(5);
  standBy[3] = EEPROM.read(8);
  throwing[0] = EEPROM.read(2);
  throwing[1] = EEPROM.read(4);
  throwing[2] = EEPROM.read(6);
  throwing[3] = EEPROM.read(9);
  initialPellet = EEPROM.read(7);
}

void levelMenu() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis[13] >= 100) {
    switch (levelTwo) {
      case 0:
        levelTwo = 10;
        break;
      case 1:
        menuLcd("XRON.AN.H", "XRON.RIP.H", 0);
        selected(3, 1);
        break;
      case 2:
        menuLcd("XRON.AN.H", "XRON.RIP.H", 1);
        selected(3, 2);
        break;
      case 3:
        menuLcd("XRON.AN.M", "XRON.RIP.M", 0);
        selected(3, 3);
        break;
      case 4:
        menuLcd("XRON.AN.M", "XRON.RIP.M", 1);
        selected(3, 4);
        break;
      case 5:
        menuLcd("XRON.AN.L", "XRON.RIP.L", 0);
        selected(3, 5);
        break;
      case 6:
        menuLcd("XRON.AN.L", "XRON.RIP.L", 1);
        selected(3, 6);
        break;
      case 7:
        menuLcd("XRON.A.PEL", "XRON.AN.ZES", 0);
        selected(3, 7);
        break;
      case 8:
        menuLcd("XRON.A.PEL", "XRON.AN.ZES", 1);
        selected(3, 8);
        break;
      case 9:
        menuLcd("XRON.PEL.ZES", "OPEN 3 RELAYS", 0);
        selected(3, 9);
        break;
      case 10:
        menuLcd("XRON.PEL.ZES", "OPEN 3 RELAYS", 1);
        selected(3, 9);
        break;
      case 11:
        levelTwo = 1;
        break;
    }

    switch (levelThree) {
      case 0:
        setTimerStandBy(0, 0, 1);
        break;
      case 1:
        setTimerThrowing(0, 1, 2);
        break;
      case 2:
        setTimerStandBy(1, 2, 3);
        break;
      case 3:
        setTimerThrowing(1, 3, 4);
        break;
      case 4:
        setTimerStandBy(2, 4, 5);
        break;
      case 5:
        setTimerThrowing(2, 5, 6);
        break;
      case 6:
        setInitialTime();
        break;
      case 7:
        setTimerStandBy(3, 7, 8);
        break;
      case 8:
        setTimerThrowing(3, 8, 9);
        break;
      case 9:
        setOpenAllRelays();
        break;
      case 10:
        break;
    }
    previousMillis[13] = currentMillis;
  }
}

void setOpenAllRelays() {
  if (up) {
    lcd.clear();
    lcd.print("OPEN");
    EEPROM.write(15, 0);
  }
  if (down) {
    lcd.clear();
    lcd.print("CLOSE");
    EEPROM.write(15, 1);
  }
  selected(2, 9);
}

void setTimerStandBy(int timer, int back, int address) {
  lcd.clear();
  lcd.print("XRONOS ANAM = ");
  lcd.setCursor(12, 0);
  lcd.print(timer + 1);
  lcd.setCursor(0, 1);
  lcd.print(standBy[timer]);
  if (up) {
    standBy[timer]++;
    standBy[timer] = constrain(standBy[timer], 0, 25);
  } else if (down) {
    standBy[timer]--;
    standBy[timer] = constrain(standBy[timer], 0, 25);
  }
  EEPROM.write(address, standBy[timer]);
  selected(2, back);
}

void setTimerThrowing(int timer, int back, int address) {
  lcd.clear();
  lcd.print("XRONOS RISP = ");
  lcd.setCursor(12, 0);
  lcd.print(timer + 1);
  lcd.setCursor(0, 1);
  lcd.print(throwing[timer]);
  if (up) {
    throwing[timer]++;
    throwing[timer] = constrain(throwing[timer], 0, 25);
  } else if (down) {
    throwing[timer]--;
    throwing[timer] = constrain(throwing[timer], 0, 25);
  }
  EEPROM.write(address, throwing[timer]);
  selected(2, back);
}

void setInitialTime() {
  lcd.clear();
  lcd.print("ARXIKO PELLET = ");
  lcd.setCursor(0, 1);
  lcd.print(initialPellet);
  if (up) {
    initialPellet++;
    initialPellet = constrain(initialPellet, 0, 25);
  } else if (down) {
    initialPellet--;
    initialPellet = constrain(initialPellet, 0, 25);
  }
  EEPROM.write(7, initialPellet);
  selected(2, 6);
}

void updown() {
  if (up) {
    lcd.clear();
    levelTwo--;
  }
  if (down) {
    lcd.clear();
    levelTwo++;
  }
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