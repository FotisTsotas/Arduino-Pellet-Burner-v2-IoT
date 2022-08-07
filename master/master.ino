#include <arduino-timer.h>
#include <Wire.h>

void setup() {
  Serial.begin(9600); /* begin serial comm. */
  Wire.begin();       /* join i2c bus as master */
  Serial.println("I am I2C Master");
}

void loop() {

  Wire.requestFrom(8, 9); /* request & read data of size 9 from slave */
  while (Wire.available()) {
    char c = Wire.read(); /* read data received from slave */
    Serial.print(c);
  }
  Serial.println();
  delay(10);
}

void transmitData(int fireValue, int waterValue, int exhaustValue, int status) {
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