NAME    = serial
MCU     = atmega328p
F_CPU   = 16000000UL

CFLAGS  = -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Wall -pedantic -std=c99

# programming

$(NAME).hex: $(NAME).elf
	avr-objcopy -O ihex -R .eeprom $(NAME).elf $(NAME).hex

program: $(NAME).hex
	avrdude -c atmelice_isp -p $(MCU) -U flash:w:$(NAME).hex

# linking

$(NAME).elf: $(NAME).o
	avr-gcc -mmcu=$(MCU) $^ -o $@

# compiling

$(NAME).o: $(NAME).c
	avr-gcc ${CFLAGS} -c $(NAME).c

# misc

clean:
	rm -f *.elf *.hex *.o
