################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../midiSeq.ino 

CPP_SRCS += \
../MCPDAC.cpp \
../MIDInoteBuffer.cpp \
../midiSeqHW.cpp 

OBJS += \
./MCPDAC.o \
./MIDInoteBuffer.o \
./midiSeq.o \
./midiSeqHW.o 

INO_DEPS += \
./midiSeq.d 

CPP_DEPS += \
./MCPDAC.d \
./MIDInoteBuffer.d \
./midiSeqHW.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -MMD -MT "$@" -I"/Users/dasvaclav/Documents/workspace/eurorack/midiSeq" -I"/Users/dasvaclav/Documents/workspace/arduino-base" -I"/Users/dasvaclav/Documents/workspace/arduino-base/core" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Wire" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Wire/utility" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/WiFi" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/WiFi/utility" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Stepper" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/SPI" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/SoftwareSerial" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Servo" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/LiquidCrystal" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Ethernet" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Ethernet/utility" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/EEPROM" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs-external/arduino_midi_library/src" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs-external/Mozzi" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs-external/Mozzi/config" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs-external/Mozzi/tables" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs-bastl/bastl" -DARDUINO=100 -DEXTERNAL_IDE=1 -Wall -Os -ffunction-sections -fdata-sections -fno-exceptions -x c++ -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -MMD -MT "$@" -I"/Users/dasvaclav/Documents/workspace/eurorack/midiSeq" -I"/Users/dasvaclav/Documents/workspace/arduino-base" -I"/Users/dasvaclav/Documents/workspace/arduino-base/core" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Wire" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Wire/utility" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/WiFi" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/WiFi/utility" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Stepper" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/SPI" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/SoftwareSerial" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Servo" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/LiquidCrystal" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Ethernet" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/Ethernet/utility" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs/EEPROM" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs-external/arduino_midi_library/src" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs-external/Mozzi" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs-external/Mozzi/config" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs-external/Mozzi/tables" -I"/Users/dasvaclav/Documents/workspace/arduino-base/libs-bastl/bastl" -DARDUINO=100 -DEXTERNAL_IDE=1 -Wall -Os -ffunction-sections -fdata-sections -fno-exceptions -x c++ -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


