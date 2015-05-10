################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../cvToDCMotor.ino 

OBJS += \
./cvToDCMotor.o 

INO_DEPS += \
./cvToDCMotor.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -MMD -MT "$@" -I"/media/sharedHDD/Eclipse/Bastl/eurorack/cvToDCMotor" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/core" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/Wire" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/Wire/utility" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/WiFi" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/WiFi/utility" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/Stepper" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/SPI" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/SoftwareSerial" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/Servo" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/LiquidCrystal" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/Ethernet" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/Ethernet/utility" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs/EEPROM" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs-external/arduino_midi_library/src" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs-external/Tlc5940" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs-external/Mozzi" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs-external/Mozzi/config" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs-external/Mozzi/tables" -I"/media/sharedHDD/Eclipse/Bastl/arduino-base/libs-bastl/bastl" -DARDUINO=100 -DEXTERNAL_IDE=1 -Wall -Os -ffunction-sections -fdata-sections -fno-exceptions -x c++ -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


