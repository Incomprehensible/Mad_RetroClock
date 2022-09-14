 #ifndef DFPLAYER_H
#define DFPLAYER_H

#include "driver/uart.h"
#include <cstring>

#define UART_NUM UART_NUM_2

#define UART_BUFF_SIZE (1024 * 2)
#define UART_QUEUE_SIZE (20)

#define UART_CHUNK_SZ   128
#define TX_TIMEOUT      100
#define RX_TIMEOUT      500

#define CONN_NOTIFY_WAIT_MS 10

#endif