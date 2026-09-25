################################################################################
# MRS Version: 2.5.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/code/src/adc.c \
../User/code/src/lead.c \
../User/code/src/pid.c \
../User/code/src/pwm.c 

C_DEPS += \
./User/code/src/adc.d \
./User/code/src/lead.d \
./User/code/src/pid.d \
./User/code/src/pwm.d 

OBJS += \
./User/code/src/adc.o \
./User/code/src/lead.o \
./User/code/src/pid.o \
./User/code/src/pwm.o 

DIR_OBJS += \
./User/code/src/*.o \

DIR_DEPS += \
./User/code/src/*.d \

DIR_EXPANDS += \
./User/code/src/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/code/src/%.o: ../User/code/src/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"f:/MounRiver/MyProject/CH32/BuckBoost/Debug" -I"f:/MounRiver/MyProject/CH32/BuckBoost/Core" -I"f:/MounRiver/MyProject/CH32/BuckBoost/User" -I"f:/MounRiver/MyProject/CH32/BuckBoost/Peripheral/inc" -I"f:/MounRiver/MyProject/CH32/BuckBoost/User/code/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

