# Makefile

# Step 1: Include the sources
include sources.mk

# Step 2: create name of the final target
TARGET := main

# Step 3: Setup flags based on platform
CC := arm-none-eabi-gcc
OBJDUMP := arm-none-eabi-objdump
SIZE := arm-none-eabi-size
GDB := arm-none-eabi-gdb
OPENOCD := openocd
LINKER_FLASH := -T $(LINKER_DIR)/STM32F446RETX_FLASH.ld
CPU := -mcpu=cortex-m4
ARCH_FLAGS := -mthumb \
							-mfloat-abi=hard \
							-mfpu=fpv4-sp-d16 \
							-ffunction-sections \
							-fdata-sections \
							-fstack-usage \
							--specs=nano.specs
CPPFLAGS :=
LDFLAGS := $(LINKER_FLASH) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

# Add warning flags
WFLAGS = -Wall -Wpedantic -Wextra -Wshadow -Werror

# Note: Compile with maximum debug info and zero optimisation
CFLAGS := $(WFLAGS) -std=gnu11 -g3 -O0 -DSTM32F446xx

# Step 4: Create names for the files
# Note: Substitute the object files with the same name as the sources with a different extension (input, replacement, the actual text)
OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))
STARTUP_OBJECTS := $(patsubst %.s,$(BUILD_DIR)/%.o,$(STARTUP_SOURCE))
ALL_OBJS := $(OBJECTS) $(STARTUP_OBJECTS)

# Dependencies
DEPS := $(patsubst %.c,$(BUILD_DIR)/%.d,$(SOURCES))
-include $(DEPS)

# Step 6: Create recipes
# Note: 
# - Create directory by extracting the directory portion of the path
# - This recipe is to create assembly for a single source file
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CPU) $(ARCH_FLAGS) $(INCLUDES) -MMD -MP -MF $(BUILD_DIR)/$*.d -c $< -o $@

# Build the board startup assembly file into an object
# Separate rule as it's a '.s' file
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CPU) $(ARCH_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).out: $(ALL_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CPU) $(ARCH_FLAGS) $(INCLUDES) $(LDFLAGS) $^ -o $@
	$(SIZE) $@

.PHONY: build
build: $(BUILD_DIR)/$(TARGET).out

.PHONY: flash
flash: $(BUILD_DIR)/$(TARGET).out
	$(OPENOCD) -f stm32_health_monitor.cfg -c "program $(BUILD_DIR)/$(TARGET).out verify reset exit" -l $(LOG_DIR)/openocd.log 

.PHONY: debug-server
debug-server: flash
	$(OPENOCD) -f stm32_health_monitor.cfg
	@echo "Open a new terminal and type 'make debug'"

.PHONY: debug
debug: $(BUILD_DIR)/$(TARGET).out
	$(GDB) $(BUILD_DIR)/$(TARGET).out -x $(GDB_INIT)

# Note: This is to create assembly for the final linked binary by disassembling the it and showing the original C code alongside the assembly
.PHONY: $(TARGET).asm
$(TARGET).asm: $(BUILD_DIR)/$(TARGET).out
	$(OBJDUMP) -S $< > $(BUILD_DIR)/$@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: show-temp
show-temp:
	picocom --baud 115200 --databits 8 --parity n --stopbits 1 --flow n /dev/ttyACM0
