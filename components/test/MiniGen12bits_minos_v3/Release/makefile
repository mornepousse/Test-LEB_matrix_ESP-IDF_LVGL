################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: MiniGen12bits_minos_v3.axf

# Tool invocations
MiniGen12bits_minos_v3.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"E:\LEMIA\PROJECTS\BRUITEUR_SDP\MCU\CMSISv2p00_LPC11xx\Debug" -Xlinker -Map="MiniGen12bits_minos_v3.map" -Xlinker --gc-sections -mcpu=cortex-m0 -mthumb -T "MiniGen_Release.ld" -o "MiniGen12bits_minos_v3.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) MiniGen12bits_minos_v3.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size MiniGen12bits_minos_v3.axf; arm-none-eabi-objcopy -O ihex MiniGen12bits_minos_v3.axf MiniGen12bits_minos_v3.hex ;
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
