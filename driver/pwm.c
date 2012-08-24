/*
 * pwm.c
 *
 * Created: 28.07.2012 18:08:27
 *  Author: OliverS
 *
 * $Id$
 */ 

#include "global.h"
#include "rx.h"
#include <avr/pgmspace.h>

uint16_t PWM[8];
static uint16_t _lastStart, _lastLoStart;

prog_uchar masktable[] = {_BV(OUT1_BIT), _BV(OUT2_BIT), _BV(OUT3_BIT), _BV(OUT4_BIT), _BV(OUT5_BIT), _BV(OUT6_BIT), _BV(OUT7_BIT), _BV(OUT8_BIT)};

__attribute__ ((section(".lowtext")))
ISR(TIMER1_COMPA_vect)
{
	static int8_t _index;
	uint16_t t = TCNT1;
	uint16_t t2 = millis();
	uint8_t loActive = OFF;
	
	if (t2 - _lastLoStart > 20)
	{
		_lastLoStart = t2;
		loActive = ON;
	}
	
	if (PWM[_index] && ((Config.Mixer.Channel[_index].flags & FLAG_HIGH) || loActive ))
	{
		OUT_PORT = pgm_read_byte(&masktable[_index]);
		OCR1A = (uint16_t)(t + (PWM[_index]));
	}
	else
	{
		OUT_PORT = 0;
		OCR1A = t + 100;
	}
		
	_index = (_index + 1) & 0x07;
	if (!_index)
		_lastStart = t;
}

void pwmInit()
{
	OUT_DDR = 0xFF;			// all pins to output mode
	OUT_PORT = 0x00;		// all OFF
	
	OCR1A = TCNT1;
	TIFR1 |= _BV(OCF1A);	// clear OCA interrupt
	TIMSK1 |= _BV(OCIE1A);	// enable OCA interrupt on timer1
}

void pwmWrite(uint8_t channel, uint16_t value)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
		PWM[channel] = value * TICKSPERMICRO;
};