/* eeprom.h
 *
 */ 


#ifndef EEPROM_H_
#define EEPROM_H_

#define EEPROM_START_ADDR   0x00
#define EEPROM_MAGIC_LEN	4

#ifdef __cplusplus
extern "C" {
#endif

extern const char eeprom_magic[EEPROM_MAGIC_LEN];

void eeprom_dump(void);
void eeprom_setup(void);
void eeprom_reset(void);
void eeprom_load_state(void);
void eeprom_store_state(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* EEPROM_H_ */