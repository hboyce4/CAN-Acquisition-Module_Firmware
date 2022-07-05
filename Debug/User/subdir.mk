################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/UI.c \
../User/descriptors.c \
../User/interrupt.c \
../User/main.c \
../User/sys.c \
../User/vcom_serial.c 

OBJS += \
./User/UI.o \
./User/descriptors.o \
./User/interrupt.o \
./User/main.o \
./User/sys.o \
./User/vcom_serial.o 

C_DEPS += \
./User/UI.d \
./User/descriptors.d \
./User/interrupt.d \
./User/main.d \
./User/sys.d \
./User/vcom_serial.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wextra  -g -I"../Library/CMSIS/Include" -I"../Library/Device/Nuvoton/M480/Include" -I"../Library/StdDriver/inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


