#include <avr/io.h> // This contains the definitions of the terms used
volatile unsigned long numTimerOverflow = 0;

#define CYCLES_PER_MILLISECOND F_CPU / 1000


ISR(TIMER1_OVF_vect)
{
	numTimerOverflow++;
}

void InitializeTimer()
{
	TIMSK1 |= (1<<ICIE1) | (1<<TOIE1);
	// triggering of the interrupt service routine.
	// Setting Clock source of timer to be ClkIO/1
	TCCR1B |= 1 << (CS10);
}

unsigned long GetCycleStamp()
{
	return numTimerOverflow;
}

