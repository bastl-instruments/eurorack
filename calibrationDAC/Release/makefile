################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
calibrationDAC.lss \

FLASH_IMAGE += \
calibrationDAC.hex \

EEPROM_IMAGE += \
calibrationDAC.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: calibrationDAC.elf secondary-outputs

# Tool invocations
calibrationDAC.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C++ Linker'
	avr-g++ -lc -lm -Wl,-Map,calibrationDAC.map,--cref -mrelax -Wl,--gc-sections -L"/Users/dasvaclav/Documents/workspace/arduino-base/Release" -mmcu=atmega328p -o "calibrationDAC.elf" $(OBJS) $(USER_OBJS) $(LIBS) -lc
	@echo 'Finished building target: $@'
	@echo ' '

calibrationDAC.lss: calibrationDAC.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S calibrationDAC.elf  >"calibrationDAC.lss"
	@echo 'Finished building: $@'
	@echo ' '

calibrationDAC.hex: calibrationDAC.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex calibrationDAC.elf  "calibrationDAC.hex"
	@echo 'Finished building: $@'
	@echo ' '

calibrationDAC.eep: calibrationDAC.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex calibrationDAC.elf  "calibrationDAC.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: calibrationDAC.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p calibrationDAC.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C_UPPER_DEPS)$(S_DEPS)$(LSS)$(C_DEPS)$(FLASH_IMAGE)$(CC_DEPS)$(ELFS)$(C++_DEPS)$(OBJS)$(CXX_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(INO_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(CPP_DEPS) calibrationDAC.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
