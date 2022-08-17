#include <Wire.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>

const char *ssid = "VODAFONE_4041";
const char *password = "r9fsfcscdfasf8ah";
int valWater = 0;
int valFire = 0;
int valExeust = 0;
int fireStatus = 0;
int statusCode = 0;
int correctTransmition = 0;
const char *myWriteAPIKey = "QT1YBK5WMHL1EJKJ";
const char *readAPi = "V8MMJ9CTRPGPHY90";
unsigned long myChannelNumber = 1735462;
unsigned long previousMillis = 0;
int connection = 13;
bool state = LOW;
WiFiClient client;
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
int relayInput = 2;

void setup() {
  pinMode(connection, OUTPUT);
  pinMode(relayInput, OUTPUT);
  Wire.begin(D1, D2); /* join i2c bus as master */
  wifiConnection();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    collectData();
    if (!checkTransmition()) {
      collectData();
    } else {
      wiFiSendData();
    }
    lastTime = millis();
  }
}


void wifiConnection() {
  Serial.begin(115200); /* begin serial comm. */
  Serial.println();
  Serial.print("Wifi connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(connection, HIGH);
  }

  Serial.println("Wifi Connected Success!");
  Serial.print("NodeMCU IP Address : ");
  Serial.println(WiFi.localIP());
  digitalWrite(connection, LOW);
  ThingSpeak.begin(client);
}

void wiFiSendData() {
  ThingSpeak.setField(1, valWater);
  ThingSpeak.setField(2, valFire);
  ThingSpeak.setField(3, valExeust);
  ThingSpeak.setField(4, fireStatus);
  statusCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (statusCode == 200) {
    Serial.println("Success");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(statusCode));
  }
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

void collectData() {
  valWater = 0;
  valFire = 0;
  valExeust = 0;
  fireStatus = 0;
  correctTransmition = 0;
  byte buf[12];
  int n = Wire.requestFrom(8, 12);
  for (int i = 0; i < n; i++) {
    buf[i] = Wire.read();
  }

  byte highW = buf[0];
  byte lowW = buf[1];
  char typeW = buf[2];
  valWater = word(highW, lowW);

  byte highF = buf[3];
  byte lowF = buf[4];
  char typeF = buf[5];
  valFire = word(highF, lowF);

  byte highE = buf[6];
  byte lowE = buf[7];
  char typeE = buf[8];
  valExeust = word(highE, lowE);

  char typeS = buf[10];
  byte valS = buf[9];
  fireStatus = valS;
  correctTransmition = buf[11];
  Serial.println(correctTransmition);
}

bool checkTransmition() {
  if (correctTransmition != 1) {
    digitalWrite(relayInput, HIGH);
    delay(1500);
    digitalWrite(relayInput, LOW);
    return false;
  } else {
    correctTransmition = 0;
    return true;
  }
}