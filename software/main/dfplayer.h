 #ifndef DFPLAYER_H
#define DFPLAYER_H

#include "driver/uart.h"
#include <string.h>

// UART settings
#define UART_NUM UART_NUM_2
#define BAUD_RATE 9600

#define UART_BUFF_SIZE (1024 * 2)
#define UART_QUEUE_SIZE (20)

#define UART_CHUNK_SZ   128
#define TX_TIMEOUT      100
#define RX_TIMEOUT      1000

// #define CONN_NOTIFY_WAIT_MS 10
#define DFP_PACKET_LEN 10

// Protocol
#define DFP_START_BYTE  0x7E
#define DFP_STOP_BYTE   0xEF
#define DFP_LEN         0x06
#define DFP_VERSION     0xFF
#define DFP_FEEDBACK    0x00

// Command parameters
#define DFP_MAX_VOL         30
#define DFP_NO_EFFECT       0x0
#define DFP_ROCK            0x2
#define DFP_CLASSIC         0x04
#define DFP_BASE            0x05
#define DFP_REPEAT          0x0
#define DFP_REPEAT_FOLDER   0x01
#define DFP_REPEAT_ONCE     0x02
#define DFP_RANDOM          0x03
#define DFP_PLAY_U          0x0
#define DFP_PLAY_U          0x0
#define DFP_PLAY_TF         0x01
#define DFP_PLAY_AUX        0x02
#define DFP_PLAY_SLEEP      0x03
#define DFP_PLAY_FLASH      0x04
#define DFP_MAX_FOLDERS     10
#define DFP_MAX_VOL_GAIN    31

// Commands
typedef enum DFP_CMD {
    DFP_NEXT = 0x01,
    DFP_PREV = 0x02,
    DFP_TRACK = 0x03,
    DFP_INC_VOL = 0x04,
    DFP_DEC_VOL = 0x05,
    DFP_VOL = 0x06,
    DFP_EFFECT = 0x07,
    DFP_PLAY_MODE = 0x08,
    DFP_PLAY_SRC = 0x09,
    DFP_STANDBY = 0x0A,
    DFP_NORMAL = 0x0B,
    DFP_RST = 0x0C,
    DFP_PLAY = 0x0D,
    DFP_PAUSE = 0x0E,
    DFP_FOLDER = 0x0F,
    DFP_VOL_ADJ = 0x10,
    DFP_REP_PLAY = 0x11,
    DFP_REQ_PARAMS = 0x3F,
    DFP_REQ_RETX = 0x40,
    DFP_REQ_REPLY = 0x41,
    DFP_REQ_STATUS = 0x42,
    DFP_REQ_VOL = 0x43,
    DFP_REQ_EFFECT = 0x44,
    DFP_REQ_PLAY_MODE = 0x45,
    DFP_REQ_VERS = 0x46,
    DFP_REQ_FILES_CARD = 0x47,
    DFP_REQ_TRACK = 0x4B,
} DFP_CMD;

esp_err_t execute_command(DFP_CMD, uint16_t);
esp_err_t await_feedback();

#endif