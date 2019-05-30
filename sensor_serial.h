#include <stdio.h>
char NUM[10] = {'0', '1', '2','3', '4', '5', '6', '7', '8', '9'};
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

void transmitString(unsigned char* string)
{
	int count = 0;
	for (count = 0; string[count] != '\0' ; count++)
	{
		transmitChar(string[count]);
	}
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

uint16_t bytesToUint16(uint8_t lowByte, uint8_t highByte)
{
	return (uint16_t)(highByte << 8 | lowByte);
}

void transmitByteAsDec(uint16_t toDec)
{
	uint16_t tenPower = 10000;
	unsigned char hasNum = 0;
	while(toDec > 0)
	{	
		uint16_t currDigit = toDec / tenPower;
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

void transmitLongAsDec(unsigned long toDec)
{
	if (toDec == 0)
		transmitChar(NUM[0]);

	unsigned long tenPower = 1000000000UL;
	unsigned char hasNum = 0;

	while(tenPower > 0)
	{
		unsigned long currDigit = toDec / tenPower;
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

void transmitFloat(float val)
{
	unsigned char printArray[50];
	sprintf(printArray, "%f", val);
	transmitString(printArray);
}
