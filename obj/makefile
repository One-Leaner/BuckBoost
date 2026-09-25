################################################################################
# MRS Version: 2.5.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/code/src/subdir.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
BuckBoost.hex \

SECONDARY_LIST += \
BuckBoost.lst \

SECONDARY_SIZE += \
BuckBoost.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: BuckBoost.elf secondary-outputs

# Tool invocations
BuckBoost.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -T "f:\MounRiver\MyProject\CH32\BuckBoost\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"BuckBoost.map" --specs=nano.specs -u _printf_float -u _scanf_float --specs=nosys.specs -o "BuckBoost.elf" $(OBJS) $(LIBS)
BuckBoost.hex: BuckBoost.elf
	@	riscv-none-embed-objcopy -O ihex "BuckBoost.elf" "BuckBoost.hex"
BuckBoost.lst: BuckBoost.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "BuckBoost.elf" > "BuckBoost.lst"
BuckBoost.siz: BuckBoost.elf
	riscv-none-embed-size --format=berkeley "BuckBoost.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) BuckBoost.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets