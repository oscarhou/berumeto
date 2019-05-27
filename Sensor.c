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
volatile int update = 0;
char NUM[10] = {'0', '1', '2','3', '4', '5', '6', '7', '8', '9'};
	
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

void InitializeInputCaptureTimer()
{
	TIMSK1 |= (1<<ICIE1);
}

void init_uart()
{
		/*Set baud rate */
		UBRR0H = (unsigned char)((UBRRVAL)>>8);
		UBRR0L = (unsigned char)UBRRVAL;
	//	UCSR0A = ~(_BV(U2X0));
		/*Enable receiver and transmitter */
		UCSR0B = (1<<RXEN0)|(1<<TXEN0);
		/* Set frame format: 8data, 1stop bit */
		UCSR0C = (3<<UCSZ00);
		
}

void transmitChar(unsigned char val)
{
		/* Wait for empty transmit buffer */
		while ( !( UCSR0A & (1 <<(UDRE0))) )
		;
		/* Put data into buffer, sends the data */
		UDR0 = val;
}

void transmitByte(char toBits)
{
	int numBits =0;
	for(; numBits < 8; numBits++)
	{
		if (toBits & (1 << numBits))
		{
			transmitChar('1');
		}
		else 
		{
			transmitChar('0');
		}
	}
	transmitChar('\n');
}

void transmitByteAsDec(unsigned char toDec)
{
	unsigned char tenPower = 100;
	unsigned char hasNum = 0;
	for(int i = 0; i < 3; i++)
	{	
		unsigned char currDigit = toDec / tenPower;
		toDec -= currDigit * tenPower;
		if (!hasNum && currDigit > 0)
		{
			hasNum = 1;
		}
		
		if (currDigit > 0 || hasNum)
			transmitChar(NUM[currDigit]);
			
		tenPower = tenPower / 10;
	}

}
ISR(TIMER1_CAPT_vect) {
	update = 1;
}

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	init_uart();
	SPI_MasterInit();
	InitializeInputCaptureTimer();

	// triggering of the interrupt service routine.
	TCCR1B |= 1 << (CS10);

	// Turn interrupts on.
	sei();
	while(1)
	{
		//transmitByte(SPI_MasterTransmit(0b10001110));
		//transmitByte(SPI_MasterTransmit(0b10000000));
		//transmitByte(SPI_MasterTransmit(0b10000010));

		 _delay_ms(1000); 
	//	PORTD = 0xff;

		if (update) {
			volatile unsigned char lowReg = ICR1L;
			volatile unsigned char highReg = ICR1H;
		transmitByteAsDec(lowReg);
		transmitChar('-');
		transmitByteAsDec(highReg);
		transmitChar('\n');


			
		

			update = 0;
		}
	}
	
	/* Insert application code here, after the board has been initialized. */
}
