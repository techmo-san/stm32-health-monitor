/**
 * @file uart.c
 * @brief Source file to configure UART
 *
 * @author Anirudh Singh
 * @date 26th October 2026
 */
#include <stdint.h>

#include "rcc.h"
#include "stm32f4xx.h"
#include "uart.h"

/*******************************************
 ******* Private function definitions ******
 ******************************************/
static void UART_cr1_config(void);
static void UART_cr2_config(void);
static void UART_baud_rate_config(void);
static void UART_disable(void);
static void UART_enable(void);

void UART_init(void) {
  /**
   * Enable the USART2 clock in the APB1 bus
   * USART22EN bit is 17th bit in the register
   */
  RCC->APB1ENR |= (1U << 17);

  UART_disable(); // UART disable
  UART_cr1_config();
  UART_cr2_config();
  UART_baud_rate_config();
  UART_enable(); // UART enable
}

static void UART_cr1_config(void) {
  /**
   * OVER8 bit 15: Oversampling by 16
   * 0: oversampling by 16
   * 1: oversampling by 8
   */
  USART2->CR1 &= ~(1U << 15);

  /**
   * M bit 12: Word length
   * 0: 1 Start bit, 8 Data bits, n Stop bit
   * 1: 1 Start bit, 9 Data bits, n Stop bit
   */
  USART2->CR1 &= ~(1U << 12);

  /**
   * TE bit 3: Transmitter enable
   * 0: Transmitter is disabled
   * 1: Transmitter is enabled
   */
  USART2->CR1 |= (1U << 3);

  /**
   * RE bit 2: Receiver disable
   * 0: Receiver is disabled
   * 1: Receiver is enabled and begins searching for a start bit
   */
  USART2->CR1 &= ~(1U << 2);

  /**
   * PCE bit 10: Parity control disable
   * 0: Parity control disabled
   * 1: Parity control enabled
   */
  USART2->CR1 &= ~(1U << 10);
}

static void UART_cr2_config(void) {
  /**
   * STOP: Program number of stop bits
   * 00: 1 Stop bit
   * 01: 0.5 Stop bit
   * 10: 2 Stop bits
   * 11: 1.5 Stop bit
   */
  USART2->CR2 &= ~(3U << 12);
}

static void UART_baud_rate_config(void) {
  uint32_t apb1_clk_speed = rcc_get_pclk1_freq();
  uint8_t over8_val = 0;

  // If the OVER8 bit is enabled
  if (USART2->CR1 & (1U << 15)) {
    over8_val = 1;
  }

  uint8_t fraction_steps = (8U * (2U - over8_val));
  float usartdiv = (float)apb1_clk_speed / (float)(fraction_steps * UART_BAUD_RATE);
  uint32_t mantissa = (uint32_t)usartdiv;
  float fraction = usartdiv - (float)mantissa;

  uint8_t fraction_field = (uint8_t)(fraction * (float)fraction_steps + 0.5F);

  // If the round off fraction comes to 16 or 8 then add 1 to the mantissa
  if (fraction_field == fraction_steps) {
    mantissa++;
    fraction_field = 0U;
  }

  /**
   * Calculating USART2DIV for the baud rate:
   * - Baud rate = 115200
   * - Oversampling fraction OVER8 = 0 (Hence, oversampling by 16)
   * - USART2DIV = (f_ck) / (8 * (2 - OVER8) * Baud Rate) = 24.4140625
   * - Mantissa (whole number part) = 24
   * - Fraction = (0.4140625 * 16) = 6.625 => ~7
   */
  USART2->BRR &= ~(0xFFFF);             // Reset and clear register
  USART2->BRR |= (mantissa << 4);       // DIV_Mantissa [15:4]
  USART2->BRR |= (fraction_field << 0); // DIV_Fraction [3:0]
}

static void UART_disable(void) {
  /**
   * UE bit 13: Disable USART2
   * 0: USART2 prescaler and outputs disabled
   * 1: USART2 enabled
   */
  USART2->CR1 &= ~(1U << 13);
}

static void UART_enable(void) {
  /**
   * UE bit 13: Enable USART2
   * 0: USART2 prescaler and outputs disabled
   * 1: USART2 enabled
   */
  USART2->CR1 |= (1U << 13);
}

void UART_write_byte(uint8_t byte) {
  while (!(USART2->SR & (1U << 7))) {
    // wait;
  }

  USART2->DR |= byte << 0;
}

void UART_write_string(char *text) {
  while (*text != '\0') {
    UART_write_byte((uint8_t)*text);
    text++;
  }
}

void UART_write_temp(float temp) {
  uint8_t count = 0;
  char mag_digits[10];
  char frac_digits[10];

  // If temperature is less than zero
  if (temp < 0) {
    UART_write_byte('-');
    temp = -temp;
  }

  uint32_t magnitude = (uint32_t)temp;
  uint32_t fraction = (uint32_t)((temp - magnitude) * 1000.0F); // For 3 digit precision

  // Extract the digits in reverse order
  while (magnitude != 0) {
    mag_digits[count] = ('0' + (char)(magnitude % 10)); // Extract the right most digit
    count++;                                            // Increase the count to save the next digit
    magnitude = magnitude / 10;                         // Extract number for next digit
  }

  // Print the digits
  while (count > 0U) {
    UART_write_byte((uint8_t)mag_digits[--count]);
  }

  UART_write_byte('.');

  count = 0; // Reset counter

  // Extract the digits in reverse order
  while (fraction != 0) {
    frac_digits[count] = ('0' + (char)(fraction % 10)); // Extract the right most digit
    count++;                                            // Increase the count to save the next digit
    fraction = fraction / 10;                           // Extract number for next digit
  }

  // Print the fraction digits
  while (count > 0U) {
    UART_write_byte((uint8_t)frac_digits[--count]);
  }
}
