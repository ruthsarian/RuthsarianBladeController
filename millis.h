/*
 * millis.h
 *
 */ 

#ifndef MILLIS_H_
#define MILLIS_H_

#ifdef __cplusplus
extern "C" {
#endif
	
extern volatile uint32_t timer_millis;

uint32_t millis(void);
uint32_t micros(void);
void millis_setup(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* MILLIS_H_ */