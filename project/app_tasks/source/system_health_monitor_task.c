/**
 * @file system_health_monitor_task.c
 * @brief Source file for the system health monitor task.
 *
 * It does the following:
 * - Monitors the internal CPU temperature
 * - Monitors the watchdog timer and activates it only when the debugger is not attached
 *
 * @author Anirudh Singh
 * @date 12th October 2026
 */

#include <stdbool.h>

#include "FreeRTOSTasks.h"
#include "adc.h"
#include "error_handler_task.h"
#include "system_health_monitor_task.h"
#include "tim.h"
#include "uart.h"

/**********************************************
 ******* Private function definitions *********
 *********************************************/

static void vSystemHealthMonitorTask(void *param);

/**
 * Use the following function to read the analog value
 * for the internal temperature sensor. Then convert it
 * to Celsius.
 *
 * @return read_temp_celsius Temperature in Celsius
 */
static float raw_to_celsius(void);

/**
 * Task to get temperature values
 *
 * - Call a function that returns temperature in Celsius
 * - Check the resulting temperature against a threshold
 * - If its outside the threshold send an error message
 * - If its inside this value then let it run
 */
static void vSystemHealthMonitorTask(void *param) {
  (void)param;

  while (1) {
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0U) {
      float internal_temp_celsius = raw_to_celsius();
      internal_temp_celsius = raw_to_celsius();

      // Post the temperature on the COM port to be displayed on terminal
      UART_write_string("Internal CPU Temperature (°C) = ");
      UART_write_temp(internal_temp_celsius);
      UART_write_string(" °C\r\n");

      if ((internal_temp_celsius < MIN_INT_TEMP_THRESHOLD_CELSIUS) || (internal_temp_celsius > MAX_INT_TEMP_THRESHOLD_CELSIUS)) {
        vErrorHandlerSendMsg(EVT_SYSTEM_HEALTH_TEMP_EXCEEDED);
      }
    }
  }
}

static float raw_to_celsius(void) {
  float temp_celsius = 0.0F;                  // Variable to store the ADC converted temperature value
  float V_SENSE = 0.0F;                       // Convert analog value to a voltage
  uint16_t temp_raw = adc_get_int_temp_raw(); // Get the raw analog value from adc driver file

  V_SENSE = ((float)temp_raw / 4095.0F) * V_REF_PLUS; // ADC is 12-bit (2^12 = 4096)
  temp_celsius = ((V_SENSE - V_25) / AVG_SLOPE_SENSE_CURVE) + 25;

  return temp_celsius;
}

void vSystemHealthMonitorTaskStart(void) {
  BaseType_t xReturned;
  TaskHandle_t xSystemHealthHandle = NULL;

  // Initialise and start the ADC clock
  adc_init();

  // Create the health monitor task
  xReturned = xTaskCreate(vSystemHealthMonitorTask,
                          "System Health Monitor Task",
                          SYSTEM_HEALTH_MONITOR_TASK_STACK_SIZE,
                          NULL,
                          SYSTEM_HEALTH_MONITOR_TASK_STACK_PRIORITY,
                          &xSystemHealthHandle);

  // Save the handler to be used later for ADC ISR notifications
  adc_get_system_health_handle(xSystemHealthHandle);

  // Start the timer clock and ADC conversion
  tim_2_init();

  if (xReturned != pdPASS) {
    vErrorHandlerSendMsg(EVT_SYSTEM_HEALTH_TASK_CREATE_FAILED); // Delete the system health monitor task if it can't be created
  }
}
