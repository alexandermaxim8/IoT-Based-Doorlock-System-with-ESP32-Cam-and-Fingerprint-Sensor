#include <Adafruit_Fingerprint.h>
#include <String.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "time.h"
#include "sntp.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <GSheet32.h>

#define mySerial Serial2
#define button1 12
#define button2 14
#define button3 2
#define buzzer 15
#define lockPin 25


GSheet32 Sheet("AKfycbyUwB69dxeQmiLGtV3X1BxPAqJlTb6OWWHlKHOQxAAoMUI4mRDcnCWe4wPXPF_hRs7J");
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

const char* ssid       = "314";
const char* password   = "1020304050";

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;

const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3"; 

uint8_t id;
int state, i;
int counter=0;
String name[20];

void IRAM_ATTR function_ISR() {
  ESP.restart();
}

void setup()
{
  Serial.begin(115200);
  attachInterrupt(button3, function_ISR, RISING);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(lockPin, OUTPUT);
  while (!Serial);  
  delay(100);

  lcd.init();
  lcd.clear();         
  lcd.backlight();     

  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
  lcd.print("Hello:)");
  lcd.setCursor(0,1);
  delay(1000);
  lcd.clear();

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    lcd.setCursor(0,0);   
    lcd.print("Found Sensor!");
    delay(1000);
    lcd.clear();
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.setCursor(0,0);   
    lcd.print("Sensor Problem");
    delay(1000);
    lcd.setCursor(0,1);   
    lcd.print("Restarting");
    lcd.setCursor(10,1);
    while(counter<3){
      lcd.print(".");
      delay(1000);
      counter+=1;
    }
    counter=0;
    lcd.clear();
    ESP.restart();
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  Serial.setTimeout(100);

  sntp_set_time_sync_notification_cb( timeavailable );
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  lcd.setCursor(0,0);  
  lcd.print("Wifi");
  lcd.setCursor(0,1);
  lcd.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    lcd.setCursor(10,1);
    while(counter<3){
      delay(1000);
      lcd.print(".");
      counter+=1;
    }
    delay(1000);
    lcd.setCursor(10,1);
    lcd.print("   ");
    counter=0;
  }
  lcd.clear();
  lcd.setCursor(0,0);   
  lcd.print("Wifi");
  lcd.setCursor(0,1);   
  lcd.print("Connected!");
  delay(1000);
  lcd.clear();

  if(!SD.begin()){
        Serial.println("Card Mount Failed");
        lcd.print("SD Card Mount");
        lcd.setCursor(0,1); 
        lcd.print("Failed!");
        delay(1000);
        lcd.clear();
        lcd.print("Restarting");
        lcd.setCursor(10,0);
        while(counter<3){
          lcd.print(".");
          delay(1000);
          counter+=1;
        }
        counter=0;
        lcd.clear();
        ESP.restart();
    }
  lcd.print("SD Card");
  lcd.setCursor(0,1);
  lcd.print("Connected!");
  delay(1000);
  lcd.clear();
  lcd.print("Getting data");
  while(counter<3){
    lcd.print(".");
    delay(1000);
    counter+=1;
  }
  counter=0;
  while(counter<20){
    readFromLine(SD, "/FP_data.txt", counter+1, name);
    counter+=1;
  }
  counter=0;

  Sheet.connectWiFi(ssid, password);
  lcd.clear();
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  Serial.read();
  return num;
}

void loop()                     // run over and over again
{
  printLocalTime(); 
  state=0;
  if(digitalRead(button1)==HIGH && digitalRead(button2)==LOW){
    state=1;
  }
  else if(digitalRead(button1)==LOW && digitalRead(button2)==HIGH){
    state=2;
  }
  fingerprint();
  delay(100);
  if(state==1){
    enroll();
    delay(1000);
  }
  else if(state==2){
    delete_();
    delay(1000);
  }
  delay(100);
}

void enroll(){
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  lcd.clear();
  lcd.print("Enter ID");
  lcd.setCursor(0,1);
  lcd.print("(via IDE)");
  id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);
  lcd.clear();
  lcd.print("Place your");
  lcd.setCursor(0,1);
  lcd.print("finger");
  while (!  getFingerprintEnroll() );
}

void fingerprint(){
  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
  getFingerprintID();
}

void delete_(){
  Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
  lcd.clear();
  lcd.print("Enter ID");
  lcd.setCursor(0,1);
  lcd.print("(via IDE)");
  uint8_t id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }

  Serial.print("Deleting ID #");
  Serial.println(id);

  deleteFingerprint(id);
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      finger_err();
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      finger_err();
      break;
    default:
      Serial.println("Unknown error");
      finger_err();
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      finger_process();
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      finger_err();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      finger_err();
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      finger_err();
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      finger_err();
      return p;
    default:
      Serial.println("Unknown error");
      finger_err();
      return p;
  }

  Serial.println("Remove finger");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Remove finger");
  delay(2000);
  lcd.clear();
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcd.setCursor(0,0);
  lcd.print("Place same");
  lcd.setCursor(0,1);
  lcd.print("finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      finger_err();
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      finger_err();
      break;
    default:
      Serial.println("Unknown error");
      finger_err();
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      finger_err();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      finger_err();
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      finger_err();
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      finger_err();
      return p;
    default:
      Serial.println("Unknown error");
      finger_err();
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    finger_err();
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    finger_err();
    return p;
  } else {
    Serial.println("Unknown error");
    finger_err();
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    finger_process();
    lcd.clear();
    lcd.print("Enter name:");
    lcd.setCursor(0,1);
    lcd.print("(via IDE)");
    Serial.println("Input your name: ");
    Serial.print(Serial.available());
    delay(1000);
    while(Serial.available() == 0){
      delay(50);
    }
    name[id-1] = Serial.readStringUntil('\n');

    Serial.print(id); Serial.print(" ");
    Serial.print(name[id-1]); Serial.println(" Stored!");
    lcd.clear();
    lcd.print("#"); lcd.print(id);
    lcd.setCursor(0,1);
    lcd.print(name[id-1]);
    lcd.clear();
    writeFile(SD, "/FP_data.txt", name);
    lcd.print("Stored!");
    delay(1000);
    tone(buzzer, 450);
    delay(200);
    noTone(buzzer);

    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    finger_err();
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    finger_err();
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    finger_err();
    return p;
  } else {
    Serial.println("Unknown error");
    finger_err();
    return p;
  }

  return true;
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      finger_process();
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      finger_err_detect();
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      finger_err_detect();
      return p;
    default:
      Serial.println("Unknown error");
      finger_err_detect();
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      finger_err_detect();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      finger_err_detect();
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      finger_err_detect();
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      finger_err_detect();
      return p;
    default:
      Serial.println("Unknown error");
      finger_err_detect();
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    Serial.println(name[finger.fingerID-1]);
    sendtoSheet(name[finger.fingerID-1], finger.fingerID, finger.confidence);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Hello "); lcd.print("#"); lcd.print(finger.fingerID);
    lcd.setCursor(0,1);
    lcd.print(name[finger.fingerID-1]);
    tone(buzzer, 450);
    delay(200);
    noTone(buzzer);
    delay(1000);
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" "); Serial.print(name[finger.fingerID]);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Closing back in:");
    digitalWrite(lockPin, HIGH);
    for(i = 8; i>=0; i--){
      lcd.setCursor(0,1);
      lcd.print(i);
      delay(1000);
    }
    digitalWrite(lockPin, LOW);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    finger_err_detect();
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    finger_err_detect();
    return p;
  } else {
    Serial.println("Unknown error");
    finger_err_detect();
    return p;
  }

  // found a match!
  
  lcd.clear();
  delay(1000);

  return finger.fingerID;
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.print(id); Serial.print(" ");
    Serial.print(name[id-1]); Serial.println(" Deleted!");
    lcd.clear();
    lcd.print("#"); lcd.print(id);
    lcd.setCursor(0,1);
    lcd.print(name[id-1]);
    delay(1500);
    name[id-1]="";
    lcd.clear();
    lcd.print("Deleted!");
    writeFile(SD, "/FP_data.txt", name);
    tone(buzzer, 450);
    delay(200);
    noTone(buzzer);
    delay(1000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    finger_err();
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    finger_err();
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    finger_err();
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    finger_err();
  }

  return p;
}

// returns -1 if failed, otherwise returns ID #

void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Time not");
    lcd.setCursor(0,1);
    lcd.print("available.");
    lcd.clear();
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M");
  lcd.setCursor(0,0);
  lcd.print(&timeinfo, "%b %d %Y");
  lcd.setCursor(0,1);
  lcd.print(&timeinfo, "%a, %H:%M");
}

void finger_err(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Error");
  lcd.setCursor(0,1);
  lcd.print("Occured!");
  delay(1500);
  lcd.clear();
}

void finger_err_detect(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Error");
  lcd.setCursor(0,1);
  lcd.print("Occured!");
  delay(1500);
  sendtoSheet("Error", -1, 0);
  lcd.clear();
}

void finger_process(){
  lcd.clear();
  lcd.setCursor(0,0);  
  lcd.print("Processing");
  lcd.setCursor(0,1);
  lcd.print("fingerprint");
  lcd.setCursor(11,1);
  while(counter<3){
    lcd.print(".");
    delay(1000);
    counter+=1;
  }
  counter=0;
}

void readFromLine(fs::FS &fs, const char * path, size_t lineNumber, String name[20]){
    Serial.printf("Reading from line %u of file: %s\n", lineNumber, path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    size_t currentLine = 1;  // Track the current line number
    while(file.available()){
        String line = file.readStringUntil('\n');
        if(currentLine == lineNumber){
            name[lineNumber-1]=line;
            Serial.println(name[lineNumber-1]);
            break;
        }
        currentLine++;
    }

    file.close();
}

void writeFile(fs::FS &fs, const char * path, String name[20]){
    Serial.printf("Writing file: %s\n", path);

    int counter=0;
    String temp;

    for(counter=0; counter<20; counter++){
      temp=temp+name[counter]+"\n";
    }
    Serial.println(temp);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(temp)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void sendtoSheet(String name, int id, int confidence){
  String name_sheet, id_sheet, time_minute, confidence_level;
  name_sheet=name;
  name_sheet.replace(" ", "%20");
  id_sheet=String(id);
  time_minute=String(millis()/60000);
  confidence_level=String(confidence);
  Sheet.sendData(time_minute, id_sheet, name_sheet, confidence_level);
}
