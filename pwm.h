/* pwm.h
 *
 */ 

#ifndef PWM_H_
#define PWM_H_

// I/O pin configuration for RGB LED strip color channels
#define RGB_PORT              PORTB
#define RED_PIN_bm            PIN5_bm               // PB5, WO2
#define GRN_PIN_bm            PIN4_bm               // PB4, WO1
#define BLU_PIN_bm            PIN3_bm               // PB3, WO0
#define RED_PWMEN_bm          TCA_SPLIT_LCMP2EN_bm  // bitmask used to enable/disable PWM for the colors
#define GRN_PWMEN_bm          TCA_SPLIT_LCMP1EN_bm  //   i use these defines to try and make it easier on me to change pins later on
#define BLU_PWMEN_bm          TCA_SPLIT_LCMP0EN_bm  //   as i design the PCB and discover different pins offer easier trace routing

// I/O pin configuration for RGB LED strip segments
#define SEG_PORT              PORTA
#define SEG1_PIN_bm           PIN7_bm               // PA7, "GP1" on stock blade
#define SEG2_PIN_bm           PIN6_bm               // PA6, "GP2" on stock blade
#define SEG3_PIN_bm           PIN5_bm               // PA5, "GP3" on stock blade
#define SEG4_PIN_bm           PIN4_bm               // PA4, "GP4" on stock blade

// these are the compare registers used to control PWM duty cycle for the color channels
#define RED_VAL               TCA0.SPLIT.LCMP2      // RED;
#define GRN_VAL               TCA0.SPLIT.LCMP1      // GRN;
#define BLU_VAL               TCA0.SPLIT.LCMP0      // BLU;

// manage bit depth for blade colors and segment brightness
#define SINGLE_COLOR_DEREZ    1                     // number of bits to reduce color bit depth in single-color mode; greater bit depth = slower PWM frequency
#define MULTI_COLOR_DEREZ     3                     // number of bits to reduce color bit depth in multi-color mode; (MIN: SINGLE_COLOR_DEREZ + 2)
#define DEREZ(X)              (X>>color_derez)      // a macro to make the process of "DEREZ-ing" bit depth values easier
#define PWM_MAX               0xFE                  // the maximum value the PWM timer can hold
#define SEG_REZ               3                     // bit depth for segment brightness

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t color_derez;

void set_multi_mode(void);
void set_single_mode(void);
void pwm_setup(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* PWM_H_ */