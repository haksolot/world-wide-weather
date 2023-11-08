BOARD_TAG = arduino:avr:uno

ARDUINO_PORT = COM3

ARDUINO_CLI = arduino-cli.exe

SRC_DIR = src
BUILD_DIR = build

all: build upload serial

libraries:
	@libraries.bat
build:
	@echo "Compilation en cours..."
	@$(ARDUINO_CLI) compile -b $(BOARD_TAG) --output-dir $(BUILD_DIR) $(SRC_DIR)

upload:
	@echo "Televersement en cours..."
	@$(ARDUINO_CLI) upload -b $(BOARD_TAG) --port $(ARDUINO_PORT) --input-dir $(BUILD_DIR)

serial:
	@$(ARDUINO_CLI) monitor -p $(ARDUINO_PORT)

clean:
	@echo "Nettoyage en cours..."
	@rm -r $(BUILD_DIR)

.PHONY: all build upload install_libraries clean
