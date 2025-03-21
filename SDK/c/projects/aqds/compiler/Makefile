OUTFILE      = compiler.bin
INC_DIRS    += .
SRC_DIRS    += .
CFLAGS      += --max-allocs-per-node 30000

BUILD_DIR   ?= build
AQPLUS_SDK  ?= ../..

OUT         ?= $(BUILD_DIR)/$(OUTFILE)

S_SRCS      += $(wildcard $(addsuffix /*.s, $(SRC_DIRS)))
C_SRCS      += $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))

S_OBJS      := $(addprefix $(BUILD_DIR)/, $(S_SRCS:.s=.rel))
C_OBJS      := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.rel))
OBJS        := $(S_OBJS) $(C_OBJS)

CFLAGS      += -mz80 --opt-code-speed
CFLAGS      += -I$(AQPLUS_SDK)/c/include $(addprefix -I,$(INC_DIRS))
LFLAGS      += -mz80 --data-loc 0 --no-std-crt0 -L $(AQPLUS_SDK)/c/lib aqplus.lib
LFLAGS      += --code-loc 0x109

.PHONY: all clean run_emu run_aqp

all: $(OUT) $(DEPS)

$(OUT): $(OBJS) crt0.asm
	@echo Assembling crt0.asm
	@sdasz80 -o build/crt0.rel crt0.asm
	@echo Linking $@
	@mkdir -p $(dir $@)
	@sdcc $(LFLAGS) build/crt0.rel $(OBJS) -o $(OUT).ihx
	@sdobjcopy -I ihex -O binary $(OUT).ihx $(OUT)

$(S_OBJS): $(BUILD_DIR)/%.rel: %.s
	@echo Assembling $<
	@mkdir -p $(dir $@)
	@sdasz80 -o $@ $<

$(C_OBJS): $(BUILD_DIR)/%.rel: %.c
	@echo Compiling $<
	@mkdir -p $(dir $@)
	@sdcc $(CFLAGS) -o $@ -c $<

clean:
	@echo Removing build directory
	@rm -rf $(BUILD_DIR)

pc:
	gcc -Istubs -Wall -Wextra -g -O0 -o compiler main.c tokenizer.c common.c parser.c expr.c symbols.c stubs/file_io.c

pc_test: pc
	./compiler testdata/tetris.cb
	../assembler/assembler testdata/out/tetris.asm

pc_emu: pc_test
	cp testdata/out/out/tetris.aqx ../../../../../EndUser/sdcard/
	../../../../../System/emulator/build/AquariusPlusEmu.app/Contents/MacOS/aquarius_emu -t "\n   RUN\"tetris.aqx\"\n"

# pc_test: pc
# 	./compiler testdata/test.cb
# 	../assembler/assembler testdata/out/test.asm

# pc_emu: pc_test
# 	cp testdata/out/out/test.aqx ../../../../../EndUser/sdcard/
# 	../../../../../System/emulator/build/AquariusPlusEmu.app/Contents/MacOS/aquarius_emu -t "\n   RUN\"test.aqx\"\n"
