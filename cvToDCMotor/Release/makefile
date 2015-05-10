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
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
cvToDCMotor.lss \

FLASH_IMAGE += \
cvToDCMotor.hex \

EEPROM_IMAGE += \
cvToDCMotor.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: cvToDCMotor.elf secondary-outputs

# Tool invocations
cvToDCMotor.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C++ Linker'
	avr-g++ -lc -lm -Wl,-Map,cvToDCMotor.map,--cref -mrelax -Wl,--gc-sections -L"/media/sharedHDD/Eclipse/Bastl/arduino-base/Release" -mmcu=atmega328p -o "cvToDCMotor.elf" $(OBJS) $(USER_OBJS) $(LIBS) -lc
	@echo 'Finished building target: $@'
	@echo ' '

cvToDCMotor.lss: cvToDCMotor.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S cvToDCMotor.elf  >"cvToDCMotor.lss"
	@echo 'Finished building: $@'
	@echo ' '

cvToDCMotor.hex: cvToDCMotor.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex cvToDCMotor.elf  "cvToDCMotor.hex"
	@echo 'Finished building: $@'
	@echo ' '

cvToDCMotor.eep: cvToDCMotor.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex cvToDCMotor.elf  "cvToDCMotor.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: cvToDCMotor.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p cvToDCMotor.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(CXX_DEPS)$(S_UPPER_DEPS)$(C++_DEPS)$(ASM_DEPS)$(CC_DEPS)$(INO_DEPS)$(CPP_DEPS)$(C_UPPER_DEPS)$(SIZEDUMMY) cvToDCMotor.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
