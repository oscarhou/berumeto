/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#define F_CPU 16000000UL
#define BAUD 9600UL
//#define UBRRVAL (F_CPU/(8UL*BAUD) - 1)/2 // This calculation worked originally when the below calculation didn't work........
#define UBRRVAL (F_CPU/(16UL*BAUD) - 1) 

#include <avr/io.h> // This contains the definitions of the terms used
#include <util/delay.h> // This contains the definition of delay function
#include <avr/interrupt.h>
#include <stdlib.h>
#include "sensor_timer.h"
#include "sensor_serial.h"
#include "sensor_spi.h"

float getConcentration(unsigned long lowDuration_ms, unsigned long sampleTime_ms)
{
	float ratio = (float)(lowDuration_ms) / (float)(sampleTime_ms);
	transmitString("ratio is ");
	transmitFloat(ratio);
	transmitChar('\r');
	transmitChar('\n');
	return 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
}

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	init_uart();
	SPI_MasterInit();
	InitializeTimer();
	DDRD = DDRD & !(1 << PD7);
	transmitString("Particle Sensor\r\n");
	StartLevelTimer(0, 30000);
	// Turn interrupts on.
	unsigned long lowDuration_ms = 0;
	unsigned long endTime = GetTimeStampMs() + 30000;
	sei();
	while(1)
	{
		if (IsDataReady())
		{
			transmitString("Done Pulse\r\n");
			unsigned long test = GetLevelDurationMs();
			transmitLongAsDec(test);
			transmitString("\r\n");
			lowDuration_ms += GetLevelDurationMs();
			transmitChar('\r');
			transmitChar('\n');
			TransmitState();
			transmitChar('\r');
			transmitChar('\n');

			if (GetTimeStampMs() > endTime)
			{
				transmitString("Done All, final location ms is ");
				transmitLongAsDec(lowDuration_ms);
				transmitString("\r\n");
				transmitFloat(getConcentration(lowDuration_ms, 30000));
				transmitString("\r\n");
				lowDuration_ms = 0;
				endTime = GetTimeStampMs() + 30000;
			}

			StartLevelTimer(0, 30000);
		}

	}
}
