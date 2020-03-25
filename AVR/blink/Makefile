CC      = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS  = -Os -mmcu=atmega328p -DF_CPU=16000000UL

blink.hex: blink.elf
	    ${OBJCOPY} -O ihex -R .eeprom blink.elf blink.hex

blink.elf: blink.o
	    ${CC} -o blink.elf blink.o

install: blink.hex
	    avrdude -c usbasp -p ATMEGA328P -U flash:w:blink.hex
