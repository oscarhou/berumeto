#include <avr/io.h> // This contains the definitions of the terms used
#include <stdbool.h>
#include "sensor_serial.h"
#define CYCLES_PER_MILLISECOND F_CPU / 1000
#define OVERFLOW_TO_MILLI 4
#define SECOND_IN_OVERFLOWS 1000 / OVERFLOW_TO_MILLI

typedef struct SampleData_t
{
	unsigned long currStart_tick;
	unsigned long sampleEnd_tick;
	unsigned long levelDuration_tick;
	unsigned long levelDuration_fraction;
	int lastLevel;
	int watchLevel;
	bool bReady;
	bool bStart;
} SampleData;

static volatile unsigned long numTimerOverflow = 0;
static volatile unsigned char lowReg;
static volatile unsigned char highReg;
static volatile SampleData sample;
static volatile unsigned long secondEnd_ticks = 0;
static volatile int secondReady = 0;
static volatile int update = 0;

static const uint16_t kMax16Bit = 65535;
static inline void UpdateSample()
{
	uint16_t tempFraction = (ICR1H << 8) + ICR1L;
	uint32_t tempAdd = tempFraction + sample.levelDuration_fraction;
	sample.levelDuration_tick += numTimerOverflow - sample.currStart_tick;
	if (tempAdd >= kMax16Bit)
	{
		sample.levelDuration_fraction = tempAdd - kMax16Bit;
		numTimerOverflow++;
	}
	else
	{
		sample.levelDuration_fraction = tempAdd;
	}
}

ISR(TIMER1_CAPT_vect) {
	if (!sample.bReady)
	{
		// Rising Edge when ICES1 is 1
		if (TCCR1B & (1 << ICES1))
		{
			if (!sample.bStart && sample.watchLevel == 1)
			{
				sample.bStart = true;
				sample.currStart_tick = numTimerOverflow;
			}
			else if (sample.bStart && sample.watchLevel == 0)
			{
				sample.levelDuration_tick = numTimerOverflow - sample.currStart_tick;
				sample.bReady = true;
			}

			TCCR1B = (TCCR1B & ~(1 << ICES1));
		}
		else // Falling Edge when ICES1 is 0
		{
			if (sample.bStart && sample.watchLevel == 1)
			{
				sample.levelDuration_tick = numTimerOverflow - sample.currStart_tick;
				sample.bReady = true;
			}
			else if (!sample.bStart && sample.watchLevel == 0)
			{
				sample.bStart = true;
				sample.currStart_tick = numTimerOverflow;
			}
			
			TCCR1B = (TCCR1B | (1 << ICES1));
		}
	}
}

ISR(TIMER1_OVF_vect)
{
	if (numTimerOverflow >= sample.sampleEnd_tick)
	{
		if (sample.bStart)
			sample.levelDuration_tick = numTimerOverflow - sample.currStart_tick;

		sample.bReady = true;
	}

	numTimerOverflow++;
}

unsigned long GetWatchLevelTime()
{
	return sample.levelDuration_tick;
}

int HasSecondPassed()
{
	int returnSec = update;
	if (update)
	{
		update = 0;
		secondEnd_ticks = numTimerOverflow + 1000 / OVERFLOW_TO_MILLI;
	}
	return returnSec;
}

void InitializeTimer()
{
	TIMSK1 |= (1<<ICIE1) | (1<<TOIE1);
	// triggering of the interrupt service routine.
	// Setting Clock source of timer to be ClkIO/1
	TCCR1B |= 1 << (CS10);
}

unsigned long GetTimeStampMs()
{
	return numTimerOverflow * OVERFLOW_TO_MILLI;
}

void StartLevelTimer(int watchLevel, unsigned long duration_ms)
{
	sample.bReady = false;
	sample.bStart = false;
	sample.watchLevel = watchLevel;
	sample.levelDuration_tick = 0;
	sample.currStart_tick = numTimerOverflow;
	// FIXME: Better check that overflow boi
	sample.sampleEnd_tick = numTimerOverflow + (duration_ms / OVERFLOW_TO_MILLI);
}

int IsDataReady()
{
	return sample.bReady;
}

unsigned long GetLevelDurationMs()
{
	if (!sample.bReady)
		return 0;
	else
	{
		return sample.levelDuration_tick * OVERFLOW_TO_MILLI;
	}
}

void TransmitState()
{
	transmitString("Info:");
	transmitChar('-');
	transmitLongAsDec(sample.levelDuration_tick * OVERFLOW_TO_MILLI);
	transmitChar('-');
	transmitLongAsDec(numTimerOverflow * OVERFLOW_TO_MILLI);
	transmitChar('-');
	transmitLongAsDec(sample.currStart_tick * OVERFLOW_TO_MILLI);
	transmitChar('-');
	transmitLongAsDec(sample.sampleEnd_tick * OVERFLOW_TO_MILLI);
	transmitChar('-');

}

