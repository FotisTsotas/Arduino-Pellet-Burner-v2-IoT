#include <Wire.h>
float myDelay = 23.5;  // this is for milliseconds
int myInt = round(myDelay);
byte high = highByte(myInt);
byte low = lowByte(myInt);
int  val = word(high, low);

void setup() {
 Serial.begin(9600); /* begin serial comm. */
 Wire.begin(); /* join i2c bus as master */
 Serial.println("I am I2C Master");
}

void loop() {
 Wire.beginTransmission(8); /* begin with device address 8 */
 Wire.write(high);  /* sends hello string */
 Wire.write(low);  /* sends hello string */
 Wire.endTransmission();    /* stop transmitting */

 Wire.requestFrom(8, 9); /* request & read data of size 9 from slave */
 while(Wire.available()){
    char c = Wire.read();/* read data received from slave */
  Serial.print(c);
 }
Serial.print(val);
 delay(1000);
}
