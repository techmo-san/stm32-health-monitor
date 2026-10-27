/**
 * @file gpio.c
 * @brief Source file to configure GPIO pins
 *
 * @author Anirudh Singh
 * @date 6th October 2026
 */

#include "gpio.h"
#include "gpio_defs.h"
#include "stm32f4xx.h"

/******************************************
 *********** Private functions ************
 *****************************************/

/**
 * Sets the GPIO pin mode (i/p, o/p, alternate function or analog)
 *
 * @param gpio Pointer to the GPIO_pin_config_s struct object defined in file 'gpio_defs.c'
 */
static void GPIO_set_pin_mode(GPIO_pin_config_s *gpio) {
  // Clear the 2 bit field by using an inverted mask
  gpio->port->MODER &= ~(3U << (2U * gpio->pin));

  // Set the pin mode except for input pins
  if (gpio->pin_mode != GPIO_MODE_INPUT) {
    gpio->port->MODER |= gpio->pin_mode << (2U * gpio->pin);
  }
}

/**
 * Sets the type of output GPIO pin (PUSH_PULL or OPEN_DRAIN or NONE).
 * This type determines whether a line can be pulled HIGH or LOW (PUSH_PULL).
 * Or in OPEN_DRAIN's scenario the line can only be pulled LOW allowing the line to be pulled HIGH by using an external source.
 *
 * @param gpio Pointer to the GPIO_pin_config_s struct object defined in file 'gpio_defs.c'
 */
static void GPIO_set_output_type(GPIO_pin_config_s *gpio) {
  // Clear the bit for a push-pull type
  gpio->port->OTYPER &= ~(1U << gpio->pin);

  // Set the OPEN_DRAIN output type
  if (gpio->pin_output_type == GPIO_OUTPUT_OPEN_DRAIN) {
    gpio->port->OTYPER |= gpio->pin_output_type << gpio->pin;
  }
}

/**
 * Sets the speed of the output pin (LOW < MEDIUM < FAST < HIGH)
 * This affects the rate at which signals can rise or fall.
 * Hence, being particularly useful for configuring high frequency peripherals.
 *
 * @param gpio Pointer to the GPIO_pin_config_s struct object defined in file 'gpio_defs.c'
 */
static void GPIO_set_output_speed(GPIO_pin_config_s *gpio) {
  // Clear the bit field by setting it to low speed
  gpio->port->OSPEEDR &= ~(3U << (2U * gpio->pin));

  // Check if the pin is set to output mode and then set the bitfield
  if ((gpio->pin_mode == GPIO_MODE_OUTPUT) || (gpio->pin_mode == GPIO_MODE_AF)) {
    gpio->port->OSPEEDR |= (gpio->pin_output_speed << (2U * gpio->pin));
  }
}

/**
 * Sets the push pull resistor type for a specific GPIO pin.
 * This determines whether that pin used an internal pull-up, pull-down resistor or none.
 * This is useful for a stable logic from a pin when it's not floating.
 *
 * @param gpio Pointer to the GPIO_pin_config_s struct object defined in file 'gpio_defs.c'
 */
static void GPIO_set_pull_resistor(GPIO_pin_config_s *gpio) {
  // Clear the bit field to set no pull-up or pull-down resistor
  gpio->port->PUPDR &= ~(3U << (2 * gpio->pin));

  // Set the pull-up or pull-down resistor
  if (gpio->pin_pull != GPIO_PULL_NONE) {
    gpio->port->PUPDR |= (gpio->pin_pull << (2 * gpio->pin));
  }
}

/**
 * Sets the GPIO alternalte function for the specified port and pin.
 * This includes setting up the pin for different peripherals (I2C, SPI, UART etc.)
 *
 * @param gpio Pointer to the GPIO_pin_config_s struct object defined in file 'gpio_defs.c'
 */
static void GPIO_set_alt_fn(GPIO_pin_config_s *gpio) {
  if (gpio->pin <= GPIO_PIN_7) {
    // Clear the bit field for the given pin inside AFRL register
    gpio->port->AFR[0] &= ~(15U << (4 * gpio->pin));

    // Set the alternate function
    if ((gpio->pin_mode == GPIO_MODE_AF) && (gpio->pin_alt_fn != GPIO_AF_NONE)) {
      gpio->port->AFR[0] |= (gpio->pin_alt_fn << (4 * gpio->pin));
    }
  } else {
    // Clear the bit field and subtract 8 as bit field for pin 8 is [3:0] for the AFRH register
    gpio->port->AFR[1] &= ~(15U << (4 * (gpio->pin - 8U)));

    // Set the alternate function
    if ((gpio->pin_mode == GPIO_MODE_AF) && (gpio->pin_alt_fn != GPIO_AF_NONE)) {
      gpio->port->AFR[1] |= (gpio->pin_alt_fn << (4 * (gpio->pin - 8U)));
    }
  }
}

/**
 * Combines all the functions mentioned above for easier pin configuration
 *
 * @param gpio Pointer to the GPIO_pin_config_s struct object defined in file 'gpio_defs.c'
 */
static void GPIO_set_config(GPIO_pin_config_s *gpio) {
  GPIO_set_pin_mode(gpio);
  GPIO_set_output_type(gpio);
  GPIO_set_output_speed(gpio);
  GPIO_set_pull_resistor(gpio);
  GPIO_set_alt_fn(gpio);
}

/******************************************
 *********** Public functions *************
 *****************************************/

void GPIO_init(void) {
  // Initialise the Port A clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

  // Configure the USER_LED object inside 'gpio_defs.c'
  GPIO_set_config(&USER_LED);

  // Configure the PIN2 in port A for UART USB transmission
  GPIO_set_config(&PA2);
}

void GPIO_set_pin(GPIO_pin_config_s *gpio) {
  gpio->port->BSRR = (1U << gpio->pin); // Set the bit inside the BSRR BSy bit field
}

void GPIO_reset_pin(GPIO_pin_config_s *gpio) {
  gpio->port->BSRR = (1U << (gpio->pin + 16U)); // Set the bit inside the BSRR BRy bit field
}

gpio_pin_state_e GPIO_read_pin(GPIO_pin_config_s *gpio) {
  gpio_pin_state_e pin_state;
  pin_state = (gpio->port->IDR & (1U << gpio->pin)); // Mask the register
  if (pin_state != 0) {                              // If the value after the masking is non-zero then that means the bit is set
    return GPIO_PIN_SET;                             // Means pin state is HIGH
  }
  return GPIO_PIN_RESET; // Means pin state is LOW
}

void GPIO_toggle_pin(GPIO_pin_config_s *gpio) {
  gpio_pin_state_e pin_state = GPIO_read_pin(gpio);
  if (pin_state == GPIO_PIN_SET) { // If pin state is HIGH, turn it to LOW
    GPIO_reset_pin(gpio);
  } else {
    GPIO_set_pin(gpio); // vice versa
  }
}
