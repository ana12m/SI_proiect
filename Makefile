# Makefile for ATmega328P Project
# Sistem de Control Ambiental Inteligent

# Microcontroller Settings
MCU = atmega328p
F_CPU = 16000000UL

# Programmer Settings
PROGRAMMER = arduino
PORT = COM3
BAUD = 57600

# Board Selection (default to nano)
BOARD ?= nano

# Compiler Settings
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

# Directories
OBJDIR = obj
BINDIR = bin

# Compiler Flags
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=gnu99
CFLAGS += -I. -Idrivers/gpio -Idrivers/interrupt -Idrivers/timer \
          -Idrivers/eeprom -Idrivers/adc -Idrivers/pwm -Ibsp -Iutils

ifeq ($(BOARD), nano)
    CFLAGS += -DBOARD_NANO
else ifeq ($(BOARD), uno)
    CFLAGS += -DBOARD_UNO
else
    $(error Invalid BOARD specified. Use 'nano' or 'uno')
endif

# Source Files
SRC = src/main.c \
      drivers/gpio/gpio.c \
      drivers/interrupt/external_interrupt.c \
      drivers/timer/timer0.c \
      drivers/timer/timer1.c \
      drivers/timer/timer2.c \
      drivers/pwm/pwm.c \
      drivers/eeprom/eeprom.c \
      drivers/adc/adc.c \
      utils/delay.c

# Object Files
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

# Target Name
TARGET = $(BINDIR)/main

# -------------------------------------------------------
# Build Rules
# -------------------------------------------------------
all: directories $(TARGET).hex

directories:
	@mkdir -p $(BINDIR)
	@mkdir -p $(OBJDIR)/src
	@mkdir -p $(OBJDIR)/drivers/gpio
	@mkdir -p $(OBJDIR)/drivers/interrupt
	@mkdir -p $(OBJDIR)/drivers/timer
	@mkdir -p $(OBJDIR)/drivers/eeprom
	@mkdir -p $(OBJDIR)/drivers/adc
	@mkdir -p $(OBJDIR)/drivers/pwm
	@mkdir -p $(OBJDIR)/utils

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

flash: $(TARGET).hex
	$(AVRDUDE) -v -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -D -U flash:w:$<:i

clean:
	rm -rf $(OBJDIR) $(BINDIR)

# -------------------------------------------------------
# Test Rules
# -------------------------------------------------------
TEST_FLAGS = -I. -Isrc -Idrivers/gpio -Idrivers/interrupt -Idrivers/timer \
             -Idrivers/eeprom -Idrivers/adc -Idrivers/pwm -Ibsp -Iutils \
             -Itest -Itest/mocks -DUNIT_TEST

test_gpio: directories
	@mkdir -p $(BINDIR)/test
	gcc $(TEST_FLAGS) -o $(BINDIR)/test/test_gpio \
	    test/test_gpio.c test/mocks/registers.c
	@echo "Running GPIO Tests..."
	@./$(BINDIR)/test/test_gpio

test_pwm: directories
	@mkdir -p $(BINDIR)/test
	gcc $(TEST_FLAGS) -o $(BINDIR)/test/test_pwm \
	    test/test_pwm.c drivers/timer/timer1.c drivers/timer/timer2.c \
	    test/mocks/registers.c
	@echo "Running PWM Tests..."
	@./$(BINDIR)/test/test_pwm

test_pwm_wrapper: directories
	@mkdir -p $(BINDIR)/test
	gcc $(TEST_FLAGS) -o $(BINDIR)/test/test_pwm_wrapper \
	    test/test_pwm_wrapper.c drivers/timer/timer1.c drivers/timer/timer2.c \
	    drivers/pwm/pwm.c test/mocks/registers.c
	@echo "Running PWM Wrapper Tests..."
	@./$(BINDIR)/test/test_pwm_wrapper

test: test_gpio test_pwm test_pwm_wrapper
	@echo "All tests passed!"

.PHONY: all flash clean directories test test_gpio test_pwm test_pwm_wrapper