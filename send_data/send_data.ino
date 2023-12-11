#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Firebase_ESP_Client.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Create a TinyGPS++ object
TinyGPSPlus gps;

// Create a HardwareSerial object to communicate with the GPS module
HardwareSerial SerialGPS(1); // Use Serial1 for ESP32
HardwareSerial SerialPort(2);

// Insert your network credentials
#define WIFI_SSID "Undead"
#define WIFI_PASSWORD "12345678"

#define API_KEY "AIzaSyBFXji0tc6nLJfv-NFcuKod-IHU-5jTZX8"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "18221001@std.stei.itb.ac.id"
#define USER_PASSWORD "Wanazis111202"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://esp32-firebase-demo-f2551-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Variable locations
float lat;
float lgt;
int countMhs;

// Variables to save database paths
String databasePath;
String mhsPath;
String latPath;
String lgtPath;
String terPath;
// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 2000;

String terminal;
String receivedData;
String temp;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Write integer values to the database
void sendInt(String path, int value){
  if (Firebase.RTDB.setInt(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

// Write integer values to the database
void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}
// Write integer values to the database
void sendString(String path, String value){
  String temp=" ";
  value = value + temp;
  if (Firebase.RTDB.setString(&fbdo, path.c_str(), value)){
  Serial.print("Writing value: ");
  Serial.print (value);
  Serial.print(" on the following path: ");
  Serial.println(path);
  Serial.println("PASSED");
  Serial.println("PATH: " + fbdo.dataPath());
  Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void setup() {
  SerialPort.begin(115200, SERIAL_8N1, 18, 19);
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
  delay(1000);

  initWiFi();

  //Firebase setUp
  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/ShuttleData/" + uid;

  // Update database path for sensor readings
  mhsPath = databasePath + "/countMhs";
  latPath = databasePath + "/latitude";
  lgtPath = databasePath + "/longitude";
  terPath = databasePath + "/Terminal";

  // Sync Data in Firebase
  if(Firebase.RTDB.getInt(&fbdo, mhsPath)){
    countMhs = fbdo.intData();
  }
  if(Firebase.RTDB.getString(&fbdo, terPath)){
    terminal = fbdo.stringData();
    terminal.trim(); 
    temp = String(countMhs)+","+terminal+".";
    SerialPort.print(temp);
  }
}

void loop() {
  // Read from the other ESP32 via Serial2
  if (SerialPort.available()) {
    Serial.println("get");
    receivedData = SerialPort.readString();
    temp = receivedData.substring(0,receivedData.indexOf(","));
    countMhs = temp.toInt();
    terminal = receivedData.substring(receivedData.indexOf(",")+1,receivedData.indexOf("."));
    Serial.print("Received from ESP32 #2: ");
    Serial.println(receivedData);
  }

  //Read location
  if (SerialGPS.available() > 0) {
    while (SerialGPS.available() > 0) {
      if (gps.encode(SerialGPS.read())) {
        // If valid data is available, print the GPS information
        if (gps.location.isValid()) {
          lat = gps.location.lat();
          lgt = gps.location.lng();
          Serial.print("Latitude: ");
          Serial.println(lat);
          Serial.print("Longitude: ");
          Serial.println(lgt);
        } else {
          Serial.println("GPS data not valid yet.");
        }
      }
    }
  }

  //Send to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    // Send readings to database:
    sendInt(mhsPath, countMhs);
    sendFloat(latPath, lat);
    sendFloat(lgtPath, lgt);
    sendString(terPath, terminal);
  }
  delay(1000);
}