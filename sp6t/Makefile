NAME    = sp6t
OBJ     = $(NAME).o uart.o

# options
CC      = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS  = -Os -mmcu=atmega328p -pedantic -Wall -std=c99

$(NAME).hex: $(NAME).elf
	    ${OBJCOPY} -O ihex -R .eeprom $(NAME).elf $(NAME).hex

$(NAME).elf: $(OBJ)
	    ${CC} -o $(NAME).elf $(OBJ)

install: $(NAME).hex
	    avrdude -c atmelice_isp -p ATMEGA328P -U flash:w:$(NAME).hex

clean:
	rm -f *.elf *.o *.hex
