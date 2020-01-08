CC      = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS  = -Os -mmcu=atmega328p

blink.hex: blink.elf
	    ${OBJCOPY} -O ihex -R .eeprom blink.elf blink.hex

blink.elf: blink.o
	    ${CC} -o blink.elf blink.o

install: blink.hex
	    avrdude -c atmelice_isp -p ATMEGA328P -U flash:w:blink.hex
