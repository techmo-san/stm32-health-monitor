/**
 * @file gpio_defs.h 
 * @brief Header file to configure GPIO pins
 *
 * @author Anirudh Singh
 * @date 6th October 2026
 */

#ifndef INC_GPIO_DEFS_H
#define INC_GPIO_DEFS_H

// Include this file containing the enumerations for pin configuration and different functions
#include "gpio.h"

extern GPIO_pin_config_s USER_LED; // Used for error reporting
extern GPIO_pin_config_s PA2;      // Used for printing values to the terminal

#endif // !INC_GPIO_DEFS_H
