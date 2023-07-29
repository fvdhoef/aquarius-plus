#-----------------------------------------------------------------------------
# Makefile
#-----------------------------------------------------------------------------
ifeq '$(findstring ;,$(PATH))' ';'
    detected_OS := Windows
else
    detected_OS := $(shell uname 2>/dev/null || echo Unknown)
    detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
    detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
    detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif

VERSION_STR  := $(shell date -u "+%Y-%m-%d")_$(shell git describe --always)

INC_DIRS    += . libz80
SRC_DIRS    += .
C_SRCS      += libz80/z80.c

FLAGS       += -O3 -g
OUT          = $(BUILD_DIR)/aquarius_emu

FLAGS       += -DEMULATOR

ifeq ($(detected_OS),Darwin)
FLAGS       += -Fmacos/AquariusPlusEmu.app/Contents/Frameworks -Imacos/AquariusPlusEmu.app/Contents/Frameworks/SDL2.framework/Headers
LFLAGS      += -Fmacos/AquariusPlusEmu.app/Contents/Frameworks -framework SDL2
LFLAGS      += -Wl,-rpath,@executable_path/../Frameworks/

FLAGS       += -arch arm64 -arch x86_64
LFLAGS      += -arch arm64 -arch x86_64
else
FLAGS       += `pkg-config --cflags sdl2`
LFLAGS      += `pkg-config --libs sdl2`
endif

FLAGS       += -D_BSD_SOURCE -D_DEFAULT_SOURCE -pedantic
FLAGS       += -Wall -Wextra -Wshadow -Winit-self -Wfloat-conversion -Wdouble-promotion -Wmissing-include-dirs
CFLAGS      += -Werror=implicit-function-declaration
BUILD_DIR   ?= build

FLAGS       += -MD $(addprefix -I,$(INC_DIRS))

CFLAGS      += -std=c11 $(FLAGS)
CXXFLAGS    += -std=c++17 $(FLAGS)

#-----------------------------------------------------------------------------
# Sources
#-----------------------------------------------------------------------------
C_SRCS      += $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))
CXX_SRCS    += $(wildcard $(addsuffix /*.cpp, $(SRC_DIRS)))

#-----------------------------------------------------------------------------
# Object files
#-----------------------------------------------------------------------------
C_OBJS      := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o))
CXX_OBJS    := $(addprefix $(BUILD_DIR)/, $(CXX_SRCS:.cpp=.o))
OBJS        := $(C_OBJS) $(CXX_OBJS)
DEPS        := $(OBJS:.o=.d)

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------
.PHONY: all clean run

all: $(OUT)

$(OUT): $(BUILD_DIR) $(OBJS)
	@echo Linking $@
	$(CXX) $(CXXFLAGS) $(OBJS) $(LFLAGS) -o $@
	@ln -sf ../aquarius.rom build/aquarius.rom

ifeq ($(detected_OS),Darwin)
	@echo Building app bundle
	@rm -rf $(BUILD_DIR)/AquariusPlusEmu.app
	@cp -r macos/AquariusPlusEmu.app build/
	@mkdir -p build/AquariusPlusEmu.app/Contents/MacOS/
	@cp $(OUT) $(BUILD_DIR)/AquariusPlusEmu.app/Contents/MacOS/aquarius_emu
	@cp aquarius.rom $(BUILD_DIR)/AquariusPlusEmu.app/Contents/Resources/
	@sed -i '' 's/#VERSION#/$(VERSION_STR)/' $(BUILD_DIR)/AquariusPlusEmu.app/Contents/Info.plist
	@cd $(BUILD_DIR); zip -rq AquariusPlusEmu_macOS_$(VERSION_STR).zip AquariusPlusEmu.app
endif

ifeq ($(detected_OS),Linux)
	@echo Building distribution
	@rm -rf $(BUILD_DIR)/AquariusPlusEmu
	@mkdir -p $(BUILD_DIR)/AquariusPlusEmu
	@cp aquarius.rom $(BUILD_DIR)/AquariusPlusEmu/
	@cp $(OUT) $(BUILD_DIR)/AquariusPlusEmu/
	@cp -r ../../EndUser/sdcard $(BUILD_DIR)/AquariusPlusEmu/
endif

$(C_OBJS): $(BUILD_DIR)/%.o: %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -std=c11 -o $@ -c $<

$(CXX_OBJS): $(BUILD_DIR)/%.o: %.cpp
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) -std=c++17  -o $@ -c $<

$(BUILD_DIR):
	@mkdir -p $(dir $(C_OBJS))

run: all
ifeq ($(detected_OS),Darwin)
	$(BUILD_DIR)/AquariusPlusEmu.app/Contents/MacOS/aquarius_emu -u ../../EndUser/sdcard
else
	$(OUT) -u ../testdata/usb
endif

clean:
	@echo Cleaning...
	@rm -f $(OUT)
	@rm -rf $(BUILD_DIR)

.DEFAULT_GOAL = all

-include $(DEPS)
