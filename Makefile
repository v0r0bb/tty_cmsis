PROJECT = tty
BUILD_DIR = build
SRC_DIR = src
CMSIS_DIR = cmsis/src

# Toolchain 
CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as 
SZ = $(CROSS_COMPILE)size 
OBJCOPY = $(CROSS_COMPILE)objcopy 
OBJDUMP = $(CROSS_COMPILE)objdump 

# Project includes 
INCLUDES = -I./include
INCLUDES += -I./cmsis/include 

# Project sources 
SRC_FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(CMSIS_DIR)/*.c)
ASM_FILES = startup_stm32l152xe.s 
LD_SCRIPT = STM32L152RETX_FLASH.ld

# Obj files 
SRC_OBJS = $(addprefix $(BUILD_DIR)/$(SRC_DIR)/, $(patsubst %.c, %.o, $(notdir $(SRC_FILES))))
ASM_OBJS = $(addprefix $(BUILD_DIR)/$(SRC_DIR)/, $(ASM_FILES:.s=.o))
OBJS = $(SRC_OBJS) $(ASM_OBJS) 

# Compiler flags 
CFLAGS = -g -O0 -Wall -Wextra
CFLAGS += -DSTM32L152xE
CFLAGS += -mcpu=cortex-m3
CFLAGS += -mthumb
CFLAGS += -mfloat-abi=soft
CFLAGS += $(INCLUDES)

ASFLAGS = -mcpu=cortex-m3
ASFLAGS += -mthumb
ASFLAGS += -mfloat-abi=soft 
ASFLAGS += $(INCLUDES)

# Linker flas 
LFLAGS = -T$(LD_SCRIPT)

TARGET = $(BUILD_DIR)/$(PROJECT).elf

.PHONY: all clean flash 

all: $(TARGET) 

$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)/$(SRC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ 

$(BUILD_DIR)/$(SRC_DIR)/%.o: $(CMSIS_DIR)/%.c | $(BUILD_DIR)/$(SRC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ 

$(BUILD_DIR)/$(SRC_DIR)/%.o: %.s | $(BUILD_DIR)/$(SRC_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@ 

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) $(LFLAGS) $^ -o $@ 
	$(SZ) $@

flash: 
	$(OBJCOPY) -O binary $(TARGET) $(BUILD_DIR)/$(PROJECT).bin  
	st-flash write $(BUILD_DIR)/$(PROJECT).bin 0x08000000

$(BUILD_DIR)/$(SRC_DIR):
	mkdir -p $@

clean: 
	rm -rf $(BUILD_DIR)

