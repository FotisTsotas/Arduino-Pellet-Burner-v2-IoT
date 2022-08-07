#include <Wire.h>
int waterValue = 0;
int fireValue = 0;
int exhaustValue = 0;

void setup() {
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Wire.onRequest(requestEvent); /* register request event */
  Serial.begin(9600);           /* start serial comm. */
  Serial.println("I am I2C Slave");
}

void loop() {
  delay(100);
}

void receiveEvent(int howMany) {

  int val = 0;
  while (1 < Wire.available()) {
    byte high = Wire.read();
    byte low = Wire.read();
    val = word(high, low);
    char type = Wire.read();
    String typeString = String(type);
    if (typeString == "w") {
      waterValue = val;
    } else if (typeString == "f") {
      fireValue = val;
    }
    else if (typeString == "e") {
      exhaustValue = val;
    } else {
      hasStatus(typeString);
    }
    
  }
  Serial.println(exhaustValue);
  Serial.println(fireValue);
  Serial.println(waterValue);
}

void hasStatus(String status) {
  Serial.println(status);
}

void requestEvent() {
 Wire.write("Hi Master");  /*send string on request */
}