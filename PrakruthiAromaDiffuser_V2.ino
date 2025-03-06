/***
Lol this is AI optimized version of code version 1 i.e PrakruthiAromaDiffuser.ino
***/
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
#define USER_PASSWORD "****************"

// Constants and Global Variables
#define WIFI_CHECK_INTERVAL 5000
#define ERROR_DISPLAY_TIME 2000
#define MAX_WIFI_ATTEMPTS 20
#define SPRAY_DURATION 5000

unsigned long lastWifiCheck = 0;
bool lastWifiState = false;  
int SprayDelay = 0;
bool SystemStatus = false;
int setDoshaRelay = -1;
String CurrentUser = "";
String Dosha = "";
unsigned long previousMillis = 0; 
const unsigned long displayInterval = 2000; 
int displayState = 0; 
int SprayFromDb = 0;

// Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

// Pins
#define RELAY_P 13         
#define RELAY_K 12         
#define RELAY_V 14         
#define RELAY_PK 27        
#define RELAY_PV 26        
#define RELAY_VK 25        
#define RELAY_FAN 33       
#define WIFI_STATUS_LED 2  
#define MANNUAL_SPRAY_SWITCH 34 

void errorLcdDisplay(String error) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Error Occurred!");
    lcd.setCursor(0,1);
    lcd.print(error);
    delay(ERROR_DISPLAY_TIME);
    lcd.clear();
}

bool connectWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting to Wi-Fi...");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_WIFI_ATTEMPTS) {
        digitalWrite(WIFI_STATUS_LED, !digitalRead(WIFI_STATUS_LED));
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        errorLcdDisplay("WiFi Failed!");
        return false;
    }
    
    lastWifiState = true;
    digitalWrite(WIFI_STATUS_LED, HIGH);
    lcd.clear();
    lcd.print("WiFi Connected!");
    delay(2000);
    return true;
}

bool initFirebase() {
    config.api_key = API_KEY;
        auth.user.email = USER_EMAIL;
        auth.user.password = USER_PASSWORD;
        config.database_url = DATABASE_URL;
        Firebase.begin(&config, &auth);
    if (!Firebase.ready()) {
        errorLcdDisplay("Firebase Failed!");
        return false;
    }
    
    return true;
}

int getDoshaRelay(String dosha) {
    if (dosha == "Pitta") return RELAY_P;
    if (dosha == "Vata") return RELAY_V;
    if (dosha == "Kapha") return RELAY_K;
    if (dosha == "PittaVata") return RELAY_PV;
    if (dosha == "PittaKapha") return RELAY_PK;
    if (dosha == "VataKapha") return RELAY_VK;
    return -1;
}

bool controlRelays() {
    if (setDoshaRelay == -1) {
        errorLcdDisplay("Invalid Dosha!");
        return false;
    }
    
    digitalWrite(setDoshaRelay, HIGH);
    digitalWrite(RELAY_FAN, HIGH);
    delay(SPRAY_DURATION);
    digitalWrite(setDoshaRelay, LOW);
    digitalWrite(RELAY_FAN, LOW);
    return true;
}

bool fetchSettings() {
    bool success = false;
    
    // Fetch System Status
    if (Firebase.RTDB.getString(&fbdo, String("/Data/Devices/") + String(DEVICE_NUMBER) + "/Status")) {
        SystemStatus = (fbdo.to<String>() == "Active");
        success = true;
    }

    if (SystemStatus) {
        // Fetch Dosha
        if (Firebase.RTDB.getString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Dosha")) {
            Dosha = fbdo.to<String>();
            setDoshaRelay = getDoshaRelay(Dosha);
            success = true;
        }
        
        // Fetch Spray Delay
        if (Firebase.RTDB.getInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/SprayDelay")) {
            SprayDelay = (fbdo.to<int>()) * 60000;
            success = true;
        }
        
        // Fetch User
        if (Firebase.RTDB.getString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/User")) {
            CurrentUser = fbdo.to<String>();
            success = true;
        }
        
        // Check for manual spray trigger
        if (Firebase.RTDB.getInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Spray")) {
            SprayFromDb = fbdo.to<int>();
            if (SprayFromDb == 1) {
                if (controlRelays()) {
                    Firebase.RTDB.setInt(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/Spray", 0);
                }
            }
        }
    }
    
    return success;
}

bool checkSettings() {
    if (!SystemStatus) {
        errorLcdDisplay("System Inactive");
        return false;
    }
    
    if (Dosha.isEmpty() || setDoshaRelay == -1) {
        errorLcdDisplay("Invalid Dosha!");
        return false;
    }
    
    if (SprayDelay == 0) {
        errorLcdDisplay("No Spray Time!");
        return false;
    }
    
    return true;
}

void handleDisplay(unsigned long currentMillis, unsigned long timeLeft) {
    if (currentMillis - previousMillis >= displayInterval) {
        previousMillis = currentMillis;
        lcd.clear();
        
        switch (displayState) {
            case 0:
                lcd.setCursor(0, 0);
                lcd.print("User: ");
                lcd.print(CurrentUser.isEmpty() ? "None" : CurrentUser);
                lcd.setCursor(0, 1);
                lcd.print("Status: ");
                lcd.print(SystemStatus ? "Active" : "Inactive");
                displayState = 1;
                break;

            case 1:
                lcd.setCursor(0, 0);
                lcd.print("Dosha: ");
                lcd.print(Dosha);
                lcd.setCursor(0, 1);
                lcd.print("Delay: ");
                lcd.print(SprayDelay / 60000);
                lcd.print("min");
                displayState = 2;
                break;

            case 2:
                lcd.setCursor(0, 0);
                lcd.print(WiFi.status() == WL_CONNECTED ? "Online" : "Offline");
                lcd.setCursor(0, 1);
                lcd.print("Next: ");
                lcd.print(timeLeft);
                lcd.print("min");
                displayState = 0;
                break;
        }
    }
}

void setup() {
    Serial.begin(9600);
    
    // Initialize pins
    pinMode(RELAY_P, OUTPUT);
    pinMode(RELAY_K, OUTPUT);
    pinMode(RELAY_V, OUTPUT);
    pinMode(RELAY_PK, OUTPUT);
    pinMode(RELAY_PV, OUTPUT);
    pinMode(RELAY_VK, OUTPUT);
    pinMode(RELAY_FAN, OUTPUT);
    pinMode(WIFI_STATUS_LED, OUTPUT);
    pinMode(MANNUAL_SPRAY_SWITCH, INPUT_PULLUP);
    
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Prakruthi Based");
    lcd.setCursor(0, 1);
    lcd.print("Aroma Diffuser");
    delay(4000);
    lcd.clear();
    
    // Initialize connections and fetch data
    bool wifiConnected = false;
    bool firebaseInitialized = false;
    
    // Try to connect to WiFi and Firebase multiple times
    for (int i = 0; i < 3; i++) {
        lcd.clear();
        lcd.print("Attempt ");
        lcd.print(i + 1);
        lcd.print("/3");
        
        if (connectWiFi()) {
            wifiConnected = true;
            if (initFirebase()) {
                firebaseInitialized = true;
                timeClient.begin();
                if (fetchSettings()) {
                    break;
                }
            }
        }
        delay(2000);
    }
    
    if (!wifiConnected || !firebaseInitialized || !checkSettings()) {
        errorLcdDisplay("Init Failed!");
        delay(3000);
        ESP.restart();
    }
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Check WiFi and try to reconnect if necessary
    if (currentMillis - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
        if (WiFi.status() != WL_CONNECTED) {
            digitalWrite(WIFI_STATUS_LED, LOW);
            if (connectWiFi()) {
                fetchSettings();
            }
        }
        lastWifiCheck = currentMillis;
    }
    
    // Only proceed if settings are valid
    if (!checkSettings()) {
        delay(ERROR_DISPLAY_TIME);
        return;
    }
    
    // Handle manual spray switch
    if (digitalRead(MANNUAL_SPRAY_SWITCH) == LOW) {
        if (controlRelays() && WiFi.status() == WL_CONNECTED) {
            timeClient.update();
            Firebase.RTDB.setString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/LastUpdate", 
                                  timeClient.getFormattedTime());
        }
    }
    
    // Handle automatic spray based on delay
    if (currentMillis >= SprayDelay) {
        if (controlRelays()) {
            if (WiFi.status() == WL_CONNECTED) {
                timeClient.update();
                Firebase.RTDB.setString(&fbdo, String("/Data/Devices/") + DEVICE_NUMBER + "/LastUpdate", 
                                      timeClient.getFormattedTime());
            }
            SprayDelay = currentMillis + SprayDelay;
        }
    }
    
    // Update display
    unsigned long timeLeft = (SprayDelay > currentMillis) ? (SprayDelay - currentMillis) / 60000 : 0;
    handleDisplay(currentMillis, timeLeft);
}
