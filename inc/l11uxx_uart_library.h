/*
 * l11uxx_uart_library.h
 *
 *  Created on: 19.04.2017
 *      Author: Denry
 */

#ifndef INC_L11UXX_UART_LIBRARY_H_
#define INC_L11UXX_UART_LIBRARY_H_


#define L11UXX_UART_RX_BUFFER_LEN 200

volatile char l11uxx_uart_rx_buffer[L11UXX_UART_RX_BUFFER_LEN];
volatile int l11uxx_uart_rx_buffer_current_index = 0;

volatile char rxBusy = 0;


#endif /* INC_L11UXX_UART_LIBRARY_H_ */
