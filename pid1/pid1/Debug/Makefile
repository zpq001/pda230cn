################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
../inc \
../src


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../src/adc.c \
../src/buttons.c \
../src/control.c \
../src/fir_filter.c \
../src/led_indic.c \
../src/led_indic_hw.c \
../src/menu.c \
../src/my_string.c \
../src/pid1.c \
../src/pid_controller.c \
../src/power_control.c \
../src/progmem_func.c \
../src/soft_timer.c \
../src/systimer.c \
../src/usart.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
src/adc.o \
src/buttons.o \
src/control.o \
src/fir_filter.o \
src/led_indic.o \
src/led_indic_hw.o \
src/menu.o \
src/my_string.o \
src/pid1.o \
src/pid_controller.o \
src/power_control.o \
src/progmem_func.o \
src/soft_timer.o \
src/systimer.o \
src/usart.o


OBJS_AS_ARGS +=  \
src/adc.o \
src/buttons.o \
src/control.o \
src/fir_filter.o \
src/led_indic.o \
src/led_indic_hw.o \
src/menu.o \
src/my_string.o \
src/pid1.o \
src/pid_controller.o \
src/power_control.o \
src/progmem_func.o \
src/soft_timer.o \
src/systimer.o \
src/usart.o


C_DEPS +=  \
src/adc.d \
src/buttons.d \
src/control.d \
src/fir_filter.d \
src/led_indic.d \
src/led_indic_hw.d \
src/menu.d \
src/my_string.d \
src/pid1.d \
src/pid_controller.d \
src/power_control.d \
src/progmem_func.d \
src/soft_timer.d \
src/systimer.d \
src/usart.d


C_DEPS_AS_ARGS +=  \
src/adc.d \
src/buttons.d \
src/control.d \
src/fir_filter.d \
src/led_indic.d \
src/led_indic_hw.d \
src/menu.d \
src/my_string.d \
src/pid1.d \
src/pid_controller.d \
src/power_control.d \
src/progmem_func.d \
src/soft_timer.d \
src/systimer.d \
src/usart.d


OUTPUT_FILE_PATH +=pid1.elf

OUTPUT_FILE_PATH_AS_ARGS +=pid1.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

# AVR32/GNU C Compiler































src/%.o: ../src/%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 
	$(QUOTE)C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.2.876\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -funsigned-char -funsigned-bitfields -DDEBUG  -I"../inc" -I"../src"  -Os -ffunction-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega8 -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o"$@" "$<" 
	@echo Finished building: $<



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : 
	$(QUOTE)C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.2.876\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="pid1.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=atmega8 
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.2.876\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  "pid1.elf" "pid1.hex"
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.2.876\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "pid1.elf" "pid1.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.2.876\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "pid1.elf" > "pid1.lss"
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.2.876\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature  "pid1.elf" "pid1.srec"
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.2.876\avr8-gnu-toolchain\bin\avr-size.exe" "pid1.elf"
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS)$(C_DEPS_AS_ARGS) $(EXECUTABLES) 
	rm -rf "pid1.elf" "pid1.a" "pid1.hex" "pid1.lss" "pid1.eep" "pid1.map" "pid1.srec"
	