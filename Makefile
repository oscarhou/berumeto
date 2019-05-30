CC=avr-gcc
OBJCOPY=avr-objcopy
SIZE=avr-size
MMCU=-mmcu=atmega328p
DEBUG_FLAGS=-g
C_FLAGS=-Os
LINK_FLAGS=-Wl,-u,vfprintf -lprintf_flt


sensor.o: sensor.c sensor_timer.h sensor_serial.h
	# Compile into object file
	$(CC) $(DEBUG_FLAGS) $(C_FLAGS) $(MMCU) -c sensor.c
	$(CC) $(DEBUG_FLAGS) $(LINK_FLAGS) $(MMCU) -o sensor.elf sensor.o
	$(OBJCOPY) -j .text -j .data -O ihex sensor.elf sensor.hex
	$(SIZE) --format=avr --mcu=atmega328p sensor.elf

all: sensor.o


clean:
	rm *.hex
	rm *.o
	rm *.elf




