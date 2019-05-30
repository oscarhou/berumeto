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
#define OVERFLOW_TO_MILLI 4
#define SECOND_IN_OVERFLOWS 1000 / OVERFLOW_TO_MILLI

#include <avr/io.h> // This contains the definitions of the terms used
#include <util/delay.h> // This contains the definition of delay function
#include <avr/interrupt.h>
#include <stdlib.h>
#include "sensor_timer.h"
#include "sensor_serial.h"
volatile int update = 0;
volatile unsigned long currCycleStamp = 0;
volatile unsigned char lowReg;
volatile unsigned char highReg;
volatile int lastState = 0;

void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB = (1<<DDB3)|(1<<DDB5) |(1<<DDB1);
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	PORTB |= 0b00000010;
}
char SPI_MasterTransmit(char cData)
{
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
	return SPDR;
}

ISR(TIMER1_CAPT_vect) {
	update = 1;
	currCycleStamp = GetCycleStamp();
	// Rising Edge when ICES1 is 1
	if (TCCR1B & (1 << ICES1))
	{
		lowReg = ICR1L;
		highReg = ICR1H;
		TCCR1B = (TCCR1B & ~(1 << ICES1));
		lastState = 0;
	}
	else // Falling Edge when ICES1 is 0
	{
		lastState = 1;
		TCCR1B = (TCCR1B | (1 << ICES1));
	}
}

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	init_uart();
	SPI_MasterInit();
	InitializeTimer();
	DDRD = DDRD & !(1 << PD7);
	unsigned long start_of = GetCycleStamp();
	unsigned long sampleStart_of = GetCycleStamp();
	unsigned long currStamp = 0;
	unsigned long lowDuration_of = 0;
	float ratio = 0.0f;
	float concentration = 0.0f;
	transmitString("Particle Sensor");
	transmitChar('\r');
	transmitChar('\n');
	// Turn interrupts on.
	sei();
	while(1)
	{
		currStamp = GetCycleStamp();
		unsigned long diff_of = currStamp - start_of;

		if (currStamp - sampleStart_of > 30 * SECOND_IN_OVERFLOWS)
		{
			lowDuration_of += diff_of;
			ratio = lowDuration_of / (30 * SECOND_IN_OVERFLOWS * 10.0f);
			concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
			transmitFloat(concentration);
			transmitChar('\r');
			transmitChar('\n');
			sampleStart_of = currStamp;
			lowDuration_of = 0;
		}

		//transmitByte(SPI_MasterTransmit(0b10001110));
		//transmitByte(SPI_MasterTransmit(0b10000000));
		//transmitByte(SPI_MasterTransmit(0b10000010));
		if (update)
		{
			// if lastState == 0 then that means we detected a rising edge
			if (!lastState)
			{
				transmitString("#Update: Rising Edge\r\n");
				lowDuration_of += currCycleStamp - start_of;
			}
			else // Last state was high, we are falling edge so restart count
			{
				transmitString("#Update: Falling Edge\r\n");
				start_of = currCycleStamp;
			}
			update = 0;
		}
	}
}
