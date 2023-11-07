BOARD_TAG = arduino:avr:uno

ARDUINO_PORT = /dev/ttyACM0

ARDUINO_CLI = arduino-cli

SRC_DIR = src

BUILD_DIR = build

BUILD_CMD = $(ARDUINO_CLI) compile -b $(BOARD_TAG) --output-dir $(BUILD_DIR) $(SRC_DIR)
UPLOAD_CMD = $(ARDUINO_CLI) upload -b $(BOARD_TAG) --port $(ARDUINO_PORT) --input-dir $(BUILD_DIR)

all: build upload

build:
	@echo "Compilation en cours..."
	@$(BUILD_CMD)

upload:
	@echo "Téléversement en cours..."
	@$(UPLOAD_CMD)

clean:
	@echo "Nettoyage en cours..."
	@rm -rf $(BUILD_DIR)

.PHONY: all build upload clean
