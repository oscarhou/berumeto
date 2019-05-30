#!/bin/bash
MAIN_AVR_HEADERS=`find /usr/lib/avr/include/* ! -path "/usr/lib/avr/include/avr*"`
AVR_IO_HEADERS=`find /usr/lib/avr/include/avr/* ! -name io*`

echo $MAIN_AVR_HEADERS
echo $AVR_IO_HEADERS


ctags-exuberant -R . $MAIN_AVR_HEADERS $AVR_IO_HEADERS /usr/lib/avr/include/avr/iom328p.h /usr/lib/avr/include/avr/io.h
