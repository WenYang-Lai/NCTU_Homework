################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../src/funcion.s 

C_SRCS += \
../src/L10-1.c \
../src/L10-2.c \
../src/L10-3.c 

OBJS += \
./src/L10-1.o \
./src/L10-2.o \
./src/L10-3.o \
./src/funcion.o 

C_DEPS += \
./src/L10-1.d \
./src/L10-2.d \
./src/L10-3.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo %cd%
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32L4 -DSTM32L476RGTx -DNUCLEO_L476RG -DSTM32 -DDEBUG -I"C:/Users/user/workspace/lab10/inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/funcion.o: C:/Users/user/workspace/lab10/src/funcion.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	@echo %cd%
	arm-none-eabi-as -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -I"C:/Users/user/workspace/lab10/inc" -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


