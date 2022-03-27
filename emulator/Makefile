#-----------------------------------------------------------------------------
# Makefile
#-----------------------------------------------------------------------------
INC_DIRS    += . libz80
SRC_DIRS    += .
C_SRCS      += libz80/z80.c

CFLAGS      += -O3 -g
OUT          = $(BUILD_DIR)/aquarius_emu
CFLAGS      += `pkg-config --cflags sdl2`
LFLAGS      += `pkg-config --libs sdl2`

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
