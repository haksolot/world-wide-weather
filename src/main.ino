#include <SoftwareSerial.h>
#include <ChainableLED.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include <TinyGPS++.h>
#include <SD.h>
#include <EEPROM.h>

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

byte adresse;
bool levier;
int var;
byte mode;
bool state = false;
String gps_data;

unsigned long debutRed = 0;
unsigned long debutGreen = 0;
unsigned long configModeStartTime = 0;
unsigned long timerSD;

const unsigned long CONFIGURATION_TIMEOUT = 100000;
unsigned long LOG_INTERVALL = 1000;
unsigned long FILE_MAX_SIZE = 4096;
bool RESET = 0;
float VERSION = 1.0;
int TIMEOUT = 30;

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

void setup()
{
    Serial.begin(9600);
    led.init();

    RTC.begin();

    mode = 5;

    pinMode(BUTTON_RED, INPUT_PULLUP);
    pinMode(BUTTON_GREEN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_RED), clickButtonRedEvent, FALLING);
    attachInterrupt(digitalPinToInterrupt(BUTTON_GREEN), clickButtonGreenEvent, FALLING);

    if (digitalRead(BUTTON_RED) == LOW)
    {
      configModeStartTime = millis();
      mode = 1; // config mode
    }
}

void loop()
{
    switch (mode)
    {
    case 0: // mode standard
      if (((millis() - debutRed >= 5000) && digitalRead(BUTTON_RED) == 0) && state == false)
        {
            state = true;
            mode = 2;
        }
        else if ((millis() - debutGreen >= 5000) && digitalRead(BUTTON_GREEN) == 0)
        {
            mode = 3;
        }
        
        if (millis()-timerSD >= LOG_INTERVALL)
        {
            Write(getData());
            timerSD = millis();
        }
        led.setColorRGB(0, 0, 255, 0);
      break;

    case 1: // mode config
      if (millis() - configModeStartTime >= CONFIGURATION_TIMEOUT)
      {
        Serial.println(CONFIGURATION_TIMEOUT);
          mode = 0;
      }
      led.setColorRGB(0, 255, 200, 0);
      funcConfig();

      break;

    case 2: // mode maintenance
        if (((millis() - debutRed >= 5000) && digitalRead(BUTTON_RED) == 0) && state == false)
        {
            state = true;
            debutRed = 0;
            mode = 0;
        }
        Serial.print(getData());
        led.setColorRGB(0, 255, 50, 0);
        break;

    case 3: // mode eco
        if (((millis() - debutRed >= 5000) && digitalRead(BUTTON_RED) == 0)&& state == false)
        {
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
        if (millis()-timerSD >= LOG_INTERVALL)
        {
            Write(getData());
            timerSD = millis();
        }
        break;

    default: // mode standard
      checkError();
      ledError();
    break;
    }
}

void clickButtonRedEvent()
{
    debutRed = millis();
    state = false;
}

void clickButtonGreenEvent()
{
    debutGreen = millis();
}

void Write(String valeur){
    DateTime now = RTC.now();
    int year = now.year();
    int month = now.month();
    int day = now.day();
    //String fichier = String(year)+ String(month)+ String(day)+"0.txt";
    File myFile = SD.open("fichier.LOG", FILE_WRITE);
    if (myFile.size()<= FILE_MAX_SIZE)
    {
      Serial.print(myFile.size());
      if (myFile) {
        myFile.print(valeur);
        myFile.close();
      }
      else
      {
        Serial.print(F("erreur"));
      }
    }
    else
    {
      Serial.println(F("Le fichier est plein"));
      myFile.close();
      revision = revision + 1;
      String fileName =  String(year) + String(month) + String(day) + String(revision) + ".txt";
      if (SD.exists("fichier.LOG")) 
      {
        myFile = SD.open("fichier.LOG", FILE_READ);
        if (myFile)
        {
          Serial.println(F("ouvert fichier source"));
          Serial.println(fileName);
          Serial.println(revision);
          File myFile2 = SD.open(fileName, FILE_WRITE);
          if (SD.exists(fileName))
          {
           if (myFile2)
           {
            Serial.println(F("ouvert fichier copie"));
            while (myFile.available())
            {
              char data = myFile.read();
              myFile2.write(data);
            }
            myFile.close();
            myFile2.close();
            Serial.println(F("Ok"));
            SD.remove("fichier.LOG");
           }
           else 
           {
            Serial.println(F("Pas ouverture de copie"));
            if (SD.exists(fileName))
            {
              Serial.println(F("La copie exsite"));
            }
            else
            {
              Serial.println(F("La copie no existe"));
            }
           } 
         }
         else
         {
          Serial.println(F("pas present"));
         }
        }
        else
        {
          Serial.println(F("Pas de fichier source"));
        }
      } 
    }
      
}

String getGPS()
{
    if (GPS.available()) 
    {
    char c = GPS.read();
    if (c == '$') 
    {
      gps_data = "";
    }
    gps_data += c;
    
    if (c == '\n') 
    {
      if (gps_data.startsWith("$GPRMC")) 
      {
        return gps_data;
      }
    }
  }
}

String getData()
{
    String text;
    bme.begin(0x76);
    if(LUMIN == true)
    {
    text += "Luminosité : ";
    text += analogRead(MODULELUM);
    text += "\n";
    }
    if(TEMP_AIR == true)
    {
    text += "Température : ";
    text += bme.readTemperature();
    text += "\n";
    }
    if(PRESSURE == true)
    {
    text += "Pression : ";
    text += bme.readPressure();
    text += "\n";
    }
    if(HYGR == true)
    {
    text += "Hygormétrie : ";
    text += bme.readHumidity();
    text += "\n";
    }
    text += "GPS : ";
    text += getGPS();
    text += "\n";
    text += "RTC : ";
    text += RTC.now();
    text += "\n";
    return text;
}

void checkError(byte i)
{
  switch (i)
  {
  case 0: // check rtc access
    if (! RTC.begin())
    {
      ledError(i);
      return 1;
    }
    else
    {
      RTC.begin();
      return 0;
    }
    break;
  
  case 1: // check gps access
    if (! GPS.available())
    {
      ledError(i);
      return 1;
    }
    else
    {
      GPS.available();
      return 0;
    }
    break;
  
  case 2: // check captor access
    if (! bme.begin(0x76))
    {
      ledError(i);
      return 1;
    }
    else
    {
      bme.begin(0x76);
      return 0;
    }
    break;
  
  case 3: // check data
    if (( bme.readHumidity() < HYGR_MINT )||( bme.readHumidity() > HYGR_MAXT )) // humidité
    {
      ledError(i);
      return 1;
    }
    else if (( bme.readPressure() < PRESSURE_MIN )||( bme.readPressure() > PRESSURE_MAX )) // pression
    {
      ledError(i);
      return 1;
    }
    else if (( bme.readTemperature() < MIN_TEMP_AIR )||( bme.readTemperature() > MAX_TEMP_AIR )) // température
    {
      ledError(i);
      return 1;
    }
    else if () // luminosité
    {
      ledError(i);
      return 1;
    }
    else 
    {
      return 0;
    }
    break;
  
  case 4: // check SD full
    if ()
    {
      ledError(i);
      return 1;
    }
    else 
    {
      return 0;
    }
    break;
  
  case 5: // check SD access
    if (! SD.begin(4))
    {
      ledError(i);
      return 1;
    }
    else 
    {
      return 0;
    }
    break;
  
  default:
    break;
  }
}

void ledError(byte i)
{
  switch(i)
    {
      case 0:
      
        if (((millis() - configModeStartTime)>= 1000))
        {
        if(levier == true)
        {
          led.setColorRGB(0, 255, 0, 0);
          levier = false;
          configModeStartTime = millis();
        }
        else
        {
          led.setColorRGB(0, 0, 0, 255);
          levier = true;
          configModeStartTime = millis();
          }
        }
        break;
        
      case 1:
        if (((millis() - configModeStartTime)>= 1000))
        {
          if(levier == true)
          {
          led.setColorRGB(0, 255, 0, 0);
          levier = false;
          configModeStartTime = millis();
          }
        else
        {
          led.setColorRGB(0, 255, 200, 0);
          levier = true;
          configModeStartTime = millis();
        }
        }
        break;

      case 2:
        if (((millis() - configModeStartTime)>= 1000))
        {
          if(levier == true)
          {
          led.setColorRGB(0, 255, 255, 255);
          levier = false;
          configModeStartTime = millis();
          }
        else if (((millis() - configModeStartTime)>= 2000))
        {
          led.setColorRGB(0, 255, 0, 0);
          levier = true;
          configModeStartTime = millis();
        }
        }
        break;

      case 3:
        if (((millis() - configModeStartTime)>= 1000))
        {
          if(levier == true)
          {
          led.setColorRGB(0, 255, 0, 0);
          levier = false;
          configModeStartTime = millis();
          }
        else
        {
          led.setColorRGB(0, 255, 255, 255);
          levier = true;
          configModeStartTime = millis();
        }
        }
        break;

      case 4:
        if (((millis() - configModeStartTime)>= 1000))
        {
          if(levier == true)
          {
          led.setColorRGB(0, 255, 0, 0);
          levier = false;
          configModeStartTime = millis();
          }
        else
        {
          led.setColorRGB(0, 0, 255, 0);
          levier = true;
          configModeStartTime = millis();
        }
        }
        break;

      case 5:
        if (((millis() - configModeStartTime)>= 1000))
        {
          if(levier == true)
          {
          led.setColorRGB(0, 0, 255, 0);
          levier = false;
          configModeStartTime = millis();
          }
        else if (((millis() - configModeStartTime)>= 2000))
        {
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

// void updateEEPROMValues()
// {
//   EEPROM.write(adresse++, LUMIN);
//   EEPROM.write(adresse, LUMIN_LOW);
//   adresse += sizeof(LUMIN_LOW);
//   EEPROM.write(adresse, LUMIN_HIGH);
//   adresse += sizeof(LUMIN_HIGH);
//   EEPROM.write(adresse++, TEMP_AIR);
//   EEPROM.write(adresse, MIN_TEMP_AIR);
//   adresse += sizeof(MIN_TEMP_AIR);
//   EEPROM.write(adresse, MAX_TEMP_AIR);
//   adresse += sizeof(MAX_TEMP_AIR);
//   EEPROM.write(adresse++, HYGR);
//   EEPROM.write(adresse, HYGR_MINT);
//   adresse += sizeof(HYGR_MINT);
//   EEPROM.write(adresse, HYGR_MAXT);
//   adresse += sizeof(HYGR_MAXT);
//   EEPROM.write(adresse++, PRESSURE);
//   EEPROM.write(adresse, PRESSURE_MIN);
//   adresse += sizeof(PRESSURE_MIN);
//   EEPROM.write(adresse, PRESSURE_MAX);
//   adresse += sizeof(PRESSURE_MAX);
// }

// void readEEPROMValues() {
//   int adresse = 0; // Adresse dans l'EEPROM

//   // Lire les variables depuis l'EEPROM
//   LUMIN = EEPROM.read(adresse++);
//   LUMIN_LOW = EEPROM.read(adresse);
//   adresse += sizeof(LUMIN_LOW);
//   LUMIN_HIGH = EEPROM.read(adresse);
//   adresse += sizeof(LUMIN_HIGH);
//   TEMP_AIR = EEPROM.read(adresse++);
//   MIN_TEMP_AIR = EEPROM.read(adresse);
//   adresse += sizeof(MIN_TEMP_AIR);
//   MAX_TEMP_AIR = EEPROM.read(adresse);
//   adresse += sizeof(MAX_TEMP_AIR);
//   HYGR = EEPROM.read(adresse++);
//   HYGR_MINT = EEPROM.read(adresse);
//   adresse += sizeof(HYGR_MINT);
//   HYGR_MAXT = EEPROM.read(adresse);
//   adresse += sizeof(HYGR_MAXT);
//   PRESSURE = EEPROM.read(adresse++);
//   PRESSURE_MIN = EEPROM.read(adresse);
//   adresse += sizeof(PRESSURE_MIN);
//   PRESSURE_MAX = EEPROM.read(adresse);
//   adresse += sizeof(PRESSURE_MAX);
// }

void funcConfig() 
{
  if (Serial.available() > 0) 
  {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("LOG_INTERVALL=")) 
    {
      LOG_INTERVALL = command.substring(14).toInt();
      Serial.print(F("LOG_INTERVALL mis à jour: "));
      Serial.println(LOG_INTERVALL);
    } else if (command.startsWith("FILE_MAX_SIZE=")) 
    {
      FILE_MAX_SIZE = command.substring(13).toInt();
      Serial.print(F("FILE_MAX_SIZE mis à jour: "));
      Serial.println(FILE_MAX_SIZE);
    } else if (command.startsWith("LUMIN=")) 
    {
      LUMIN = command.substring(6).toInt();
      Serial.print(F("LUMIN mis à jour: "));
      Serial.println(LUMIN);
    } else if (command.startsWith("LUMIN_LOW=")) 
    {
      LUMIN_LOW = command.substring(10).toInt();
      Serial.print(F("LUMIN_LOW mis à jour: "));
      Serial.println(LUMIN_LOW);
    } else if (command.startsWith("LUMIN_HIGH=")) 
    {
      LUMIN_HIGH = command.substring(11).toInt();
      Serial.print(F("LUMIN_HIGH mis à jour: "));
      Serial.println(LUMIN_HIGH);
    } else if (command.equals("RESET")) 
    {
      LOG_INTERVALL = 2000;  
      FILE_MAX_SIZE = 4096;  
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
      Serial.println(F("Paramètres réinitialisés aux valeurs par défaut."));
    } else if (command.equals("VERSION")) 
    {
      Serial.println(F("Version du programme: 1.0"));
    } else if (command.startsWith("TEMP_AIR=")) 
    {
      TEMP_AIR = command.substring(9).toInt();
      Serial.print(F("TEMP_AIR mis à jour: "));
      Serial.println(TEMP_AIR);
    } else if (command.startsWith("MIN_TEMP_AIR=")) 
    {
      MIN_TEMP_AIR = command.substring(13).toInt();
      Serial.print(F("MIN_TEMP_AIR mis à jour: "));
      Serial.println(MIN_TEMP_AIR);
    } else if (command.startsWith("MAX_TEMP_AIR=")) 
    {
      MAX_TEMP_AIR = command.substring(13).toInt();
      Serial.print(F("MAX_TEMP_AIR mis à jour: "));
      Serial.println(MAX_TEMP_AIR);
    } else if (command.startsWith("HYGR=")) 
    {
      HYGR = command.substring(5).toInt();
      Serial.print(F("HYGR mis à jour: "));
      Serial.println(HYGR);
    } else if (command.startsWith("HYGR_MINT=")) 
    {
      HYGR_MINT = command.substring(10).toInt();
      Serial.print(F("HYGR_MINT mis à jour: "));
      Serial.println(HYGR_MINT);
    } else if (command.startsWith("HYGR_MAXT=")) 
    {
      HYGR_MAXT = command.substring(10).toInt();
      Serial.print(F("HYGR_MAXT mis à jour: "));
      Serial.println(HYGR_MAXT);
    } else if (command.startsWith("PRESSURE=")) 
    {
      PRESSURE = command.substring(10).toInt();
      Serial.print(F("PRESSURE mis à jour: "));
      Serial.println(PRESSURE);
    } else if (command.startsWith("PRESSURE_MIN=")) 
    {
      PRESSURE_MIN = command.substring(14).toInt();
      Serial.print(F("PRESSURE_MIN mis à jour: "));
      Serial.println(PRESSURE_MIN);
    } else if (command.startsWith("PRESSURE_MAX=")) 
    {
      PRESSURE_MAX = command.substring(14).toInt();
      Serial.print(F("PRESSURE_MAX mis à jour: "));
      Serial.println(PRESSURE_MAX);
    } else 
    {
      Serial.println(F("Commande non reconnue."));
    }
  }
}
