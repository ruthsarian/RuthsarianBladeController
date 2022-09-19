/* dmode_handler.h
 *
 * set defines that control display mode operations and define any global functions and variables that would
 * be accessed from outside dmode code
 */ 

#ifndef DMODE_HANDLER_H_
#define DMODE_HANDLER_H_

// Display Modes (DMODE)
#define DMODE_STOCK						0
#define DMODE_COLOR_PICKER				1
#define DMODE_COLOR_PICKER_PICKED		2
#define DMODE_BLADE_WHEEL				3
#define DMODE_SEGMENT_WHEEL				4
#define DMODE_MULTI_MODE				5
#define DMODE_MAX						6	// maximum dmode value; once reached, dmode will reset to 0
											// changing this value? be sure to update RESET_THRESHOLD_COUNT in blade_state.h

// Display Sub-Modes (DSUBMODE)
#define DSUBMODE_NORMAL					0
#define DSUBMODE_FLICKER_FULL			1
#define DSUBMODE_FLICKER_SEGMENTED		2
#define DSUBMODE_FLICKER_BRIGHT			3
#define DSUBMODE_FLICKER_DARK			4
#define DSUBMODE_FLICKER_GRADIENT		5
#define DSUBMODE_BREATHING				6
#define DSUBMODE_STATIC_GRADIENT_1		7
#define DSUBMODE_STATIC_GRADIENT_2		8
#define DSUBMODE_BRIGHTNESS_66			9
#define DSUBMODE_BRIGHTNESS_33			10
#define DSUBMODE_BRIGHTNESS_10			11
#define DSUBMODE_MAX					12	// a cheap way to keep track of how many display sub-modes there are

// DMODE Timing Elements
#define DMODE_THRESHOLD_TIME			1000	// remain powered off for less than this value in milliseconds to increment display mode (DMODE)
#define DSUBMODE_THRESHOLD_TIME			3000	// remain powered off for less than this value in milliseconds, but more than DSUBMODE_THRESHOLD_TIME, to increment display sub-mode (DSUBMODE)

// Dynamic Color Picker presets, constraints, and basic formula
#define DCP_BRIGHTNESS_LEVELS			5														// recommend this be an odd value below 8, must have a value greater than 0
#define DCP_MAX_COLORSPACE				255
#define DCP_COLOR_COUNT					(uint16_t)(DCP_MAX_COLORSPACE / DCP_BRIGHTNESS_LEVELS)	// the size of the color value space per brightness level. must be uint16_t because brightness could be 1
#define DCP_FORMULA_SEPARATOR			(uint8_t)(DCP_COLOR_COUNT / 3)							// the point at which different color formulas (red, green, blue) come into play
#define DCP_MIDDLE_LEVEL				(uint8_t)((DCP_BRIGHTNESS_LEVELS - 1) / 2)				// identify the brightness level that represents the 'middle' or 'normal' level of brightness
#define DCP_MAX_STEP					(uint8_t)(DCP_FORMULA_SEPARATOR / 2)					// this sets a lower limit on the number of available colors in the color picker (6)
#define DCP_STEP_TABLE_MAX				32														// set a limit to the size of the color picker step lookup table; should probably be 16 not 32

// Ensure DCP_BRIGHTNESS_LEVELS is never less than one
#if (DCP_BRIGHTNESS_LEVELS < 1)
#error "DCP_BRIGHTNESS_LEVELS must have a value greater than 0."
#endif

#ifdef __cplusplus
extern "C" {
#endif

// manage custom display modes and animations
void dmode_handler(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* DMODE_HANDLER_H_ */