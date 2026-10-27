/**
 * @file adc.c
 * @brief Setup the ADC converter to interface with the internal temperature sensor
 *
 * @author Anirudh Singh
 * @date 12th October 2026
 */

#include <stdbool.h>

#include "FreeRTOSTasks.h"
#include "adc.h"
#include "error_handler_task.h"
#include "interrupts.h"
#include "stm32f4xx.h"

/*****************************************
 ****** Private function definitions *****
 *****************************************/
static void adc_register_set_CCR(void);
static void adc_1_register_set_CR1(void);
static void adc_1_register_set_CR2(void);
static void adc_1_register_set_SMPR1(void);
static void adc_1_register_set_SQR3(void);

/*****************************************
 ********* Temperature Handler ***********
 *****************************************/
static volatile uint16_t temp_raw = 0;
static TaskHandle_t xSystemHealthHandle = NULL; // Save the task handler to be used later for ISR notifications

void adc_init(void) {
  /**
   * Enable ADC1 clock
   *
   * ADC1EN bit [8]
   */
  RCC->APB2ENR |= (1U << 8);

  adc_1_register_set_CR1();
  adc_1_register_set_CR2();
  adc_register_set_CCR();
  adc_1_register_set_SMPR1();
  adc_1_register_set_SQR3();

  /**
   * Enable the NVIC ADC Interrupt Line.
   * Without it CPU will ignore any interrupt requests from the ADC.
   *
   * The interrupt chain needs two bits fields to be set:
   * 1. At the peripheral level: EOCIE
   * 2. NVIC Enable Interrupt Request: ADC_IRQn is the enumeration with
   * value 18 inside 'stm32f4xx.h'
   */
  NVIC_SetPriority(ADC_IRQn, 5);
  NVIC_EnableIRQ(ADC_IRQn);
}

/**
 * Setup the ADC Common Control Register (CCR)
 *
 * - Enable the temperature sensor channel
 * - Disable the VABTE
 * - Set the prescaler for the ADC clock
 */
static void adc_register_set_CCR(void) {
  ADC->CCR = 0; // Reset the ADC1 common control register

  /**
   * TSVREFE bit [23]
   */
  ADC->CCR &= ~(1U << 23); // Reset : VREFINT and Temperature Sensor Channel

  // Initialise the following bits because they require startup time
  // Set the VREFINT and Temperature Sensor channel bit
  ADC->CCR |= 1U << 23;

  /**
   *  VBATE bit [22] : VBAT enable
   *
   * Disable the VBATE to perform temperature measurement,
   * as VBAT and temperature sensor are on the same channel
   */
  ADC->CCR &= ~(1U << 22);

  /**
   * ADCPRE [17:16] Bit field -> ADC Prescaler
   * Set the prescaler for ADC1 clock
   *
   * 0U -> 2
   * 1U -> 4
   * 2U -> 6
   * 3U -> 8
   */
  ADC->CCR |= (1U << 16);
}

/**
 * This function sets the required bits for the ADC_CR1 register.
 *
 * - Clear the register and reset it
 * - Enable the EOC (End Of Conversion) Interrupt Enable bit
 * - This allows an interrupt function to be executed once conversion is complete.
 */
static void adc_1_register_set_CR1(void) {
  /**
   * Reset the ADC1 control register 1
   */
  ADC1->CR1 = 0;

  /**
   * EOCIE bit (EOC Interrupt Enable)
   * - Enable this bit so that EOC interrupt is enabled.
   * - 0U -> Disable
   * - 1U -> Enable
   */
  ADC1->CR1 |= (1U << 5);
}

static void adc_1_register_set_CR2(void) {
  /**
   * Reset the ADC1 control register 2
   */
  ADC1->CR2 = 0;

  /**
   * ADON bit [0]
   * Enable the A/D Converter as it has a startup time
   */
  ADC1->CR2 |= (1U << 0);

  /**
   * EOCS bit (End Of Conversion Selection)
   * - Enable this bit so that EOC bit is set when a conversion is completed.
   * - 0U -> Disable
   * - 1U -> Enable
   */
  ADC1->CR2 |= (1U << 10);

  /**
   * EXTEN Bit [29:28] -> EXternal Trigger ENable for regular channels
   * Enable trigger detection on the rising edge
   */
  ADC1->CR2 |= (1U << 28);

  /**
   * EXTSEL Bit [27:24]: EXTernal Event SELect for regular group
   * These bits select the external event used to trigger the start of conversion of a regular group.
   *
   * In our case we would like to use the rising edge of the event to trigger a conversion.
   * TRGO means trigger output. Therefore, each time TIMER2 reaches end of its count and starts
   * a new period, it sends a trigger to ADC1 to begin the conversion.
   */
  ADC1->CR2 |= (6U << 24); // Trigger TIMER2 TRGO event
}

/**
 * Use the following function to set the SMPR1 (Sampling) register.
 */
static void adc_1_register_set_SMPR1(void) {
  /**
   * Set the sampling rate for the temperature sensor
   */
  ADC1->SMPR1 &= ~(7U << 24); // Reset the register by masking enumeration with value ~(111)

  /**
   * - APB2 clock frequency at 45 MHz
   * - Temperature sensor sampling time = 10 us
   * - ADC1 Clock  = APB2 / Prescaler (2, 4, 6, 8)
   * - ADC1 Clock Period at prescaler 2 (22.5 MHz) = 44.444 ns
   * - ADC1 Clock Period at prescaler 4 (11.25 MHz) = 88.888 ns
   * - Sampling Rate = (10 * 10^3 (ns) / ADC1 Clock Period (ns)) = 225 cycles or 112.5 cycles
   *
   * Choosing the sampling rate of 144 cycles at 11.25 MHz ADC1 clock frequency and shifting to the left by 24
   * 
   * 0U -> 3 cycles
   * 1U -> 15 cycles
   * .
   * .
   * 6U -> 144 cycles
   * 7U -> 480 cycles
   */
  ADC1->SMPR1 |= (6U << 24);
}

/**
 * The following function allows the ADC converter to know which analog channel it needs
 * to pay attention to for conversion.
 *
 * SQ1 [0:4] -> 5 bit field in SQR3
 *
 * Temperature sensor is on Channel 18 (i.e. 18U) which is then
 * set to the register SQR3 from bit 0 to 4.
 */
static void adc_1_register_set_SQR3(void) {
  ADC1->SQR3 = 0; // Reset the register
  ADC1->SQR3 |= (ADC_CH_18 << 0);
}

/********************************************************
 *********** Temperature Handling Functions *************
 *******************************************************/

/**
 * ISR for ADC interrupts defined inside 'interrupts.h'
 * This function should be minimal and flag handling should
 * be resolved by the error handler tasks.
 */
void ADC_IRQ(void) {
  BaseType_t woken = pdFALSE;

  /**
   * Compare the ADC status register with a bitmask of value 2
   * EOC is the 2nd bit in the ADC_SR register
   */
  if (ADC1->SR & (1U << 1)) {
    temp_raw = (uint16_t)(ADC1->DR & 0xFFFFU); // Mask the register with 16 bits of 1

    // Clear the EOC flag in ADC_SR register
    ADC1->SR &= ~(1U << 1);

    if (xSystemHealthHandle != NULL) {
      vTaskNotifyGiveFromISR(xSystemHealthHandle, &woken);
    }
  }

  /**
   * Check if the data is lost by checking the
   * overrun flag. This bit OVR is set by the hardware
   * if data is lost.
   */
  if (ADC1->SR & (1U << 5)) {
    ADC1->SR &= ~(1U << 5); // Clear the bit if overrun has occurred.
    vErrorHandlerSendMsgFromISR(EVT_SENSOR_READ_FAIL);
  }

  // Request a context switch
  portYIELD_FROM_ISR(woken);
}

void adc_get_system_health_handle(TaskHandle_t handle) {
  xSystemHealthHandle = handle; // Save the task handler inside the static variable
}

uint16_t adc_get_int_temp_raw(void) {
  return temp_raw;
}
