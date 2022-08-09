#include <Wire.h>
// #include <WiFiClient.h>
#include <ThingSpeak.h>
#include <arduino-timer.h>
// #include <ESP8266WiFi.h>

const char *ssid = "Nova_2.4G_2QYhEcs";
const char *password = "uG52kCNf";
int waterValue = 0;
int fireValue = 0;
int exhaustValue = 0;
int statous;
int water;
int exhust;
int flame;
int statusCode = 0;
const char *myWriteAPIKey = "QT1YBK5WMHL1EJKJ";
const char *readAPi = "V8MMJ9CTRPGPHY90";
unsigned long myChannelNumber = 1735462;
unsigned long previousMillis = 0;
int ledPin = 13;
bool state = LOW;
// WiFiClient client;
auto timer = timer_create_default(); 

void setup() {
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Wire.onRequest(requestEvent); /* register request event */
  Serial.begin(9600);           /* start serial comm. */
  Serial.println("I am I2C Slave");
  timer.every(10000, wiFiTakeData);
  timer.every(10000, wiFiSendData);
  //  wifiConnection();
}

void loop() {

  timer.tick();  // tick the timer
}

void onOffTimer(int relayPin, int long onTime, long offTime) {
  unsigned long currentMillis = millis();
  onTime = onTime * 1000;
  offTime = offTime * 1000;
  if ((state == HIGH) && (currentMillis - previousMillis >= onTime)) {
    state = LOW;
    previousMillis = currentMillis;
    digitalWrite(relayPin, state);
  } else if ((state == LOW) && (currentMillis - previousMillis >= offTime)) {
    state = HIGH;
    previousMillis = currentMillis;
    digitalWrite(relayPin, state);
  }
}


void wifiConnection() {
  //  Serial.begin(115200);
  //  Serial.println();
  //  Serial.print("Wifi connecting to ");
  //  Serial.println(ssid);
  //  WiFi.begin(ssid, password);
  //  Serial.println();
  //  Serial.print("Connecting");

  //  while (WiFi.status() != WL_CONNECTED)
  //  {
  //    delay(500);
  //    Serial.print(".");
  //    digitalWrite(LED_BUILTIN, HIGH);
  //  }

  //  Serial.println("Wifi Connected Success!");
  //  Serial.print("NodeMCU IP Address : ");
  //  Serial.println(WiFi.localIP());
  //  digitalWrite(LED_BUILTIN, LOW);
  //  ThingSpeak.begin(client);
}
bool wiFiSendData(void *) {
    Serial.println("wifiSend");

  // ThingSpeak.setField(1, fireValue);
  // ThingSpeak.setField(2, waterValue);
  // ThingSpeak.setField(3, exhaustValue);
  // statusCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  // if (statusCode == 200) {
  //   Serial.println("Success");
  // } else {
  //   Serial.println("Problem updating channel. HTTP error code " + String(statusCode));
  // }
  // return true;
}

bool wiFiTakeData(void *) {
    Serial.println("wifiTake");

  //  long statous = ThingSpeak.readLongField(myChannelNumber, 4, readAPi);
  //  statusCode = ThingSpeak.getLastReadStatus();
  //  if (statusCode == 200)
  //  {
  //    Serial.print("Temperature: ");
  //    Serial.println(statous);
  //  }
  //  else
  //  {
  //    Serial.println("Unable to read channel / No internet connection");
  //  }
  //  return true;
}

void receiveEvent() {
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
    } else if (typeString == "e") {
      exhaustValue = val;
    } else {
      hasStatus(typeString);
    }
  }
}

void hasStatus(String status) {
  Serial.println(status);
}

void requestEvent() {
  Wire.write("Hi Master"); /*send string on request */
}