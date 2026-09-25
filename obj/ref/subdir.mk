################################################################################
# MRS Version: 2.5.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ref/main.c 

C_DEPS += \
./ref/main.d 

OBJS += \
./ref/main.o 

DIR_OBJS += \
./ref/*.o \

DIR_DEPS += \
./ref/*.d \

DIR_EXPANDS += \
./ref/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
ref/%.o: ../ref/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"f:/MounRiver/MyProject/CH32/BuckBoost/Debug" -I"f:/MounRiver/MyProject/CH32/BuckBoost/Core" -I"f:/MounRiver/MyProject/CH32/BuckBoost/User" -I"f:/MounRiver/MyProject/CH32/BuckBoost/Peripheral/inc" -I"f:/MounRiver/MyProject/CH32/BuckBoost/User/code/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

