/*
 * As of 25 August 2022: Able to send all data to Firebase Realtime Database 
 * reference is https://randomnerdtutorials.com/esp32-data-logging-firebase-realtime-database/
 */
 
// LIBRARIES 
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Protocentral_MAX30205.h"
#include "AM232X.h"
#include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <Adafruit_MPU6050.h>
#include "time.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "OPPO Reno3"
#define WIFI_PASSWORD "carinachu22"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDAtJkZ4hg-ppp5XylD3EiMTMZJyLWh4WE"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "carinachu22@gmail.com"
#define USER_PASSWORD "helmet"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://helmet-v3-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID) 
String databasePath;

// Database child nodes
String timePath = "/timestamp";
String humidityPath = "/humidity";
String temperaturePath = "/temperature";
String bodytemperaturePath = "/bodytemperature";
String heartratePath = "/heartrate";
String spo2Path = "/spo2";
String fallPath = "/fall";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";
// Global Variables 
int resPin = 5;
int mfioPin = 18;
float ax=0, ay=0, az=0,at=0, gx=0, gy=0, gz=0,angleChange=0;
boolean trigger1=false; 
boolean trigger2=false; //stores if second trigger (upper threshold) has occurred
boolean trigger3=false; //stores if third trigger (orientation change) has occurred
byte trigger1count=0; //stores the counts past since trigger 1 was set true
byte trigger2count=0; //stores the counts past since trigger 2 was set true
byte trigger3count=0; //stores the counts past since trigger 3 was set true
int fall = 0;
String fall_status = "No";
int trigger_status = 0;
float bodytemperature;
float humidity;
float temperature;
float pressure;
int count = 0;
// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 5000;

// Define Sensor Objects
AM232X AM2320;
MAX30205 tempSensor;
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin);
bioData body;
Adafruit_MPU6050 mpu;

// Initialize FeverClick
void initFeverClick(){
  if(!tempSensor.scanAvailableSensors()){
    Serial.println("Couldn't find the temperature sensor, please connect the sensor." );
    delay(1000);
  }  
  tempSensor.begin();   // set continuos mode, active mode
}

// Initialize AM2320
void initAM2320(){
  if (! AM2320.begin() )
  {
    Serial.println("AM2320 Sensor not found");
    delay(500);
  }  
  AM2320.wakeUp();
  delay(2000);
}

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

unsigned long getTime(){
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
    return (0);
  }
  time(&now);
  return now;
}

void initPPG(){
      //// BIOSENSOR ////
  int result = bioHub.begin();
  if (result == 0) // Zero errors!
    Serial.println("BioSensor Sensor started!");
  else
    Serial.println("Could not communicate with the Biosensor!!!");
 
  Serial.println("Configuring BioSensor...."); 
  int error = bioHub.configBpm(MODE_ONE); // Configuring just the BPM settings. 
  if(error == 0){ // Zero errors!
    Serial.println("BioSensor configured.");
  }
  else {
    Serial.println("Error configuring Bio sensor.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }
  
}

void initMPU(){
  //// MPU6050 ////
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }  
}

// Read BPM and return string heart rate 
String readBPM() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  Serial.println("readBPM");

  body = bioHub.readBpm();
  Serial.print("Heartrate: ");
  Serial.print(body.heartRate); 
  Serial.print(", Confidence: ");
  Serial.print(body.confidence); 
  Serial.print(", Oxygen: ");
  Serial.print(body.oxygen); 
  Serial.print(", Status: ");
  Serial.println(body.status); 
  int BPM = body.heartRate;
  if (isnan(BPM)) {
    //Serial.println("Failed to read from BPM sensor!");
    return "Error";
  }
  else if (body.status !=3){
    //Serial.println("No BPM detected");
    return "--";
  }
  else {
    //Serial.println(BPM);
    return String(BPM);
  }
}

// Read PPG and return string SP02
String readSP02() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  body = bioHub.readBpm();
  int SP02 = body.oxygen;
  if (isnan(SP02)) {
    Serial.println("Failed to read from SP02 sensor!");
    return "Error";
  }
  else if (body.status !=3){
    Serial.println("No SP02 detected");
    return "--";
  }
  else {
    Serial.println(SP02);
    return String(SP02);
  }
}

// FALL DETECTION 
float readMPU(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  ax = a.acceleration.x;
  ay = a.acceleration.y;
  az = a.acceleration.z;
  at = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);    
  return at;
}

float readGyro(){
 sensors_event_t a, g, temp;
 mpu.getEvent(&a, &g, &temp);  
 gx = g.gyro.x;
 gy = g.gyro.y;
 gz = g.gyro.z;
 float radChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
 angleChange = radChange*180/3.141592654;
 return angleChange;
}

String readFallStatus(){
 at = readMPU();
 //Serial.print("AT: "); Serial.println(at);
 if (trigger3==true){
    trigger3count++;
    Serial.print("trigger3count ");Serial.println(trigger3count);
    if (trigger3count>=20){ 
       angleChange = readGyro();
       //delay(10);
       Serial.println(angleChange); 
       if ((angleChange>=0) && (angleChange<=20)){ //if orientation changes remains between 0-10 degrees
           fall=1; trigger3=false; trigger3count=0;
           trigger_status = 4;
           Serial.println(angleChange);
             }
       else{ //user regained normal orientation
          trigger3=false; trigger3count=0;
          trigger_status = 0;
          Serial.println("TRIGGER 3 DEACTIVATED");
       }
     }
  }
 if (fall==1){ //in event of a fall detection
   Serial.println("FALL DETECTED");
  // exit(1);
   }
 if (trigger2count>=6){ //allow 0.5s for orientation change
   trigger2=false; trigger2count=0;
   trigger_status = 0;
   Serial.println("TRIGGER 2 DECACTIVATED");
   }
 if (trigger1count>=6){ //allow 0.5s for AM to break upper threshold
   trigger1=false; trigger1count=0;
   trigger_status = 0;
   Serial.println("TRIGGER 1 DECACTIVATED");
   }
 if (trigger2==true){
   trigger2count++;
   //angleChange=acos(((double)x*(double)bx+(double)y*(double)by+(double)z*(double)bz)/(double)AM/(double)BM);
   angleChange = readGyro(); Serial.println(angleChange);
   if (angleChange>=30 && angleChange<=400){ //if orientation changes by between 80-100 degrees
     trigger3=true; trigger2=false; trigger2count=0;
     trigger_status = 3;
     Serial.println(angleChange);
     Serial.println("TRIGGER 3 ACTIVATED");
       }
   }
 if (trigger1==true){
   trigger1count++;
   if (at>=12){ //if AM breaks upper threshold (3g)
     trigger2=true;
     trigger_status = 2;
     Serial.println("TRIGGER 2 ACTIVATED");
     trigger1=false; trigger1count=0;
     }
   }
 if (at<=2 && trigger2==false){ //if AM breaks lower threshold (0.4g)
   trigger1=true;
   trigger_status = 1;
   Serial.println("TRIGGER 1 ACTIVATED");
   }
//It appears that delay is needed in order not to clog the port
 delay(100);
 if (trigger_status ==4){
  fall_status = "Yes";
 }
 return fall_status;
  
}


void setup(){
  Serial.begin(115200);
  Wire.begin();

  // Initialize FeverClick sensor
  initWiFi();
  initFeverClick();
  initAM2320();
  initPPG();
  initMPU();
  configTime(0,0,ntpServer);

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

  // Getting the user UID might take a few secon
  
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path with uid
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop(){
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print("time: "); Serial.println(timestamp);

    parentPath = databasePath + "/" + String(timestamp);

    // Get latest sensor readings
    int status = AM2320.read();
    bodytemperature = tempSensor.getTemperature();
    temperature = AM2320.getTemperature();
    humidity = AM2320.getHumidity();
    json.set(temperaturePath.c_str(), String(temperature));
    json.set(humidityPath.c_str(), String(humidity));
    json.set(bodytemperaturePath.c_str(), String(bodytemperature));
    json.set(heartratePath.c_str(), readBPM());
    json.set(spo2Path.c_str(), readSP02());
    json.set(fallPath.c_str(), readFallStatus());
    json.set(timePath.c_str(),String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    count++;


  }
}
