/* millis.c
 *
 * This code adds basic support for millis() and micros() functions that are 
 * compatible with similarly named functions found in the arduino library.
 *
 * TimerB will be used for millis() and micros() as TimerA will be used for PWM
 * 
 * This code is based on portions of megaTinyCore by SpenceKonde
 * https://github.com/SpenceKonde/megaTinyCore
 *
 * References:
 * https://raw.githubusercontent.com/SpenceKonde/megaTinyCore/master/megaavr/cores/megatinycore/wiring.c
 * https://raw.githubusercontent.com/SpenceKonde/megaTinyCore/master/megaavr/cores/megatinycore/timers.h
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "millis.h"

#ifndef F_CPU
	#error "F_CPU not defined!"
#endif

volatile uint32_t timer_millis = 0;		// global 32-bit value where milliseconds will be stored

ISR(TCB0_INT_vect) {					// TCB0 interrupt
	timer_millis++;						// increment millis
	TCB0.INTFLAGS = TCB_CAPT_bm;		// clear interrupt flag
}

// get number of milliseconds since start
uint32_t millis() {
	uint32_t m;					// local storage of value to be returned
	uint8_t status = SREG;		// backup SREG prior to disabling interrupts
	cli();						// disable interrupts
	m = timer_millis;			// make a copy of current millis value
	SREG = status;				// restore SREG (which re-enabled interrupts)
	return m;					// return the millis value
}

// get number of microseconds since start
uint32_t micros() {
	uint32_t overflows, microseconds;
	uint16_t ticks;
	uint8_t flags;
	uint8_t status = SREG;				// backup SREG
	cli();								// disable interrupts
	flags = TCB0.INTFLAGS;				// get timerb flags
	ticks = TCB0.CNT;					// get current ticks
	overflows = timer_millis;			// get current milliseconds
	SREG = status;						// restore SREG (enable interrupts)

	// if timer interrupt flag is set and ticks is low, then the ISR has not yet fired so increment overflows
	if ((flags & TCB_CAPT_bm) && !(ticks & 0xFF00)) {
		overflows++;
	}

	// only care about 20MHz and 10MHz cases; look at wiring.c from megaTinyCore if other clock values are needed
	#if (F_CPU==20000000UL)
		ticks = ticks >> 3;
		microseconds = overflows * 1000 + (ticks - (ticks >> 2) + (ticks >> 4) - (ticks >> 6));
	#elif (F_CPU==10000000UL)
		ticks = ticks >> 2;
		microseconds = overflows * 1000 + (ticks - (ticks >> 2) + (ticks >> 4) - (ticks >> 6));
	#endif

	return microseconds;
}

void millis_setup() {

	// setup timer B
	TCB0.CTRLB   = 0;						// set timer mode to periodic interrupt mode
	TCB0.CCMP    = (F_CPU/2000)-1;			// some timer value, we'll figure out a better value later
	TCB0.INTCTRL = TCB_CAPT_bm;				// enable TCB0 interrupt
	TCB0.CTRLA   = TCB_CLKSEL_CLKDIV2_gc	// set prescaler to 2
	             | TCB_ENABLE_bm;			// and enable timer B
}