BUILD_DIR   ?= build

AQHOST       = 192.168.178.49
AQHOST      ?= aqplus.local

OUTFILE      = pt3player.caq
OUT          = $(BUILD_DIR)/$(OUTFILE)

INC_DIRS    += . lib
SRC_DIRS    += . lib

S_SRCS      += lib/crt0.s
C_SRCS      += main.c ingarden.c

S_SRCS      += lib/getchar.s lib/putchar.s
S_SRCS      += lib/pt3play.s
# C_SRCS      += lib/file_io.c


S_OBJS      := $(addprefix $(BUILD_DIR)/, $(S_SRCS:.s=.rel))
C_OBJS      := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.rel))
OBJS        := $(S_OBJS) $(C_OBJS)

CFLAGS      += -mz80 --opt-code-speed
CFLAGS      += $(addprefix -I,$(INC_DIRS))
LFLAGS      += -mz80 --code-loc 0x391B --data-loc 0 --no-std-crt0


.PHONY: all clean run

all: $(OUT)

$(OUT): $(BUILD_DIR) $(OBJS)
	@echo Linking $@
	sdcc $(LFLAGS) $(OBJS) -o $(OUT).ihx
	makebin -p -o 0x38E1 $(OUT).ihx $(OUT)

$(S_OBJS): $(BUILD_DIR)/%.rel: %.s
	@echo Assembling $<
	sdasz80 -o $@ $<

$(C_OBJS): $(BUILD_DIR)/%.rel: %.c
	@echo Compiling $<
	sdcc $(CFLAGS) -o $@ -c $<

$(BUILD_DIR):
	@mkdir -p $(dir $(OBJS))


clean:
	rm -rf $(BUILD_DIR)

run: all
	cp $(OUT) ~/Documents/AquariusPlusDisk/
	../../../System/emulator/build/AquariusPlusEmu.app/Contents/MacOS/aquarius_emu -t "\nrun\"$(OUTFILE)\"\n"

run2: all
	curl -T $(OUT) http://$(AQHOST)/
	printf '\x1E\nRUN"$(OUTFILE)"\n' | netcat -c $(AQHOST) 23
