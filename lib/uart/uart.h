#ifndef __UART_H__
#define __UART_H__

#include <HardwareSerial.h>

/**
 * @brief Function to initialize the UART
 */
void uartInit();

/**
 * @brief Function to send a message to the UART
 * 
 * @param message - Message to send
 */
void sendToUart(String message);

/**
 * @brief Function to send a message to the UART
 * 
 * @param message - Message to send
 */
void sendToUart(char *message);

/**
 * @brief Function to handle the data received from the UART
 */
void uartHandler();

#endif // __UART_H__
