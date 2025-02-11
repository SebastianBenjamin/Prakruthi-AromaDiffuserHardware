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
      Dosha: Pitta/Vata/Kapha/PittaVata/PittaKapha/VataKapha  // Current Dosha setting
      SprayDelay: 5             // Spray interval in minutes (converted to ms: min*60000)
      LastUpdate: "12:00:00"    // Last spray timestamp
      Spray: 0/1               // Manual spray trigger
    }
  },
  Users{
    user1{
      UserEmail: user1@example.com
      UserPassword: usersecret
      Dosha: Pitta/Vata/Kapha/PittaVata/PittaKapha/VataKapha
    }
  }
}
Data
├── Devices
│   ├── Device1
│   │   ├── User: user                  // Associated user ID
│   │   ├── Status: Active/Inactive     // Device operational status
│   │   ├── Dosha: Pitta/Vata/Kapha/... // Current Dosha setting
│   │   ├── SprayDelay: 5               // Spray interval in minutes
│   │   ├── LastUpdate: "12:00:00"      // Last spray timestamp
│   │   ├── Spray: 0/1                  // Manual spray trigger
│   │
│   ├── Device2                         // Example for another device
│       ├── User: user2
│       ├── Status: Inactive
│       ├── Dosha: VataKapha
│       ├── SprayDelay: 10
│       ├── LastUpdate: "13:30:00"
│       ├── Spray: 0
│
├── Users
    ├── user1
    │   ├── UserEmail: user1@example.com
    │   ├── UserPassword: usersecret
    │   ├── Dosha: Pitta
    │
    ├── user2
        ├── UserEmail: user2@example.com
        ├── UserPassword: user2secret
        ├── Dosha: VataKapha

*/

/****************************
 * PIN CONFIGURATION
 ****************************/
/*
ESP32 DevKit Pinout:
- RELAY_P (Pitta)        : GPIO 13
- RELAY_K (Kapha)        : GPIO 12
- RELAY_V (Vata)         : GPIO 14
- RELAY_PK (Pitta-Kapha) : GPIO 27
- RELAY_PV (Pitta-Vata)  : GPIO 26
- RELAY_VK (Vata-Kapha)  : GPIO 25
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

#define DEVICE_NUMBER "PABR2D0000000001"

// WiFi Credentials
#define WIFI_SSID "SmartEnergyControlSystem"
#define WIFI_PASSWORD "12345678"

// Firebase Credentials

#define API_KEY "AIzaSyCZsHJnSoTnZQkTK_ia2ZBD-ZEoJW09thM"
#define DATABASE_URL "smartenergycontrolsystem-default-rtdb.firebaseio.com"
#define USER_EMAIL "benjaminsebastian4db@gmail.com"
#define USER_PASSWORD "database4benjamin"

// Constants and Global Variables
#define WIFI_CHECK_INTERVAL 5000
unsigned long lastWifiCheck = 0;
bool lastWifiState = false;  
int SprayDelay=0;
bool SystemStatus=false;
int setDoshaRelay=-1;
String CurrentUser="";
String Dosha = "";
unsigned long previousMillis = 0; 
const unsigned long displayInterval = 2000; 
int displayState = 0; 
int SprayFromDb=0;

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
  lcd.setCursor(0,1);
  lcd.print(error);
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
        } else {
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
}
int getDoshaRelay(String dosha){
  if (dosha == "Pitta") {
    return RELAY_P;
} else if (dosha == "Vata") {
    return RELAY_V;
} else if (dosha == "Kapha") {
    return RELAY_K;
} else if (dosha == "PittaVata") {
    return RELAY_PV;
} else if (dosha == "PittaKapha") {
    return RELAY_PK;
} else if (dosha == "VataKapha") {
    return RELAY_VK;
}else{
  return-1;
}

}
void controlRelays(){
  if(setDoshaRelay!=-1){
  digitalWrite(setDoshaRelay,HIGH);
  digitalWrite(RELAY_FAN,HIGH);
  delay(5000);
   digitalWrite(setDoshaRelay,LOW);
  digitalWrite(RELAY_FAN,LOW);}
  else{
    errorLcdDisplay(" Dosha not set");
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
        } else if (Firebase.RTDB.getInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/SprayDelay")) {
            SprayDelay = (fbdo.to<int>()) * 60000;
        }
         else if (Firebase.RTDB.getString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/User")) {
            CurrentUser = fbdo.to<String>();
        
        } else if (Firebase.RTDB.getInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Spray")) {
            SprayFromDb = fbdo.to<int>();
            if(SprayFromDb==1){
              controlRelays();
              Firebase.RTDB.setInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Spray",0);
            }
        }
      Serial.print("Fetched Settings => \n");
      Serial.print("- System Status :");
      Serial.print(SystemStatus);
      Serial.print("\n- Dosha :");
      Serial.print(Dosha);
      Serial.print("\n- Spray Interval :");
      Serial.print(SprayDelay/600000);
      Serial.print(" Minutes or ");
      Serial.print(SprayDelay);
      Serial.println(" MilliSeconds");


      } 
    }
}



void resetFirebaseNodes() {
    if (WiFi.status() == WL_CONNECTED) {
        Firebase.RTDB.setString(&fbdo, String("/Data/Devices/")+DEVICE_NUMBER, "");
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
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Prakruthi Based");
    lcd.setCursor(0, 1);
    lcd.print(" Aroma Diffuser");
    delay(4000);
    lcd.clear();
    
    connectWiFi();
    initFirebase();
    resetFirebaseNodes();
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
        displayState = 1;
        break;

      case 1:
        lcd.setCursor(0, 0);
        lcd.print("Dosha:");
        lcd.print(Dosha);
        lcd.setCursor(0, 1);
        lcd.print("Delay: ");
        lcd.print(SprayDelay);
        lcd.print("mins");

        displayState = 2;
        break;

      case 2:
        lcd.setCursor(0, 1);
        lcd.print("Next in: ");
        lcd.print(timeLeft);
        lcd.print("mins");

        displayState = 0;
        break;
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();
  int switchState=digitalRead(MANNUAL_SPRAY_SWITCH);
 if(currentMillis>=SprayDelay||switchState==LOW){
 controlRelays();
  timeClient.update();
   Firebase.RTDB.setString(&fbdo, String("/Data/Devices/")+DEVICE_NUMBER+"/LastUpdate", timeClient.getFormattedTime());
  
 SprayDelay = currentMillis +SprayDelay;
 }
unsigned long timeLeft = (SprayDelay > currentMillis) ? (SprayDelay - currentMillis) : 0;
  timeLeft=timeLeft/60000;
  

  if (currentMillis - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
            updateWiFiLED();
            fetchSettings();
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
        
        lastWifiCheck = currentMillis;
    }
 handleDisplay(currentMillis,timeLeft);

}



