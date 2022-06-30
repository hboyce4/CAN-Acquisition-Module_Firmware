################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Library/Device/Nuvoton/M480/Source/system_M480.c 

OBJS += \
./Library/Device/Nuvoton/M480/Source/system_M480.o 

C_DEPS += \
./Library/Device/Nuvoton/M480/Source/system_M480.d 


# Each subdirectory must supply rules for building sources it contributes
Library/Device/Nuvoton/M480/Source/%.o: ../Library/Device/Nuvoton/M480/Source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -Wextra  -g -I"../Library/CMSIS/Include" -I"../Library/Device/Nuvoton/M480/Include" -I"../Library/StdDriver/inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


