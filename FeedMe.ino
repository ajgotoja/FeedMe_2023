/*
  Created by Igor Barć
  University of Technology in Rzeszow
  Based on project: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/ by Rui Santos
*/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

const int trigPin = 32;
const int echoPin = 27;
const int blueButtonPin = 21;
const int servoPin = 33;
const int LEDPin = 19;
const int FSRPin = 34;
const float deepOfBox = 12.0;
unsigned long sendDataPrevTime = 0;
unsigned long errorPrevTime = 0;
unsigned long feedingPrevTime = 0;
float levelOfFood;
float fsrRead;
long duration;
bool manualFoodTime = false;
bool foodTime = false;
bool signupOK = false;
int sizeOfSchedule = 0;
String schedule;
String timeStamp;
String errorMessage;
String prevErrorMessage = "prevErrorMessage";

// OLED display dimensions in pixels
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64

// OLED pinout 
#define OLED_MOSI 23
#define OLED_CLK 18
#define OLED_DC 17
#define OLED_CS 5
#define OLED_RESET 4

// const sound of speed
#define SOUND_SPEED 0.034

// Network credentials
#define WIFI_SSID "***"
#define WIFI_PASSWORD "***"

// Firebase project API Key and RTDB URL
#define API_KEY "***"
#define DATABASE_URL "***"

// Firebase project EMAIL and PASSWORD
#define USER_EMAIL "***"
#define USER_PASSWORD "***"

// Servo object
Servo servoFeeder;

// OLED display configuration
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// pixel picture with logo
const unsigned char img_logo[] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x3f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xf8, 0x00, 0x00, 
0x00, 0x00, 0x07, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xe1, 0xf8, 0x7f, 0x80, 0x00, 
0x00, 0x00, 0x7f, 0x3f, 0xff, 0x8f, 0xc0, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xf3, 0xf0, 0x00, 
0x00, 0x01, 0xf3, 0xff, 0xff, 0xfd, 0xf8, 0x00, 0x00, 0x07, 0xcf, 0xff, 0xff, 0xff, 0x7c, 0x00, 
0x00, 0x0f, 0x9f, 0xff, 0xff, 0xff, 0xbe, 0x00, 0x00, 0x1f, 0x3f, 0xff, 0xff, 0xff, 0xdf, 0x00, 
0x00, 0x3e, 0xff, 0xff, 0xff, 0xff, 0xef, 0x80, 0x00, 0x7c, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xc0, 
0x00, 0x79, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xc0, 0x00, 0xf3, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xe0, 
0x01, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xf0, 0x03, 0xc7, 0xff, 0xf1, 0xf9, 0xff, 0xfe, 0xf0, 
0x03, 0xcf, 0xff, 0xe0, 0xf0, 0xff, 0xff, 0x78, 0x07, 0x8f, 0xff, 0xe0, 0x60, 0x7f, 0xff, 0x78, 
0x07, 0x9f, 0xff, 0xc0, 0x60, 0x7f, 0xff, 0xb8, 0x0f, 0x1f, 0xff, 0xc0, 0x40, 0x3f, 0xff, 0xbc, 
0x0f, 0x3f, 0xff, 0xc0, 0x40, 0x3f, 0xff, 0xbc, 0x1e, 0x3f, 0xff, 0xc0, 0x40, 0x3f, 0xff, 0xdc, 
0x1e, 0x3f, 0xff, 0xc0, 0x60, 0x7f, 0xff, 0xde, 0x1e, 0x3f, 0xf8, 0x60, 0xe0, 0x61, 0xff, 0xde, 
0x3e, 0x3f, 0xf0, 0x70, 0xf0, 0xc0, 0xff, 0xde, 0x3c, 0x7f, 0xf0, 0x3f, 0xff, 0xc0, 0xff, 0xce, 
0x3c, 0x7f, 0xf0, 0x3c, 0x07, 0xc0, 0xff, 0xce, 0x3c, 0x7f, 0xf0, 0x38, 0x01, 0xc0, 0xff, 0xce, 
0x3c, 0x7f, 0xf8, 0x30, 0x00, 0xc1, 0xff, 0xce, 0x3c, 0x7f, 0xfc, 0x60, 0x00, 0x63, 0xff, 0xce, 
0x3c, 0x7f, 0xff, 0xc0, 0x00, 0x7f, 0xff, 0xce, 0x3c, 0x3f, 0xff, 0xc0, 0x00, 0x3f, 0xff, 0xce, 
0x3c, 0x3f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0xce, 0x3c, 0x3f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0xde, 
0x3e, 0x3f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0x9e, 0x3e, 0x1f, 0xff, 0x00, 0x00, 0x1f, 0xff, 0x9c, 
0x3e, 0x1f, 0xff, 0x00, 0x00, 0x1f, 0xff, 0x9c, 0x3e, 0x1f, 0xff, 0x80, 0x00, 0x1f, 0xff, 0x3c, 
0x3f, 0x0f, 0xff, 0x80, 0x00, 0x3f, 0xff, 0x38, 0x1f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x38, 
0x1f, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x78, 0x1f, 0x87, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x70, 
0x0f, 0x83, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xf0, 0x0f, 0xc1, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xe0, 
0x07, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xc0, 0x07, 0xe0, 0x7f, 0xff, 0xff, 0xff, 0xe3, 0xc0, 
0x03, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xc7, 0x80, 0x01, 0xf8, 0x1f, 0xff, 0xff, 0xff, 0x0f, 0x00, 
0x01, 0xfc, 0x07, 0xff, 0xff, 0xfe, 0x1e, 0x00, 0x00, 0xfe, 0x03, 0xff, 0xff, 0xf8, 0x3c, 0x00, 
0x00, 0x7f, 0x80, 0xff, 0xff, 0xe0, 0x78, 0x00, 0x00, 0x3f, 0xc0, 0x1f, 0xff, 0x01, 0xf0, 0x00, 
0x00, 0x1f, 0xf0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x1f, 0xc0, 0x00, 
0x00, 0x07, 0xff, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x0f, 0xfe, 0x00, 0x00, 
0x00, 0x00, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xf0, 0x00, 0x00, 
0x00, 0x00, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setup(){
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);  // trigPin for distance sensor
  pinMode(echoPin, INPUT);   // echoPin for distance sensor
  pinMode(blueButtonPin, INPUT); // button
  pinMode(LEDPin, OUTPUT);   // LED
  servoFeeder.setPeriodHertz(50);    // 50 Hz frequency
	servoFeeder.attach(servoPin); // attaches the servo on pin 18 to the servo object
  
  // if OLED is not working, loop forever
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(1);
  }
  delay(200);

  // Logo display for 3 seconds
  display.clearDisplay();
  display.drawBitmap(32, 0, img_logo, 64, 64, 1);
  display.display();
  delay(3000);

  // Connecting to WiFi
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  display.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    display.print(".");
    display.display();
    delay(1000);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  // Initialization with NTP for time
  timeClient.begin();
  timeClient.setTimeOffset(3600); //Time offset is set for UTC+1

  // Assign the api key and RTDB URL
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  //Assign email and password
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Set servo on position 0
  servoFeeder.write(0);
}

void loop(){
  if(WiFi.status() == WL_CONNECTED) {
    // Get valid time
    timeClient.update();

    foodLevelSensor();
    fsrRead = analogRead(FSRPin);
    fsrRead = fsrRead/4095.0;

    // Display informations
    timeStamp = timeClient.getFormattedTime().substring(0, timeClient.getFormattedTime().length()-3);
    display.clearDisplay();
    display.setCursor(98, 0);
    display.println(timeStamp);
    display.println("Harmonogram:");
    display.println(schedule);
    display.println();
    display.println(errorMessage);
    display.println((String)fsrRead + " - nacisk");
    showLevelOfFood();
    display.display();

    // No error
    if(millis() - errorPrevTime > 5000) {
      errorMessage = " ";
      digitalWrite(LEDPin, LOW);
    }
    
    // Error about low level of food
    if(levelOfFood <= 10.0) { 
      errorMessage = "Niski poziom karmy";
      errorPrevTime = millis();
      digitalWrite(LEDPin, HIGH);
    }

    // Manual feeding activated by physical button or app
    if(manualFoodTime || digitalRead(blueButtonPin) == HIGH) {
      if(errorMessage == " ") {
        if(weightSensor())
          serveFood();
        else {
          errorPrevTime = millis();
          digitalWrite(LEDPin, HIGH);
        }
      }
      if(manualFoodTime)
        sendManualFeeding();
    }
    
    // Automatic feeding activated by schedule
    if(checkFeedingTime() && (millis() - feedingPrevTime > 60000) && errorMessage == " ") {
      feedingPrevTime = millis();
      if(weightSensor())
        serveFood();
      else {
        errorPrevTime = millis();
        digitalWrite(LEDPin, HIGH);
      }
    }
  
    // update from firebase
    if(Firebase.ready() && (millis() - sendDataPrevTime > 10000 || sendDataPrevTime == 0)) { 
      sendDataPrevTime = millis();
      getManualFeeding();
      sendFoodLevel();
      getSchedule();
    }
    sendErrorMessage();
  } else {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
  }
  delay(200);
}

void getSchedule(){
  schedule = "";
  if(Firebase.RTDB.getJSON(&fbdo, "/schedule")) {
    FirebaseJson *json = fbdo.jsonObjectPtr();
    size_t len = json->iteratorBegin();
    sizeOfSchedule = len/2;
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i=i+2) {
      json->iteratorGet(i, type, key, value);
      i == len-2 ? schedule += key : schedule += key + ", ";
    }
    json->iteratorEnd();
    Serial.println(schedule);
  } else {
  Serial.println("Blad przy pobieraniu harmonogramu: " + fbdo.errorReason());
  if(fbdo.errorReason()  == "path not exist")
    schedule = "Brak harmonogramu";
  }
}

void getManualFeeding() {
  if (Firebase.RTDB.getBool(&fbdo, "/manualFeeding"))
    manualFoodTime = fbdo.boolData();
  else {
    Serial.println("Blad przy pobieraniu manual feeding: " + fbdo.errorReason());
    manualFoodTime = false;
  }
}

void sendManualFeeding() {
  manualFoodTime = false;
  if(!Firebase.RTDB.setBool(&fbdo, "/manualFeeding", false))
    Serial.println("Blad przy recznym podawaniu karmy: " + fbdo.errorReason());
}

bool checkFeedingTime(){
  int count = 0;
  for(int i = 1; i <= sizeOfSchedule; i++) {
    String timeSchedule = schedule.substring((i-1)*7, (i-1)*7+5);
    if(timeStamp == timeSchedule)
      count++;
  }
  return count > 0 ? true : false;
}

void foodLevelSensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 
  // Convert the time into a distance
  levelOfFood = (duration * SOUND_SPEED)/2;
  levelOfFood = 100.0-(100.0*(levelOfFood-1.0)/deepOfBox);
  if(levelOfFood < -10000)
    levelOfFood = 100.0;
  else if(levelOfFood < 0)
    levelOfFood = 0.0;
}

void sendFoodLevel() {
  if(!Firebase.RTDB.setInt(&fbdo, "/foodLevel", levelOfFood)) {
    Serial.println("Blad przy wysylaniu poziomu karmy: " + fbdo.errorReason());
  }
}

bool weightSensor() {
  if(fsrRead == 0.0) {
    errorMessage = "Brak miski";
    return false;
  }
  else if(fsrRead > 0.6) {
    errorMessage = "Brak miejsca w misce";
    return false;
  }
  else
    return true;
}

void showLevelOfFood() {
  uint8_t levelBar = map(levelOfFood, 0, 100, 0, 31);
  display.drawRect(0, 0, 35, 7, SSD1306_WHITE);
  display.fillRect(2, 2, levelBar, 3, SSD1306_WHITE);
  display.setCursor(37, 0);
  display.println((String)int(levelOfFood) + "%");
}

void serveFood() {
  servoFeeder.write(126);
  delay(3000);
  servoFeeder.write(0);
}

void sendErrorMessage() {
  if(errorMessage != prevErrorMessage){
    prevErrorMessage = errorMessage;
    if(!Firebase.RTDB.setString(&fbdo, "/errorMessage", errorMessage))
      Serial.println("Blad przy wysylaniu wiadomosci z bledami: " + fbdo.errorReason());
  }
}