/* device_config.h
 *
 */ 

#ifndef DEVICE_CONFIG_H_
#define DEVICE_CONFIG_H_

// I/O pin for managing the RGB LED strip voltage regulator; PC1
#define LDO_PORT					PORTC
#define LDO_PIN_bm					PIN1_bm

// I/O pin for switch used to enable eeprom write protect
#define SW_WRITE_PROTECT_PORT		PORTB
#define SW_WRITE_PROTECT_PIN_bm		PIN0_bm
#define SW_WRITE_PROTECT_PIN_CTRL	PIN0CTRL
#define SW_WRITE_PROTECT_bp			0

// I/O pin for switch used to disable dmode
#define SW_DMODE_DISABLE_PORT		PORTB
#define SW_DMODE_DISABLE_PIN_bm		PIN1_bm
#define SW_DMODE_DISABLE_PIN_CTRL	PIN1CTRL
#define SW_DMODE_DISABLE_bp			1

// how many milliseconds after power off before putting the MCU to sleep
#define OFF_TO_SLEEP_TIME			10000

#ifdef __cplusplus
extern "C" {
#endif

// global used to keep track of the hardware switches attached to the blade
extern uint8_t switch_config;

// perform MCU and program initialization
void device_setup(void);

// Use an unconnected ADC pin to generate noise (in theory) to seed
// the random number generator. In theory.
void srand_init(void);

// sample rand() 100 times and write the output to serial for purpose of 
// auditing seeding of RNG via srand_init()
void srand_sample_report(void);

// enable the LDO that powers the blade's LEDs
void blade_power_on(void);

// disable the LDO that powers the blade's LEDs
void blade_power_off(void);

// get MCU Vcc
float measure_vcc(void);

// read the state of hardware switches attached to the blade
// this is only performed during power-up as it is assumed the switches
// are not accessible while the blade is active.
void record_switch_config(void);

// report the recorded state of hardware switches
void switch_report(void);

// determine when it is time for the MCU to go to sleep
void sleep_handler(void);

// handle the blade while in a reset state
uint8_t reset_handler(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* DEVICE_CONFIG_H_ */