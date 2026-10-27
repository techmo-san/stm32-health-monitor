/**
 * @file uart.h 
 * @brief Header file to configure UART
 *
 * @author Anirudh Singh
 * @date 26th October 2026
 */

#ifndef INC_UART_H
#define INC_UART_H

#define UART_BAUD_RATE (115200)

/**
 * Function to initialise the UART driver
 */
void UART_init(void);

void UART_write_byte(uint8_t byte);
void UART_write_string(char *text);
void UART_write_temp(float temp);

#endif // !INC_UART_H
