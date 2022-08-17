#include <LiquidCrystal.h>

#include <Wire.h>
#include <arduino-timer.h>
#include <EEPROM.h>
#include <max6675.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SI2C.h>
#define ONE_WIRE_BUS 5
const int flameSensor = A0;
const int voltageSensor = A2;
const int motorAir = 1;
const int beginResistor = 2;
const int motorPellet = 6;
const int onOffTimmers = 7;
const int buzz = 13;
int ktcSO = 8;
int ktcCS = 9;
int ktcCLK = 10;
int volt = 0;
int status = 0;
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
float fireValue = 0;
float exhaustValue = 0;
float waterValue = 0;
float firePerCent;
unsigned long previousMillis[5] = { 0, 0, 0, 0, 0 };  //number of onOfftimers
unsigned long pelletInDelayLastTime = 0;
unsigned long pelletInDelay = 10000;
bool state[5] = { 0, 0, 0, 0, 0 };
auto timer = timer_create_default();
bool open = false;
bool ignition = false;
bool manual = true;
bool menu = false;



MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  relay(1, 1, 1, 1);
  pinMode(motorAir, OUTPUT);
  pinMode(beginResistor, OUTPUT);
  pinMode(motorPellet, OUTPUT);
  pinMode(onOffTimmers, OUTPUT);
  pinMode(buzz, OUTPUT);
  lcd.begin(16, 2);
  sensors.begin();
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
}

void loop() {
  collectDataFromSensors();
  if (open) {
    mainMode();
  } else {
    closed();
  }
  // timer.tick();
}

void mainMode() {
  if (!ignition) {
    pelletIn();
  } else {
    mainWorking();
  }
}

void pelletIn() {
  if ((millis() - pelletInDelayLastTime) > pelletInDelay) {
    relay(1, 1, 0, 1);
    timeOn = round(millis() - pelletInDelayLastTime) / 1000)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("EISAGWGI PELLET");
    lcd.setCursor(0, 1);
    lcd.print(timeOn);
    lcd.print(" sec");
  }
  pelletInDelayLastTime = millis();
}

void mainWorking() {
}

void closed() {
}

void menuMode() {
}

void collectDataFromSensors() {
  sensors.requestTemperatures();
  waterValue = sensors.getTempCByIndex(0);
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
}

void relay(bool air, bool resist, bool motorpellet, bool onofftimers) {
  digitalWrite(motorAir, air);
  digitalWrite(beginResistor, resist);
  digitalWrite(motorPellet, motorpellet);
  digitalWrite(onOffTimmers, onofftimers);
}

void onOffTimer(int relayPin, int i, int long onTime, long offTime) {
  unsigned long currentMillis = millis();
  onTime = onTime * 1000;
  offTime = offTime * 1000;
  if ((state[i] == HIGH) && (currentMillis - previousMillis[i] >= onTime)) {
    state[i] = LOW;
    previousMillis[i] = currentMillis;
    digitalWrite(relayPin, state[i]);
  } else if ((state[i] == LOW) && (currentMillis - previousMillis[i] >= offTime)) {
    state[i] = HIGH;
    previousMillis[i] = currentMillis;
    digitalWrite(relayPin, state[i]);
  }
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