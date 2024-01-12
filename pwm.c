/* pwm.c
 *
 */ 

#include <stdio.h>
#include <avr/interrupt.h>
#include "blade_state.h"
#include "pwm.h"

// keep track of current blade color bit depth reduction
volatile uint8_t color_derez = SINGLE_COLOR_DEREZ;

// pwm_handler() - responsible for PWMing the segments and changing the color PWM values when in multi-color mode
//
// TimerA-based overflow interrupt service request (ISR)
//
// this triggers off the same timer used to drive PWM of colors. this is critical for keeping color
// and segment PWM operations in sync.
ISR(TCA0_LUNF_vect) {
  uint8_t current_segment = 0;
  uint8_t current_segment_step = 0;
  uint8_t status = 0;
  uint8_t reg_PORTA = PORTA.OUT;
  uint8_t r,g,b;
  
  // a timer/counter used to manage segment color and brightness
  static uint8_t seg_timer = 0;
  static uint8_t last_segment = 255;

  // whether single or multi-color mode, each segment can be PWM'd
  // pwm_handler loops over each segment
  // (1<<SEG_REZ) represents the number of ticks of seg_timer per 1 full PWM period for a segment

  // increment segment timer
  seg_timer++;

  // calculate the current segment and segment step; segment step is used to control the segment's duty cycle
  current_segment = (seg_timer >> SEG_REZ) & 0x03;
  current_segment_step = seg_timer & ((1 << SEG_REZ) - 1);

  // start handling of a new segment of the blade
  if (current_segment != last_segment) {

    // disable all segments (those that should be enabled will be enabled later on)
    reg_PORTA |= SEG1_PIN_bm | SEG2_PIN_bm | SEG3_PIN_bm | SEG4_PIN_bm;

    // if blade is in multi-color mode, or if blade is on segment 0 (in single-color mode), set the segment/blade color
    if (color_derez != SINGLE_COLOR_DEREZ || current_segment == 0) {

      // set the current segment's color;
      if (   RED_VAL != DEREZ(segment_color[current_segment][RED_IDX])
        || GRN_VAL != DEREZ(segment_color[current_segment][GRN_IDX])
        || BLU_VAL != DEREZ(segment_color[current_segment][BLU_IDX])
      ) {

        // calculate new RGB values and store them for later
        r = DEREZ(segment_color[current_segment][RED_IDX]);
        g = DEREZ(segment_color[current_segment][GRN_IDX]);
        b = DEREZ(segment_color[current_segment][BLU_IDX]);

        // set status flag
        status = 1;
      }
    }
    last_segment = current_segment;
  }

  // control the current segment's brightness by determining when to turn it on
  if (color_derez != SINGLE_COLOR_DEREZ) {
    if ( true_segment_brightness[current_segment] > 0 && ((~(true_segment_brightness[current_segment])>>(8-SEG_REZ)) & ((1 << SEG_REZ) - 1)) <= current_segment_step ) {
      switch (current_segment) {
        case 0:
          reg_PORTA &= ~(SEG1_PIN_bm);
          break;
        case 1:
          reg_PORTA &= ~(SEG2_PIN_bm);
          break;
        case 2:
          reg_PORTA &= ~(SEG3_PIN_bm);
          break;
        case 3:
          reg_PORTA &= ~(SEG4_PIN_bm);
          break;
      }
    }

  // in single-color mode all segmenets must be handled; there is no "current segment"
  } else {

    // enable segment 1
    if ( true_segment_brightness[0] > 0 && ((~(true_segment_brightness[0])>>(8-SEG_REZ)) & ((1 << SEG_REZ) - 1)) <= current_segment_step ) {
      reg_PORTA &= ~SEG1_PIN_bm;
    }

    // enable segment 2
    if ( true_segment_brightness[1] > 0 && ((~(true_segment_brightness[1])>>(8-SEG_REZ)) & ((1 << SEG_REZ) - 1)) <= current_segment_step ) {
      reg_PORTA &= ~SEG2_PIN_bm;
    }

    // enable segment 3
    if ( true_segment_brightness[2] > 0 && ((~(true_segment_brightness[2])>>(8-SEG_REZ)) & ((1 << SEG_REZ) - 1)) <= current_segment_step ) {
      reg_PORTA &= ~SEG3_PIN_bm;
    }

    // enable segment 4
    if ( true_segment_brightness[3] > 0 && ((~(true_segment_brightness[3])>>(8-SEG_REZ)) & ((1 << SEG_REZ) - 1)) <= current_segment_step ) {
      reg_PORTA &= ~SEG4_PIN_bm;
    }
  }

  // apply color change and reset PWM counter just before setting segments
  // this is done to minimize delay between color and segment changes which becomes
  // especially critical at higher PWM frequencies
  if (status) {
    RED_VAL = r;
    GRN_VAL = g;
    BLU_VAL = b;
    TCA0.SPLIT.LCNT = 0;
  }

  // copy the local PORTA.OUT value back; all segments are updated at the same time
  PORTA.OUT = reg_PORTA;

  // clear interrupt flag
  TCA0.SPLIT.INTFLAGS |= (1 << TCA_SPLIT_HUNF_bp);
}

// set environment for multi-color blade
void set_multi_mode(void) {
  color_derez = MULTI_COLOR_DEREZ;
  TCA0.SPLIT.LPER = DEREZ(PWM_MAX);
  TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESTART_gc | 0x03;
}

// set environment for single-color blade
void set_single_mode(void) {
  color_derez = SINGLE_COLOR_DEREZ;
  TCA0.SPLIT.LPER = DEREZ(PWM_MAX);
  TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESTART_gc | 0x03;
}

// initialize RGB pins
void pwm_setup(void) {

  //RGB_PORT.DIRSET = (RED_PIN_bm | GRN_PIN_bm | BLU_PIN_bm);
  RGB_PORT.OUTCLR = (RED_PIN_bm | GRN_PIN_bm | BLU_PIN_bm);

  // initialize SEGMENT pins
  //SEG_PORT.DIRSET = (SEG1_PIN_bm | SEG2_PIN_bm | SEG3_PIN_bm | SEG4_PIN_bm);
  SEG_PORT.OUTSET = (SEG1_PIN_bm | SEG2_PIN_bm | SEG3_PIN_bm | SEG4_PIN_bm);

  // use alternative output pins for WO0/1/2 for RGB pins to make routing the PCB easier
  PORTMUX.CTRLC = PORTMUX_TCA00_ALTERNATE_gc  // WO0 = PB3
                | PORTMUX_TCA01_ALTERNATE_gc  // WO1 = PB4
                | PORTMUX_TCA02_ALTERNATE_gc; // WO2 = PB5

  // setup timer A
  TCA0.SPLIT.CTRLD = (1 << TCA_SPLIT_SPLITM_bp);  // enable split mode
  TCA0.SPLIT.LPER  = DEREZ(PWM_MAX);              // set PWM period
  TCA0.SPLIT.LCMP0 = 0;                           // initialize CMP for WO0-2 to 0
  TCA0.SPLIT.LCMP1 = 0;
  TCA0.SPLIT.LCMP2 = 0;
  TCA0.SPLIT.CTRLB = RED_PWMEN_bm                 // enable PWM for RED
                   | GRN_PWMEN_bm                 // enable PWM for GREEN
                   | BLU_PWMEN_bm;                // enable PWM for BLUE
  TCA0.SPLIT.INTCTRL |= (1 << TCA_SPLIT_LUNF_bp); // enable LOW UNDERFLOW interrupt, use for timing segment PWM
  TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV8_gc     // set prescaler to 8
                   | (1 << TCA_SPLIT_ENABLE_bp);  // and enable timer A
}
