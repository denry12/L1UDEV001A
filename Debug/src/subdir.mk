################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ILI9341.c \
../src/L1UDEV001A.c \
../src/cr_startup_lpc11uxx.c \
../src/crp.c \
../src/gpio.c \
../src/l11uxx_adc_library.c \
../src/l11uxx_internal_flash_lib.c \
../src/l11uxx_spi_library.c \
../src/l11uxx_timer_lib.c \
../src/l11uxx_uart_library.c \
../src/lcd_5110_lib.c \
../src/nrf24l01_lib.c 

S_SRCS += \
../src/aeabi_romdiv_patch.s 

OBJS += \
./src/ILI9341.o \
./src/L1UDEV001A.o \
./src/aeabi_romdiv_patch.o \
./src/cr_startup_lpc11uxx.o \
./src/crp.o \
./src/gpio.o \
./src/l11uxx_adc_library.o \
./src/l11uxx_internal_flash_lib.o \
./src/l11uxx_spi_library.o \
./src/l11uxx_timer_lib.o \
./src/l11uxx_uart_library.o \
./src/lcd_5110_lib.o \
./src/nrf24l01_lib.o 

C_DEPS += \
./src/ILI9341.d \
./src/L1UDEV001A.d \
./src/cr_startup_lpc11uxx.d \
./src/crp.d \
./src/gpio.d \
./src/l11uxx_adc_library.d \
./src/l11uxx_internal_flash_lib.d \
./src/l11uxx_spi_library.d \
./src/l11uxx_timer_lib.d \
./src/l11uxx_uart_library.d \
./src/lcd_5110_lib.d \
./src/nrf24l01_lib.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M0 -D__USE_CMSIS=CMSISv2p00_LPC11Uxx -D__LPC11UXX__ -D__REDLIB__ -I"C:\Users\Denry\Documents\LPCXpresso_7.8.0_426\workspace8_01_swd\CMSISv2p00_LPC11Uxx\inc" -I"C:\Users\Denry\Documents\LPCXpresso_7.8.0_426\workspace8_01_swd\L1UDEV001A\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU Assembler'
	arm-none-eabi-gcc -c -x assembler-with-cpp -DDEBUG -D__CODE_RED -DCORE_M0 -D__USE_CMSIS=CMSISv2p00_LPC11Uxx -D__LPC11UXX__ -D__REDLIB__ -I"C:\Users\Denry\Documents\LPCXpresso_7.8.0_426\workspace8_01_swd\CMSISv2p00_LPC11Uxx\inc" -g3 -mcpu=cortex-m0 -mthumb -D__REDLIB__ -specs=redlib.specs -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


