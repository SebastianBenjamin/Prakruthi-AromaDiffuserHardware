/*****************************************************************************
 * Prakruthi Based Aroma Diffuser
 * Technical Documentation and Implementation Guide
 * 
 * Copyright © 2025 Benjamin Sebastian
 * All rights reserved.
 * 
 * Device: ESP32 DevKit
 * Version: 1.0.0
 * Last Updated: February 2025
 *****************************************************************************/

/****************************
 * DATABASE SCHEMA
 ****************************/
/*
Data{
  Devices{
    Device1{
      User: user                 // Associated user ID
      Status: Active/Inactive    // Device operational status
      Dosha: Pittaj/Vataj/Kaphaj/PittajVataj/PittajKaphaj/VatajKaphaj  // Current Dosha setting
      SprayDelay: 5             // Spray interval in minutes (converted to ms: min*60000)
      LastUpdate: "12:00:00"    // Last spray timestamp
      Spray: 0/1               // Manual spray trigger
    }
  },
  Users{
    user1{
      UserEmail: user1@example.com
      UserPassword: usersecret
      Dosha: Pittaj/Vataj/Kaphaj/PittajVataj/PittajKaphaj/VatajKaphaj
    }
  }
}
Data
├── Devices
│   ├── Device1
│   │   ├── User: user                  // Associated user ID
│   │   ├── Status: Active/Inactive     // Device operational status
│   │   ├── Dosha: Pittaj/Vataj/Kaphaj/... // Current Dosha setting
│   │   ├── SprayDelay: 5               // Spray interval in minutes
│   │   ├── LastUpdate: "12:00:00"      // Last spray timestamp
│   │   ├── Spray: 0/1                  // Manual spray trigger
│   │
│   ├── Device2                         // Example for another device
│       ├── User: user2
│       ├── Status: Inactive
│       ├── Dosha: VatajKaphaj
│       ├── SprayDelay: 10
│       ├── LastUpdate: "13:30:00"
│       ├── Spray: 0
│
├── Users
    ├── user1
    │   ├── UserEmail: user1@example.com
    │   ├── UserPassword: usersecret
    │   ├── Dosha: Pittaj
    │
    ├── user2
        ├── UserEmail: user2@example.com
        ├── UserPassword: user2secret
        ├── Dosha: VatajKaphaj

*/

/****************************
 * PIN CONFIGURATION
 ****************************/
/*
ESP32 DevKit Pinout:
- RELAY_P (Pittaj)        : GPIO 13
- RELAY_K (Kaphaj)        : GPIO 12
- RELAY_V (Vataj)         : GPIO 14
- RELAY_PK (Pittaj-Kaphaj) : GPIO 27
- RELAY_PV (Pittaj-Vataj)  : GPIO 26
- RELAY_VK (Vataj-Kaphaj)  : GPIO 25
- RELAY_FAN              : GPIO 33
- WIFI_STATUS_LED        : GPIO 2
- MANUAL_SPRAY_SWITCH    : GPIO 34 (Input only)
- I2C LCD Display        : SDA - GPIO 21, SCL - GPIO 22
*/

/****************************
 * WIRING INSTRUCTIONS
 ****************************/
/*
1. Relay Module Connections:
   - VCC: 5V from ESP32
   - GND: GND from ESP32
   - IN1-IN7: Connect to respective GPIO pins
   - COM: Connect to power supply positive
   - NO: Connect to respective aroma diffuser positive
   - NC: Not used

2. LCD Display (I2C):
   - VCC: 5V from ESP32
   - GND: GND from ESP32
   - SDA: GPIO 21
   - SCL: GPIO 22

3. Manual Spray Switch:
   - One terminal to GPIO 34
   - Other terminal to GND
   - Internal pull-up enabled in software

4. WiFi Status LED:
   - Anode: GPIO 2 through 220Ω resistor
   - Cathode: GND

5. Power Supply:
   - ESP32: 5V USB or external 5V regulated supply
   - Relay Module: 5V from same supply as ESP32
   - Aroma Diffusers: 12V DC separate supply
*/

/****************************
 * OPERATIONAL PARAMETERS
 ****************************/
/*
1. Network Configuration:
   - SSID: SmartEnergyControlSystem
   - Password: 12345678
   - Firebase RTDB: smartenergycontrolsystem-default-rtdb
   - NTP Server: pool.ntp.org
   - Time Zone: IST (UTC+5:30)

2. Timing Parameters:
   - WiFi check interval: 5000ms
   - Display refresh: 2000ms
   - Spray duration: 5000ms
   - Default spray interval: 5 minutes

3. Display Screens:
   - Screen 1: User and Status
   - Screen 2: Dosha and Delay
   - Screen 3: Next spray countdown

4. Safety Features:
   - Auto WiFi reconnect
   - Manual override switch
   - Status LED indicator
   - Real-time monitoring
   - Automatic spray timeout
*/

/****************************
 * MAINTENANCE NOTES
 ****************************/
/*
1. Regular Maintenance:
   - Check relay contacts monthly
   - Clean aroma diffuser nozzles weekly
   - Verify LCD display connections
   - Test manual override switch
   - Update Firebase security rules

2. Troubleshooting:
   - LED off: Check WiFi connection
   - No spray: Verify relay operation
   - Display issues: Check I2C connections
   - No Firebase updates: Verify credentials

3. Safety Precautions:
   - Keep power supplies separated
   - Ensure proper ventilation
   - Regular cleaning schedule
   - Monitor spray intervals
*/

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Firebase_ESP_Client.h>
#include <LiquidCrystal_I2C.h>
#include "secrets.h"

#define DEVICE_NUMBER "PABR2D0000000001"

// WiFi Credentials     
#define WIFI_SSID "prakruthiaromadiffuser"
#define WIFI_PASSWORD "12345678"

// Firebase Credentials



// Constants and Global Variables
#define WIFI_CHECK_INTERVAL 5000
#define RELAY_DELAY 5000
unsigned long lastWifiCheck = 0;
bool lastWifiState = false;  
int SprayDelay=0;
bool SystemStatus=false;
int setDoshaRelay=-1;
String CurrentUser="";
String Dosha = "";
unsigned long previousMillis = 0; 
const unsigned long displayInterval = 5000; 
int displayState = 0; 
int SprayFromDb=0;
bool PrintedDeviceNumber=false;
String LastUpdate="";

// Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

// Outputs and other Pins
#define RELAY_P 13         
#define RELAY_K 12         
#define RELAY_V 14         
#define RELAY_PK 27        
#define RELAY_PV 26        
#define RELAY_VK 25        
#define RELAY_FAN 33       
#define WIFI_STATUS_LED 2  
#define MANNUAL_SPRAY_SWITCH 34 

void errorLcdDisplay(String error){
  lcd.setCursor(0,0);
  lcd.print(" Error Occured !");
  Serial.println(" Error Occured !");
  lcd.setCursor(0,1);
  lcd.print(error);
  Serial.println(error);
}
void updateWiFiLED() {
    bool currentWifiState = (WiFi.status() == WL_CONNECTED);
    
    digitalWrite(WIFI_STATUS_LED, currentWifiState ? HIGH : LOW);
    
    if (currentWifiState != lastWifiState) {
        lcd.clear();
        if (currentWifiState) {
            lcd.setCursor(0, 0);
            lcd.print("WiFi ");
            lcd.setCursor(0, 1);
            lcd.print(" Connected !");
            Serial.println("WiFi Connected !");
        } else {
            Serial.println("Continuing without WiFi !");
            lcd.setCursor(0, 0);
            lcd.print("Continuing  ");
            lcd.setCursor(0, 1);
            lcd.print("without WiFi !");
        }
        delay(2000);
        lcd.clear();
        lastWifiState = currentWifiState;
    }
}

void connectWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting to Wi-Fi...");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        digitalWrite(WIFI_STATUS_LED, HIGH);
        delay(500);
        digitalWrite(WIFI_STATUS_LED, LOW);
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    bool currentWifiState = (WiFi.status() == WL_CONNECTED);
    lastWifiState = currentWifiState;
    
    lcd.clear();
    if (currentWifiState) {
        Serial.println("Wi-Fi connected.");
        lcd.setCursor(0, 0);
        lcd.print("WiFi ");
        lcd.setCursor(0, 1);
        lcd.print(" Connected !");
    } else {
        Serial.println("Wi-Fi connection failed, continuing without WiFi.");
        lcd.setCursor(0, 0);
        lcd.print("Continuing  ");
        lcd.setCursor(0, 1);
        lcd.print("without WiFi !");
    }
    delay(2000);
    lcd.clear();
}

void initFirebase() {
    if (WiFi.status() == WL_CONNECTED) {
        config.api_key = API_KEY;
        auth.user.email = USER_EMAIL;
        auth.user.password = USER_PASSWORD;
        config.database_url = DATABASE_URL;
        Firebase.begin(&config, &auth);
    }
      Firebase.RTDB.setInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Spray",0);
      Firebase.RTDB.setString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Dosha","");
      Firebase.RTDB.setInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/SprayDelay",0);
      Firebase.RTDB.setString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Status","Inactive");
      Firebase.RTDB.setString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/User","");
      Serial.println("Firebase initialisation compelete");
     
}
int getDoshaRelay(String dosha){
  if (dosha == "Pittaj") {
    return RELAY_P;
} else if (dosha == "Vataj") {
    return RELAY_V;
} else if (dosha == "Kaphaj") {
    return RELAY_K;
} else if (dosha == "VatajPittaj") {
    return RELAY_PV;
} else if (dosha == "PittajKaphaj") {
    return RELAY_PK;
} else if (dosha == "VatajKaphaj") {
    return RELAY_VK;
}else{
  return-1;
}

}
void controlRelays(){
  if(setDoshaRelay!=-1){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Spraying ")  ;
    lcd.print(Dosha);
    lcd.setCursor(0,1);
    lcd.print("Aroma Now !");
    Serial.print("Spraying ");
    Serial.println(Dosha);
  digitalWrite(setDoshaRelay,LOW);
  digitalWrite(RELAY_FAN,LOW);
  delay(RELAY_DELAY);
   digitalWrite(setDoshaRelay,HIGH);
  digitalWrite(RELAY_FAN,HIGH);
   Serial.print("Spraying Done ");
    Serial.println(Dosha);
     lcd.setCursor(0, 0);
    lcd.print("Spraying Aroma")  ;
    lcd.setCursor(0,1);
    lcd.print("is compelete");
  }
  else{
    errorLcdDisplay(" Dosha not set");
    Serial.println("Dosha not set");
    fetchSettings();
  }
}
void fetchSettings() {
 if (WiFi.status() == WL_CONNECTED) {
    if (Firebase.RTDB.getString(&fbdo, String("/Data/Devices/") + String(DEVICE_NUMBER) + "/Status")) {
    SystemStatus = (fbdo.to<String>() == "Active");
}



    if (SystemStatus) {
       
        if (Firebase.RTDB.getString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Dosha")) {
            Dosha = fbdo.to<String>();
            setDoshaRelay = getDoshaRelay(Dosha);
        } else {
    Serial.print("Firebase Error: ");
    Serial.println(fbdo.errorReason());
    errorLcdDisplay("Firebase Err");
}
         if (Firebase.RTDB.getInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/SprayDelay")) {
            SprayDelay = (fbdo.to<int>()) * 60000;
        }else {
    Serial.print("Firebase Error: ");
    Serial.println(fbdo.errorReason());
    errorLcdDisplay("Firebase Err");
}
          if (Firebase.RTDB.getString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/User")) {
            CurrentUser = fbdo.to<String>();
        
        }else {
    Serial.print("Firebase Error: ");
    Serial.println(fbdo.errorReason());
    errorLcdDisplay("Firebase Err");
} 
         if (Firebase.RTDB.getInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Spray")) {
            SprayFromDb = fbdo.to<int>();
            if(SprayFromDb==1){
              controlRelays();
              Firebase.RTDB.setInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Spray",0);
               Firebase.RTDB.setString(&fbdo, String("/Data/Devices/")+DEVICE_NUMBER+"/LastUpdate", timeClient.getFormattedTime());
    Serial.print("LastUpdate : ");
    Serial.println(timeClient.getFormattedTime());
            }
        }else {
    Serial.print("Firebase Error: ");
    Serial.println(fbdo.errorReason());
    errorLcdDisplay("Firebase Err");
}
 if (Firebase.RTDB.getString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/LastUpdate")) {
            LastUpdate = fbdo.to<String>();
        
        }else {
    Serial.print("Firebase Error: ");
    Serial.println(fbdo.errorReason());
    errorLcdDisplay("Firebase Err");
} 
Serial.println();
      Serial.print("Fetched Settings => \n");
      Serial.print("- System Status :");
      Serial.print(SystemStatus);
      Serial.print("\n- Dosha :");
      Serial.print(Dosha);
      Serial.print("\n- Spray Interval :");
      Serial.print((SprayDelay/60000));
      Serial.print(" Minutes or ");
      Serial.print(SprayDelay);
      Serial.println(" MilliSeconds");
Serial.println();


      } else{
        PrintedDeviceNumber=false;
      }
    }
}







void setup() {
    Serial.begin(9600);
    pinMode(RELAY_P,OUTPUT);
    pinMode(RELAY_K,OUTPUT);
    pinMode(RELAY_V,OUTPUT);
    pinMode(RELAY_PK,OUTPUT);
    pinMode(RELAY_PV,OUTPUT);
    pinMode(RELAY_VK,OUTPUT);
    pinMode(RELAY_FAN,OUTPUT);
    pinMode(MANNUAL_SPRAY_SWITCH,INPUT_PULLUP);
    digitalWrite(RELAY_P,HIGH);
digitalWrite(RELAY_K,HIGH);
digitalWrite(RELAY_V,HIGH);
digitalWrite(RELAY_PK,HIGH);
digitalWrite(RELAY_PV,HIGH);
digitalWrite(RELAY_VK,HIGH);
digitalWrite(RELAY_FAN,HIGH);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Prakruthi Based");
    lcd.setCursor(0, 1);
    lcd.print(" Aroma Diffuser");
    Serial.println("Prakruthi Based Aroma Diffuser");

    delay(4000);
    lcd.clear();
    
    connectWiFi();
    initFirebase();
    timeClient.begin();

}
void handleDisplay(unsigned long currentMillis,unsigned long timeLeft){
     if (currentMillis - previousMillis >= displayInterval) {
    previousMillis = currentMillis;
    lcd.clear();
    
    switch (displayState) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("User:");
        lcd.print(CurrentUser);
        lcd.setCursor(0, 1);
        lcd.print("Status: Active");
    Serial.print("\nUser : ");
    Serial.print(CurrentUser);
    Serial.println("Status: Active ");


        displayState = 1;
        break;

      case 1:
        lcd.setCursor(0, 0);
        lcd.print("Dosha: ");
        lcd.print(Dosha);
        lcd.setCursor(0, 1);
        lcd.print("Delay: ");
        lcd.print(SprayDelay/60000);
        lcd.print("mins");
      Serial.print("Dosha : ");
    Serial.print(Dosha);
    Serial.print("\n Delay: ");
    Serial.println(SprayDelay);
        displayState = 2;
        break;

      case 2:
       lcd.setCursor(0, 0);
        lcd.print("Dosha  : ");
        lcd.print(Dosha);
        lcd.setCursor(0, 1);
        lcd.print("Next in: ");
        lcd.print(timeLeft);
        lcd.print("mins");
        Serial.print(" \nNext in: ");
    Serial.println(timeLeft);

        displayState = 3;
        break;
        case 3:
        lcd.setCursor(0, 0);
    lcd.print("Prakruthi Based");
    lcd.setCursor(0, 1);
    lcd.print(" Aroma Diffuser");
    displayState=4;
    break;
        case 4:
        for(int i=0;i<(displayInterval/1000);i++){
        lcd.setCursor(0, 0);
    lcd.print("Time ");
    lcd.print(timeClient.getFormattedTime());
    lcd.setCursor(0, 1);
    lcd.print("Last ");
    lcd.print(LastUpdate);
    delay(1000);
    }
    displayState=0;
    break;
    }
  }
}

void loop() {
  
  fetchSettings();
  unsigned long currentMillis = millis();
  unsigned long SprayTime = currentMillis;
  while(SystemStatus&&WiFi.status() == WL_CONNECTED){
    
    currentMillis = millis();

  int switchState=digitalRead(MANNUAL_SPRAY_SWITCH);
  // Serial.print("\nswitch status : ");
  // Serial.print(switchState);
  // Serial.print("\nSpray delay : ");
  // Serial.print(SprayDelay);
  // Serial.print("\nCurrent time : ");
  // Serial.println(currentMillis);
  // Serial.println();
  
 if(currentMillis>=SprayTime){
  Serial.print("\nswitch status : ");
  Serial.print(switchState);
  Serial.print("\nSpray delay : ");
  Serial.print(SprayDelay);
  Serial.print("\nCurrent time : ");
  Serial.println(currentMillis);
 controlRelays();
  timeClient.update();
   Firebase.RTDB.setString(&fbdo, String("/Data/Devices/")+DEVICE_NUMBER+"/LastUpdate", timeClient.getFormattedTime());
    Serial.print("LastUpdate : ");
    Serial.println(timeClient.getFormattedTime());
SprayTime=currentMillis+SprayDelay;

 }
unsigned long timeLeft = (SprayTime > currentMillis) ? (SprayTime - currentMillis) : 0;
  timeLeft=timeLeft/60000;
  

  if (currentMillis - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
            updateWiFiLED();
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
        
        lastWifiCheck = currentMillis;
    }
            fetchSettings();
 handleDisplay(currentMillis,timeLeft);
  }
  if(!PrintedDeviceNumber){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("System Inactive ");
    lcd.setCursor(0,1);
    lcd.print(DEVICE_NUMBER);
  Serial.println("System Inactive");
  Serial.print("System number : ");
  Serial.println(DEVICE_NUMBER);
  PrintedDeviceNumber=true;
  }

}



