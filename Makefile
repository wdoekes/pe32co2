PROJECT := $(notdir $(CURDIR))
#FQBN := esp32.esp32.esp32wrover
FQBN := esp8266.esp8266.generic
BOARD := $(subst .,:,$(FQBN))
BINARY := build/$(FQBN)/$(PROJECT).ino.bin

ifeq ($(FQBN),esp32.esp32.esp32wrover)
BOARD_IS_MCH2022_BADGE := 1
endif

ifneq ($(BOARD_IS_MCH2022_BADGE),)
CPPFLAGS += -DBOARD_IS_MCH2022_BADGE=1
endif

ifneq ($(BOARD_IS_MCH2022_BADGE),)
.PHONY: upload
upload: build
	~/Arduino/projmch2022/mch2022-tools/app_push.py --run $(BINARY) 'my_cool_app' main.bin 1
endif

.PHONY: build
build: $(BINARY)

.PHONY: debug-build
debug-build:
	touch $(PROJECT).ino
	# DEBUG_SERIAL=Serial
	# DEBUG_SENSORS=1
	arduino-cli compile --warnings=all -eb $(BOARD) --build-property 'compiler.cpp.extra_flags=$(CPPFLAGS) -DDEBUG_SENSORS=1' $(CURDIR)

$(BINARY): $(PROJECT).ino *.h *.cpp
	arduino-cli compile --warnings=all -eb $(BOARD) --build-property 'compiler.cpp.extra_flags=$(CPPFLAGS)' $(CURDIR)
