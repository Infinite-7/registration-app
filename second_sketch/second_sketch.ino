#include <Wire.h>
#include <Adafruit_PN532.h>
#include <LiquidCrystal_PCF8574.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <time.h>

// Replace these with your WiFi credentials
#define WIFI_SSID "Galaxy A04e00c1"
#define WIFI_PASSWORD "kaglobume"

// Firebase project credentials
#define FIREBASE_HOST "https://class-registration-system-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "SocggNqpXzmPD480KZblFlDiMIef7t9qeru605Se"

#define SDA_PIN 26
#define SCL_PIN 27

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);
LiquidCrystal_PCF8574 lcd(0x27); // Use the correct LiquidCrystal_PCF8574 library

// Initialize Firebase data object
FirebaseData firebaseData;
FirebaseAuth firebaseAuth;
FirebaseConfig firebaseConfig;

// Time zone for Uganda (EAT, UTC+3, no daylight saving time)
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 10800;
const int   daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Hello!");

  Wire.begin(SDA_PIN, SCL_PIN);

  lcd.begin(16, 2); // initialize the lcd
  lcd.setBacklight(255); // set the backlight brightness to maximum
  lcd.setCursor(0, 0);
  lcd.print("Hello!");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Set the host and authentication details for Firebase
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  
  // Initialize Firebase
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);

  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  struct tm timeinfo; // Declare timeinfo variable here
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println(&timeinfo, "Time: %Y-%m-%d %H:%M:%S");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board");
    lcd.setCursor(0, 1);
    lcd.print("PN53x not found");
    while (1); // halt
  }

  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  lcd.setCursor(0, 1);
  lcd.print("Found PN5");
  lcd.print((versiondata >> 24) & 0xFF, HEX);
  lcd.print(" V");
  lcd.print((versiondata >> 16) & 0xFF, DEC);
  lcd.print(".");
  lcd.print((versiondata >> 8) & 0xFF, DEC);

  nfc.SAMConfig();

  Serial.println("Waiting for an NFC card ...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for NFC");
}

void loop() {
  readNFC();
}

void readNFC() {
  uint8_t success;
  uint8_t uid[7] = {0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("Found an NFC tag!");

    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    String uidString = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(" 0x");
      Serial.print(uid[i], HEX);
      uidString += String(uid[i], HEX);
    }
    Serial.println("");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("NFC Tag Found!");
    lcd.setCursor(0, 1);
    lcd.print("UID: ");
    lcd.print(uidString);

    if (isRegistered(uidString)) {
      handleClock(uidString);
      handleBiometrics(uidString);
    } else {
      Serial.println("Tag not registered.");
      promptRegistration(uidString);
      lcd.setCursor(0, 1);
      lcd.print("Not Registered");
      promptRegistration(uidString);
    }

    delay(1000);
  }
}

bool isRegistered(String uid) {
  String path = "/uids/" + uid;
  return Firebase.get(firebaseData, path) && firebaseData.dataType() != "null";
}

void promptRegistration(String uid) {
  Serial.println("Enter name: ");
  while (Serial.available() == 0) {}
  String name = Serial.readStringUntil('\n');

  Serial.println("Enter Student number: ");
  while (Serial.available() == 0) {}
  String number = Serial.readStringUntil('\n');

  registerTag(uid, name, number);
  Serial.println("Tag registered successfully.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tag Registered!");
}

void registerTag(String uid, String name, String number) {
  String path = "/uids/" + uid;
  Firebase.setString(firebaseData, path + "/state", "clocked out");
  Firebase.setString(firebaseData, path + "/name", name);
  Firebase.setString(firebaseData, path + "/number", number);
  Firebase.setString(firebaseData, path + "/biometrics/", "/denied/");
}

void handleClock(String uid) {
  String path = "/uids/" + uid + "/state";
  Firebase.get(firebaseData, path);
  String state = firebaseData.stringData();

  if (state == "clocked out") {
    clockIn(uid);
  } else if (state == "clocked in") {
    clockOut(uid);
  }
}

void clockIn(String uid) {
  String path = "/uids/" + uid + "/state";
  Firebase.set(firebaseData, path, "clocked in");

  String timestampPath = "/uids/" + uid + "/clockin_time";
  String timestamp = getFormattedTime();
  Firebase.set(firebaseData, timestampPath, timestamp);

  Serial.println("Clocked in");
  lcd.setCursor(0, 1);
  lcd.print("Clocked In");
}

void clockOut(String uid) {
  String path = "/uids/" + uid + "/state";
  Firebase.set(firebaseData, path, "clocked out");

  String timestampPath = "/uids/" + uid + "/clockout_time";
  String timestamp = getFormattedTime();
  Firebase.set(firebaseData, timestampPath, timestamp);

  Serial.println("Clocked out");
  lcd.setCursor(0, 1);
  lcd.print("Clocked Out");
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "N/A";
  }
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}

void handleBiometrics(String uid) {
  String path ="/uids/" + uid;
  Firebase.set(firebaseData, path + "/biometrics/", "/allowed/");
  delay(300000);
  Firebase.set(firebaseData, path + "/biometrics/", "/denied/");
}