BUILD_DIR   ?= build

OUTFILE      = aqplus.lib
OUT          = ../$(OUTFILE)

INC_DIRS    += ../../include
SRC_DIRS    += . file_io video

S_SRCS      += $(wildcard $(addsuffix /*.s, $(SRC_DIRS)))
C_SRCS      += $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))

S_OBJS      := $(addprefix $(BUILD_DIR)/, $(S_SRCS:.s=.rel))
C_OBJS      := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.rel))
OBJS        := $(S_OBJS) $(C_OBJS)

CFLAGS      += -mz80 --opt-code-speed
CFLAGS      += $(addprefix -I,$(INC_DIRS))

.PHONY: all clean run

all: $(OUT) ../crt0.rel

$(OUT): $(OBJS)
	@echo Creating library archive $@
	sdar -rc $(OUT) $(OBJS)

../crt0.rel: crt0/crt0.s
	@echo Assembling $<
	@sdasz80 -o $@ $<

$(S_OBJS): $(BUILD_DIR)/%.rel: %.s
	@echo Assembling $<
	@mkdir -p $(dir $@)
	@sdasz80 -o $@ $<

$(C_OBJS): $(BUILD_DIR)/%.rel: %.c
	@echo Compiling $<
	@mkdir -p $(dir $@)
	@sdcc $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(BUILD_DIR)
