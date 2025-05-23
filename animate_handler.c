/* animate_handler.c
 *
 * provides stock blade behavior
 *   - animate blade ignition
 *   - animate blade extinguish
 *   - animate blade clash
 *   - animate flicker effect for kylo ren legacy blade
 *   - set stock blade color based on command received from hilt
 *   - set blade state to ON after ignition animation completes
 *   - set blade state to OFF after extinguish animation completes
 *
 * function was moved into its own C file 
 */

#include <stdio.h>
#include "millis.h"
#include "serial.h"
#include "device_config.h"
#include "blade_state.h"

// manages changes in the blade display during stock animation effects (ignition, extinguish, clash)
void animate_handler(void) {
  static uint32_t next_event_time = 0;
  uint8_t state, state_step;

  // determine blade state
  state = blade.state & 0xF0;

  // are we in a state that animates?
  if (state != BLADE_STATE_OFF && state != BLADE_STATE_ON ) {

    // determine current step of animation
    state_step = blade.state & 0x0F;

    // determine which animation operation is being performed
    switch (state) {

      // the blade is in a clash state (blade has hit against something and flashes)
      case BLADE_STATE_CLASH:
        if (state_step == 0) {
          mem_blade(MEM_BLADE_BACKUP);      // backup blade state
          blade.color_state += 0x10;        // record we're currently in a CLASH state
          set_blade_color();                // set clash color
          set_blade_brightness(100);        // set blade brightness to 100%
          set_max_blade_brightness(100);    // set max blade brightness to 100%
          next_event_time = millis() + 40;  // end clash after 40ms
          blade.state++;                    // increment blade state counter
        }
        else if (millis() > next_event_time) {
          mem_blade(MEM_BLADE_RESTORE);     // restore blade state
          blade.state = BLADE_STATE_ON;
        }
        break;

      // the blade is igniting; all legacy hilts and crystal colors have the same power-on timing
      case BLADE_STATE_POWER_ON:
        if (millis() > next_event_time) {
          next_event_time = millis() + 85;        // delay this many milliseconds until next step
          switch (state_step) {
            case 0:
              set_blade_brightness(100);          // default blade brightness to 100%; dmode_handler() may change this later
              set_max_blade_brightness(0);        // set max brightness to 0; effectively turning the blade off
              blade_power_on();                   // enable the blade's LDO
              set_max_segment_brightness(0, 50);  // start turning segment 1 on
              break;

            case 1:
              set_max_segment_brightness(0, 100); // turning segment 1 on
              set_max_segment_brightness(1, 50);  // start turning segment 2 on
              break;

            case 2:
              set_max_segment_brightness(1, 100); // turning segment 2 on
              set_max_segment_brightness(2, 50);  // start turning segment 3 on
              break;

            case 3:
              set_max_segment_brightness(2, 100); // turning segment 3 on
              set_max_segment_brightness(3, 100); // turning segment 4 on
              blade.state = BLADE_STATE_ON;       // blade is fully on
              break;

            default:
              break;
          }
          blade.state++;
        }
        break;

      // blade is being extinguished
      //
      // different kyber crystals and legacy sabers begin their shutdown animation at different times after
      // the switch is turned off. step 0 in the power off animation is setting a delay before
      // power-off to align the animation timing with the stock blade.
      //
      // the stock blade also has different power-off animations (2 and 3-step fade-downs for segments)
      // this code also adds time to align this code's single power-off animation timing with the different
      // power-off animations of the stock blade
      case BLADE_STATE_POWER_OFF:
        if (millis() > next_event_time) {
          next_event_time = millis() + 85;  // default delay until next step in ms
          switch (state_step) {

            case 0: // Set Power Off Animation Delay

              // blade is in a legacy hilt
              if ((blade.color_state >> 4) == STOCK_BLADE_COLOR_TABLE_LEGACY) {
                switch (blade.color_state & 0x0F) {
                  case 0: // temple guard
                    blade.state++;      // skip a step to save 85ms and align timing of power off with stock blade performance
                    next_event_time = 0;
                    break;

                  case 5: // ahsoka (post clone wars)
                  case 6: // luke
                    next_event_time += 85;
                    break;

                  case 10: // baylan skoll/ shin hati
                    next_event_time += 140;
                    break;

                  case 9: // obi-wan, ben solo
                  case 2: // rey, rey reforge, ahsoka (clone wars)
                    next_event_time += 425;
                    break;

                  case 4: // ventress
                  case 7: // vader
                    next_event_time += 510;
                    break;

                  case 3: // mace windu
                  case 8: // maul
                    next_event_time += 595;
                    break;

                  case 1: // kylo ren
                    next_event_time += 680;
                    break;

                  default:
                    next_event_time = 0;
                    break;
                }

              // blade is in a Savi's Workshop hilt
              } else {
                switch (blade.color_state & 0x0F) {
                  case 2: // orange
                  case 3: // yellow
                    next_event_time = 0;
                    blade.state += 2;       // mimic stock blade, which skips two steps and goes right to shutting off segment 4
                    break;

                  case 5: // blue
                  case 6: // cyan
                    next_event_time += 85;  // extra delay to align end of power off with time a stock blade takes
              
                  case 4: // green
                    next_event_time += 85;  // cases 5 & 6 follow through to here, doubling their delay time
                    break;

                  case 7: // purple
                    next_event_time += 170; // extra delay to align end of power off with time a stock blade takes
              
                  case 0: // white
                    next_event_time += 255; // case 7 follows through to here
                    break;

                  case 1: // red
                  case 8: // dark purple
                    next_event_time += 765;
                    break;

                  default:
                    next_event_time = 0;
                    break;
                }
              }
              break;

            case 1:
              set_max_segment_brightness(3, 66);  // start shutdown of segment 4; this entire process will take ~425ms
              set_max_segment_brightness(3, 20);
              break;

            case 2:
              set_max_segment_brightness(3, 33);
              set_max_segment_brightness(2, 66);  // start shutdown of segment 3
              break;

            case 3:
              set_max_segment_brightness(3, 0);   // shut off segment 4
              set_max_segment_brightness(2, 33);
              set_max_segment_brightness(1, 66);  // start shutdown of segment 2
              break;

            case 4:
              set_max_segment_brightness(2, 0);   // shut off segment 3
              set_max_segment_brightness(1, 33);
              set_max_segment_brightness(0, 66);  // start shutdown of segment 1
              break;

            case 5:
              set_max_segment_brightness(1, 0);   // shut off segment 2
              set_max_segment_brightness(0, 33);
              break;

            case 6:
              set_max_segment_brightness(0, 0);   // shut off segment 1
              blade_power_off();                  // shut off LDO that powers RGB LEDs
              blade.state = BLADE_STATE_OFF;      // set blade state to off
              break;

            default:
              break;
          }
          if ((blade.state & 0xF0) != BLADE_STATE_OFF) {
            blade.state++;
          }
        }
        break;

      // command sent by Kylo Ren legacy lightsaber that causes the blade to flicker
      case BLADE_STATE_STOCK_FLICKER:

        // There are two flicker commands. 
        //   _FLICKER_1 = 0-50% brightness
        //   _FLICKER_2 = 53-100% brightness
        // 
        // Flicker is two-steps. 
        //   First step sets brightness for 40ms 
        //   Second step drops brightness and it stays there until next command           
        //
        // This is not a perfect replication of the stock blade behavior, but
        // it's close enough.

        // first flicker step
        if ((state_step & 1) == 0) {

          // _FLICKER_1
          if (state_step < 4) {
            set_blade_brightness((uint8_t)((float)(blade.color_state + 1) * 3.125));

          // _FLICKER_2
          } else {
            set_blade_brightness((uint8_t)((float)(blade.color_state + 17) * 3.125));
          }

          // increment blade state so code knows we're on step 2
          blade.state++;
          next_event_time = millis() + 40;

        // second flicker step
        } else if (millis() > next_event_time) {

          // _FLICKER_1
          if (state_step < 4) {
            if (blade.color_state < 5) {
              set_blade_brightness(0);
            } else {
              set_blade_brightness((uint8_t)((float)(blade.color_state - 5) * 3.125));
            }

          // _FLICKER_2
          } else {
            set_blade_brightness((uint8_t)((float)(blade.color_state + 10) * 3.125));
          }

          // reset blade (?)
          //blade.color_state = STOCK_BLADE_COLOR_RED;
          blade.state = BLADE_STATE_ON;
        }
        break;
    }
  }
}

void true_segment_brightness_handler(void) {
  uint8_t i;

  for(i=0;i<BLADE_SEGMENTS;i++) {
    true_segment_brightness[i] = (uint8_t)(((float)max_segment_brightness[i]/255) * segment_brightness[i]);
  }
}
