#include <arduino-timer.h>
#include <Wire.h>
#include <arduino-timer.h>

unsigned long previousMillis[5] = { 0, 0, 0, 0, 0 };  //number of onOfftimers
bool state[5] = { 0, 0, 0, 0, 0 };                    //number of onOfftimers
auto timer = timer_create_default();
float fireValue = 0;
float exhaustValue = 0;
float waterValue = 0;
int status = 0;
byte x = 0;
void setup() {
  Serial.begin(9600);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  Wire.begin();
  Serial.println("I am I2C Master");
  timer.every(1000, transmitData);
}

void loop() {
  // timer.tick();
  requestDataFromSlave();
  delay(500);
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

void requestDataFromSlave() {
  Wire.requestFrom(8, 1); /* request & read data of size 9 from slave */
  int bytes = Wire.available();
  Serial.print("Slave sent ");
  Serial.print(bytes);
  Serial.print(" of information\n");
  for (int i = 0; i < bytes; i++) {
    x = Wire.read();
    Serial.print("Slave Sent: ");
    Serial.print(x);
    Serial.println();
  }
}
void transmitData() {
  Wire.beginTransmission(8); /* begin with device address 8 */
  sendDataToSlave("w", waterValue);
  sendDataToSlave("f", fireValue);
  sendDataToSlave("e", exhaustValue);
  sendDataToSlave("s", status);
  Wire.endTransmission(); /* stop transmitting */
}

void sendDataToSlave(String type, int value) {
  int valueOfInt = round(value);
  byte high = highByte(valueOfInt);
  byte low = lowByte(valueOfInt);
  Wire.write(high);         /* sends hello string */
  Wire.write(low);          /* sends hello string */
  Wire.write(type.c_str()); /* sends hello string */
}