@echo off
echo Vérification et installation des bibliotheques...

set LIBRARIES=SoftwareSerial ChainableLED RTClib Adafruit_BusIO Wire Adafruit_BME280_Library SPI Adafruit_Unified_Sensor TinyGPSPlus SD EEPROM

for %%i in (%LIBRARIES%) do (
  echo Verification de la bibliotheque %%i...
  arduino-cli.exe lib list | findstr /C:"%%i" > nul
  if errorlevel 1 (
    echo Installation de la bibliotheque %%i...
    arduino-cli.exe lib install "%%i"
  ) else (
    echo La bibliotheque %%i est deja installee.
  )
)

echo Verification et installation des bibliotheques terminee.
