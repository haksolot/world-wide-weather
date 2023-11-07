ARDUINO_DIR = /usr/share/arduino

BOARD_TAG = uno

ARDUINO_PORT = /dev/ttyACM0

# ARDUINO_LIBS = Wire SoftwareSerial Adafruit_Sensor Adafruit_BME280 TinyGPSPlus

USER_LIB_PATH = libs

BUILD_DIR = build

SRC_DIR = src

AVR_TOOLS_DIR = $(ARDUINO_DIR)/hardware/tools/avr

ARDUINO_CORE_PATH = $(ARDUINO_DIR)/hardware/arduino/avr

USER_LIB_PATH = $(realpath $(HOME)/Arduino/libraries)

include $(ARDUINO_CORE_PATH)/Arduino.mk
