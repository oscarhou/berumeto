#!/bin/bash
HEX_PATH=$1
avrdude -c arduino -p atmega328p -P /dev/ttyUSB0 -b 57600 -U flash:w:$HEX_PATH:i -D
