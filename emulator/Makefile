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

INC_DIRS    += . libz80
SRC_DIRS    += .
C_SRCS      += libz80/z80.c

CFLAGS      += -O3 -g
OUT          = $(BUILD_DIR)/aquarius_emu

ifeq ($(detected_OS),Darwin)
CFLAGS      += -Fmacos/AquariusEmu.app/Contents/Frameworks -Imacos/AquariusEmu.app/Contents/Frameworks/SDL2.framework/Headers
LFLAGS      += -Fmacos/AquariusEmu.app/Contents/Frameworks -framework SDL2
LFLAGS      += -Wl,-rpath,@executable_path/../Frameworks/

CFLAGS      += -arch arm64 -arch x86_64
LFLAGS      += -arch arm64 -arch x86_64
else
CFLAGS      += `pkg-config --cflags sdl2`
LFLAGS      += `pkg-config --libs sdl2`
endif

CFLAGS      += -D_BSD_SOURCE -D_DEFAULT_SOURCE -std=c11 -pedantic
CFLAGS      += -Wall -Wextra -Wshadow -Winit-self -Wfloat-conversion -Wdouble-promotion -Wmissing-include-dirs
CFLAGS      += -Werror=implicit-function-declaration
BUILD_DIR   ?= build

CFLAGS      += -MD $(addprefix -I,$(INC_DIRS))

#-----------------------------------------------------------------------------
# Sources
#-----------------------------------------------------------------------------
C_SRCS      += $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))

#-----------------------------------------------------------------------------
# Object files
#-----------------------------------------------------------------------------
C_OBJS      := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o))
OBJS        := $(C_OBJS)
DEPS        := $(OBJS:.o=.d)

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------
.PHONY: all clean run

all: $(OUT)
ifeq ($(detected_OS),Darwin)
	@echo Building app bundle
	@rm -rf $(BUILD_DIR)/AquariusEmu.app
	@cp -r macos/AquariusEmu.app build/
	@cp $(OUT) $(BUILD_DIR)/AquariusEmu.app/Contents/MacOS/aquarius_emu
	@cp aquarius.rom $(BUILD_DIR)/AquariusEmu.app/Contents/Resources/
endif

$(OUT): $(BUILD_DIR) $(OBJS)
	@echo Linking $@
	@$(CC) $(OBJS) $(CFLAGS) $(LFLAGS) -o $@
	@ln -sf ../aquarius.rom build/aquarius.rom

$(C_OBJS): $(BUILD_DIR)/%.o: %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR):
	@mkdir -p $(dir $(C_OBJS))

run: $(OUT)
	$(OUT) -u ../testdata/usb

clean:
	@echo Cleaning...
	@rm -f $(OUT)
	@rm -rf $(BUILD_DIR)

.DEFAULT_GOAL = all

-include $(DEPS)
