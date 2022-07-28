################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/CAN_message.c \
../User/I2C_sensors.c \
../User/SPI_LCD.c \
../User/UI.c \
../User/analog.c \
../User/analog_channels_default.c \
../User/descriptors.c \
../User/errors.c \
../User/fonts.c \
../User/initialization.c \
../User/interrupt.c \
../User/main.c \
../User/persistent_data.c \
../User/user_sys.c \
../User/vcom_serial.c 

OBJS += \
./User/CAN_message.o \
./User/I2C_sensors.o \
./User/SPI_LCD.o \
./User/UI.o \
./User/analog.o \
./User/analog_channels_default.o \
./User/descriptors.o \
./User/errors.o \
./User/fonts.o \
./User/initialization.o \
./User/interrupt.o \
./User/main.o \
./User/persistent_data.o \
./User/user_sys.o \
./User/vcom_serial.o 

C_DEPS += \
./User/CAN_message.d \
./User/I2C_sensors.d \
./User/SPI_LCD.d \
./User/UI.d \
./User/analog.d \
./User/analog_channels_default.d \
./User/descriptors.d \
./User/errors.d \
./User/fonts.d \
./User/initialization.d \
./User/interrupt.d \
./User/main.d \
./User/persistent_data.d \
./User/user_sys.d \
./User/vcom_serial.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c User/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wextra  -g -I"../Library/CMSIS/Include" -I"../Library/Device/Nuvoton/M480/Include" -I"../Library/StdDriver/inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


