#include <SoftwareSerial.h>
#include <ChainableLED.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include <TinyGPS++.h>
#include <SD.h>

Adafruit_BME280 bme;
TinyGPSPlus gps;
RTC_DS3231 RTC;

#define BUTTON_RED 2
#define BUTTON_GREEN 3
#define MODULELUM A0
#define RX 5 //pin GPS
#define TX 6  //pin GPS

SoftwareSerial GPS(RX, TX);
ChainableLED led(4, 5, 1);

//byte adresse;
bool levier;
byte mode;
bool state = false;
String gps_data;

unsigned long debutRed = 0;
unsigned long debutGreen = 0;
unsigned long configModeStartTime = 0;
unsigned long timerSD;

const unsigned long CONFIGURATION_TIMEOUT = 100000;
unsigned long LOG_INTERVALL = 1000;
unsigned long FILE_MAX_SIZE = 2000;
bool RESET = 0;
//float VERSION = 1.0;
uint8_t revision = 0 ;

bool LUMIN = true ;
bool TEMP_AIR = true;
bool HYGR = true;
bool PRESSURE = true;

uint16_t LUMIN_LOW = 255;
uint16_t LUMIN_HIGH = 255;
int8_t MIN_TEMP_AIR = -10;
int8_t MAX_TEMP_AIR = 60;
int8_t HYGR_MINT = 0;
int8_t HYGR_MAXT = 50;
uint16_t PRESSURE_MIN = 850;
uint16_t PRESSURE_MAX = 1080;

struct CLOCK {
  uint8_t HEURE;
  uint8_t MINUTE;   
  uint8_t SECONDE;  
};

struct DATE {
  uint8_t MOIS; 
  uint8_t JOUR;  
  uint16_t ANNEE; 
};

enum DAY {
  MON,
  TUE,
  WED,
  THU,
  FRI,
  SAT,
  SUN
};

void setup() {
  Serial.begin(9600);
  led.init();
  RTC.begin();
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  mode = 5;
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GREEN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RED), clickButtonRedEvent, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_GREEN), clickButtonGreenEvent, FALLING);

  if (digitalRead(BUTTON_RED) == LOW) {
    configModeStartTime = millis();
    mode = 1; // config mode
  }
}

void loop() {
  switch (mode) {
    case 0: // mode standard
      if (((millis() - debutRed >= 5000) && digitalRead(BUTTON_RED) == 0) && state == false) {
        state = true;
        mode = 2;
      }
      else if ((millis() - debutGreen >= 5000) && digitalRead(BUTTON_GREEN) == 0) {
        mode = 3;
      }
      if (millis() - timerSD >= LOG_INTERVALL) {
        Write(getData());
        timerSD = millis();
      }
      led.setColorRGB(0, 0, 255, 0);
      checkError(0);
      checkError(1);
      checkError(2);
      checkError(3);
      break;

    case 1: // mode config
      if (millis() - configModeStartTime >= CONFIGURATION_TIMEOUT) {
        //Serial.println(CONFIGURATION_TIMEOUT);
        mode = 0;
      }
      led.setColorRGB(0, 255, 200, 0);
      funcConfig();
      break;

    case 2: // mode maintenance
      if (((millis() - debutRed >= 5000) && digitalRead(BUTTON_RED) == 0) && state == false) {
        state = true;
        debutRed = 0;
        mode = 0;
      }
      Serial.print(getData());
      led.setColorRGB(0, 255, 50, 0);
      break;

    case 3: // mode eco
      if (((millis() - debutRed >= 5000) && digitalRead(BUTTON_RED) == 0) && state == false) {
        state = true;
        debutRed = 0;
        mode = 0;
      }
      led.setColorRGB(0, 0, 50, 255);
      LOG_INTERVALL = 2 * LOG_INTERVALL;
      PRESSURE = false;
      TEMP_AIR = false;
      HYGR = false;
      LUMIN = false;
      if (millis() - timerSD >= LOG_INTERVALL) {
        Write(getData());
        timerSD = millis();
      }
      break;

    default: // mode standard
      break;
  }
}

void clickButtonRedEvent() {
  debutRed = millis();
  state = false;
}

void clickButtonGreenEvent() {
  debutGreen = millis();
}

void Write(String valeur) {
  DateTime now = RTC.now();
  int year = now.year();
  int month = now.month();
  int day = now.day();
  String fichier = String(year) + String(month) + String(day) + "0.txt";
  File myFile = SD.open(fichier, FILE_WRITE);
  if (myFile.size() <= FILE_MAX_SIZE) {
    //Serial.print(myFile.size());
    if (myFile) {
      myFile.print(valeur);
      myFile.close();
    }
    /*else {
      Serial.print(F("erreur"));
    }*/
  }
  else {
    //Serial.println(F("fichier plein"));
    myFile.close();
    revision = revision + 1;
    String fileName = String(year) + String(month) + String(day) + String(revision) + ".txt";
    if (SD.exists(fichier)) {
      myFile = SD.open(fichier, FILE_READ);
      if (myFile) {
        //Serial.println(F("source ouvert"));
        //Serial.println(fileName);
        //Serial.println(revision);
        File myFile2 = SD.open(fileName, FILE_WRITE);
        if (SD.exists(fileName)) {
          if (myFile2) {
          //  Serial.println(F("ouvert copie"));
            while (myFile.available()) {
              char data = myFile.read();
              myFile2.write(data);
            }
            myFile.close();
            myFile2.close();
            //Serial.println(F("Ok"));
            SD.remove(fichier);
          }
          else {
            //Serial.println(F("Pas ouverture de copie"));
            if (SD.exists(fileName)) {
              //Serial.println(F("copie existante"));
            }
            else {
              //Serial.println(F("copie pas existante"));
            }
          }
        }
        else {
          //Serial.println(F("pas présent"));
        }
      }
      else {
        //Serial.println(F("Pas de fichier source"));
      }
    }
  }
}

String getGPS() {
  if (GPS.available()) {
    char c = GPS.read();
    if (c == '$') {
      gps_data = "";
    }
    gps_data += c;
    if (c == '\n') {
      if (gps_data.startsWith("$GPRMC")) {
        return gps_data;
      }
    }
  }
}

String getData() {
  String text;
  char heure[10];
  DateTime now = RTC.now();
  bme.begin(0x76);
  if (LUMIN == true) {
    text += "Luminosité : ";
    text += analogRead(MODULELUM);
    text += "\n";
  }
  if (TEMP_AIR == true) {
    text += "Température : ";
    text += bme.readTemperature();
    text += "\n";
  }
  if (PRESSURE == true) {
    text += "Pression : ";
    text += bme.readPressure();
    text += "\n";
  }
  if (HYGR == true) {
    text += "Hygormétrie : ";
    text += bme.readHumidity();
    text += "\n";
  }
  text += "GPS : ";
  text += getGPS();
  text += "\n";
  text += "RTC : ";
  sprintf(heure, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  text += heure;
  text += "\n";
  return text;
}

bool checkError(byte i) {
  switch (i) {
    case 0: // check rtc access
      if (!RTC.begin()) {
        ledError(i);
        return 1;
      }
      else {
        RTC.begin();
        return 0;
      }
      break;

    case 1: // check gps access
      if (!GPS.available()) {
        ledError(i);
        return 1;
      }
      else {
        GPS.available();
        return 0;
      }
      break;

    case 2: // check captor access
      if (!bme.begin(0x76)) {
        ledError(i);
        return 1;
      }
      else {
        bme.begin(0x76);
        return 0;
      }
      break;

    case 3: // check data
      if ((bme.readHumidity() < HYGR_MINT) || (bme.readHumidity() > HYGR_MAXT)) {
        ledError(i);
        return 1;
      }
      else if ((bme.readPressure() < PRESSURE_MIN) || (bme.readPressure() > PRESSURE_MAX)) {
        ledError(i);
        return 1;
      }
      else if ((bme.readTemperature() < MIN_TEMP_AIR) || (bme.readTemperature() > MAX_TEMP_AIR)) {
        ledError(i);
        return 1;
      }
      else if ((analogRead(MODULELUM) < LUMIN_LOW) || analogRead(MODULELUM) > LUMIN_HIGH) {
        ledError(i);
        return 1;
      }
      else {
        return 0;
      }
      break;

    /*case 4: // check SD full

    File root = SD.open("/"); // Changez le chemin selon votre configuration

    int fileCount = countFilesWithSubstring(root, "log");
    
    if (fileCount >= 3000000){
      Serial.println(F("pu de place"));
    } else {
      Serial.println(F("t large frro tu pe etkrir")); 
    }


    root.close();
    break;*/


    case 5: // check SD access
      if (!SD.begin(4)) {
        ledError(i);
        return 1;
      }
      else {
        return 0;
      }
      break;

    default:
      break;
  }
}


void ledError(byte i) {
  switch (i) {
    case 0:

      if (((millis() - configModeStartTime) >= 1000)) {
        if (levier == true) {
          led.setColorRGB(0, 255, 0, 0);
          levier = false;
          configModeStartTime = millis();
        }
        else {
          led.setColorRGB(0, 0, 0, 255);
          levier = true;
          configModeStartTime = millis();
        }
      }
      break;

    case 1:
      if (((millis() - configModeStartTime) >= 1000)) {
        if (levier == true) {
          led.setColorRGB(0, 255, 0, 0);
          levier = false;
          configModeStartTime = millis();
        }
        else {
          led.setColorRGB(0, 255, 200, 0);
          levier = true;
          configModeStartTime = millis();
        }
      }
      break;

    case 2:
      if (((millis() - configModeStartTime) >= 1000)) {
        if (levier == true) {
          led.setColorRGB(0, 255, 255, 255);
          levier = false;
          configModeStartTime = millis();
        }
        else if (((millis() - configModeStartTime) >= 2000)) {
          led.setColorRGB(0, 255, 0, 0);
          levier = true;
          configModeStartTime = millis();
        }
      }
      break;

    case 3:
      if (((millis() - configModeStartTime) >= 1000)) {
        if (levier == true) {
          led.setColorRGB(0, 255, 0, 0);
          levier = false;
          configModeStartTime = millis();
        }
        else {
          led.setColorRGB(0, 255, 255, 255);
          levier = true;
          configModeStartTime = millis();
        }
      }
      break;

    case 4:
      if (((millis() - configModeStartTime) >= 1000)) {
        if (levier == true) {
          led.setColorRGB(0, 255, 0, 0);
          levier = false;
          configModeStartTime = millis();
        }
        else {
          led.setColorRGB(0, 0, 255, 0);
          levier = true;
          configModeStartTime = millis();
        }
      }
      break;

    case 5:
      if (((millis() - configModeStartTime) >= 1000)) {
        if (levier == true) {
          led.setColorRGB(0, 0, 255, 0);
          levier = false;
          configModeStartTime = millis();
        }
        else if (((millis() - configModeStartTime) >= 2000)) {
          led.setColorRGB(0, 255, 0, 0);
          levier = true;
          configModeStartTime = millis();
        }
      }
      break;

    default:
      break;
  }
}


void funcConfig() 
{
  if (Serial.available() > 0) 
  {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("LOG_INTERVALL=")) 
    {
      LOG_INTERVALL = command.substring(14).toInt();
      //Serial.print("LOG_INTERVALL maj: ");
      Serial.println(LOG_INTERVALL);
    } else if (command.startsWith("FILE_MAX_SIZE=")) 
    {
      FILE_MAX_SIZE = command.substring(13).toInt();
      //Serial.print("FILE_MAX_SIZE maj: ");
      Serial.println(FILE_MAX_SIZE);
    } else if (command.startsWith("LUMIN=")) 
    {
      LUMIN = command.substring(6).toInt();
      //Serial.print("LUMIN maj: ");
      Serial.println(LUMIN);
    } else if (command.startsWith("LUMIN_LOW=")) 
    {
      LUMIN_LOW = command.substring(10).toInt();
      //Serial.print("LUMIN_LOW maj: ");
      Serial.println(LUMIN_LOW);
    } else if (command.startsWith("LUMIN_HIGH=")) 
    {
      LUMIN_HIGH = command.substring(11).toInt();
      //Serial.print("LUMIN_HIGH maj: ");
      Serial.println(LUMIN_HIGH);
    } else if (command.equals("RESET")) 
    {
      LOG_INTERVALL = 2000;  
      FILE_MAX_SIZE = 2000;  
      LUMIN = 1;           
      LUMIN_LOW = 200;       
      LUMIN_HIGH = 700;
      TEMP_AIR = 0;        
      MIN_TEMP_AIR = -5;    
      MAX_TEMP_AIR = 30;   
      HYGR = 1;             
      HYGR_MINT = 0;        
      HYGR_MAXT = 50;       
      PRESSURE = 0;        
      PRESSURE_MIN = 450;   
      PRESSURE_MAX = 1030;  
      //Serial.println("Paramètres réinitialisés aux valeurs par défaut.");
    } else if (command.equals("VERSION")) 
    {
      //Serial.println("Version du programme: 1.0");
    } else if (command.startsWith("TEMP_AIR=")) 
    {
      TEMP_AIR = command.substring(9).toInt();
      //Serial.print("TEMP_AIR maj: ");
      Serial.println(TEMP_AIR);
    } else if (command.startsWith("MIN_TEMP_AIR=")) 
    {
      MIN_TEMP_AIR = command.substring(13).toInt();
      //Serial.print("MIN_TEMP_AIR maj: ");
      Serial.println(MIN_TEMP_AIR);
    } else if (command.startsWith("MAX_TEMP_AIR=")) 
    {
      MAX_TEMP_AIR = command.substring(13).toInt();
      //Serial.print("MAX_TEMP_AIR maj: ");
      Serial.println(MAX_TEMP_AIR);
    } else if (command.startsWith("HYGR=")) 
    {
      HYGR = command.substring(5).toInt();
      //Serial.print("HYGR maj: ");
      Serial.println(HYGR);
    } else if (command.startsWith("HYGR_MINT=")) 
    {
      HYGR_MINT = command.substring(10).toInt();
      //Serial.print("HYGR_MINT maj: ");
      Serial.println(HYGR_MINT);
    } else if (command.startsWith("HYGR_MAXT=")) 
    {
      HYGR_MAXT = command.substring(10).toInt();
      //Serial.print("HYGR_MAXT maj: ");
      Serial.println(HYGR_MAXT);
    } else if (command.startsWith("PRESSURE=")) 
    {
      PRESSURE = command.substring(10).toInt();
      //Serial.print("PRESSURE maj: ");
      Serial.println(PRESSURE);
    } else if (command.startsWith("PRESSURE_MIN=")) 
    {
      PRESSURE_MIN = command.substring(14).toInt();
      //Serial.print(F("PRESSURE_MIN maj: "));
      Serial.println(PRESSURE_MIN);
    } else if (command.startsWith("PRESSURE_MAX=")) 
    {
      PRESSURE_MAX = command.substring(14).toInt();
      //Serial.print(F("PRESSURE_MAX maj: "));
      Serial.println(PRESSURE_MAX);
    } else 
    {
      //Serial.println(F("Commande non reconnue."));
    }
  }
}
