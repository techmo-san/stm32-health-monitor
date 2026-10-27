/**
 * Application entry point
 * Pure CMSIS + FreeRTOS (no HAL)
 */
#include <stdint.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOSTasks.h"
#include "error.h"
#include "error_handler_task.h"
#include "gpio.h"
#include "gpio_defs.h"
#include "rcc.h"
#include "stm32f4xx.h"
#include "system_health_monitor_task.h"
#include "uart.h"

static void vStartup(void);

int main(void) {
  // Variables to confirm clock speeds
  uint32_t system_clock = 0;
  uint32_t hclk = 0;
  uint32_t apb1_clock = 0;
  uint32_t apb2_clock = 0;

  // Initial RCC configuration
  rcc_init();

  // Get the system and peripheral clock speeds
  system_clock = rcc_get_sysclk_freq();
  hclk = rcc_get_hclk_freq();
  apb1_clock = rcc_get_pclk1_freq();
  apb2_clock = rcc_get_pclk2_freq();

  (void)system_clock;
  (void)hclk;
  (void)apb1_clock;
  (void)apb2_clock;

  // Update the 'SystemCoreClock' variable required by FreeRTOS
  SystemCoreClockUpdate();

  // Initialise the GPIO pins for different peripherals and USER_LED
  GPIO_init();

  // Initialise the UART transmission
  UART_init();

  // Start the error task handler
  vErrorHandlerTaskStart();

  // Start the system health montor task
  vSystemHealthMonitorTaskStart();

  // Start the task
  vStartup();

  // Start the FreeRTOS Scheduler (Has the HIGHEST priority)
  vTaskStartScheduler();
}

/**
 * Creates the startup task used for FreeRTOS initialisations on startup
 */
static void vStartupTask(void *param) {
  (void)param;

  // vErrorHandlerSendMsg(EVT_SYSTEM_HEALTH_TEMP_EXCEEDED);

  // Delete startup task after creating other tasks
  vTaskDelete(NULL);
}

/**
 * Create startup task
 */
static void vStartup(void) {
  configASSERT(pdPASS == xTaskCreate(vStartupTask, "Startup Task", STARTUP_TASK_STACK_SIZE, NULL, STARTUP_TASK_PRIORITY, NULL));
}
