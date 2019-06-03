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

