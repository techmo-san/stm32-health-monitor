#include "gpio_defs.h"

GPIO_pin_config_s USER_LED = {
    .port = GPIOA,
    .pin = GPIO_PIN_5,
    .pin_mode = GPIO_MODE_OUTPUT,
    .pin_output_type = GPIO_OUTPUT_PUSH_PULL,
    .pin_output_speed = GPIO_OUTPUT_SPEED_LOW,
    .pin_pull = GPIO_PULL_UP,
    .pin_alt_fn = GPIO_AF_NONE,
};

GPIO_pin_config_s PA2 = {
    .port = GPIOA,
    .pin = GPIO_PIN_2,
    .pin_mode = GPIO_MODE_AF,
    .pin_output_type = GPIO_OUTPUT_PUSH_PULL,
    .pin_output_speed = GPIO_OUTPUT_SPEED_LOW,
    .pin_pull = GPIO_PULL_UP,
    .pin_alt_fn = GPIO_AF_7,
};
