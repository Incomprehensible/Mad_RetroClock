#include "dfplayer.h"
#include "pins.h"
#include "esp_log.h"

static const char *TAG = "UART";

static esp_err_t send_ir_data(const uint8_t* data, size_t len)
{
    ESP_LOGI(TAG, "Sending %d bytes: ", len);
    for (int i=0; i < len; ++i)
    {
        printf("%02x ", (int)data[i]);
    }
    printf("\n\n");
    fflush(NULL);
    uart_write_bytes(UART_NUM, data, len);
    return uart_wait_tx_done(UART_NUM, TX_TIMEOUT);
}

static esp_err_t receive_ir_data()
{
    const uart_port_t uart_num = UART_NUM;
    uint8_t data[UART_BUFF_SIZE] = {};
    int length = 0;

    ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, (size_t*)&length));
    if (length != 0)
    {
        length = uart_read_bytes(uart_num, data, length, RX_TIMEOUT);
        uart_flush(uart_num);
        if (length < 0) {
            ESP_LOGE(TAG, "Error receiving from UART.");
            return -1;
        }
        if (length > 0)
        {
            ESP_LOGI(TAG, "Received %d bytes: ", length);
            for (int i=0; i < length; ++i)
            {
                printf("%02x ", (int)data[i]);
            }
            printf("\n\n");
            fflush(NULL);
            //parse_ir_data(data, length);
        }
    }
    else
        ESP_LOGE(TAG, "No input data from UART.");
    return ESP_OK;
}

/*
DFPlayer data frame format:
0      1    2    3    4    5   6   7     8     9-byte
START, VER, LEN, CMD, ACK, DH, DL, SUMH, SUML, END
       -------- checksum --------
*/
static void calculate_checksum(uint8_t* databuf)
{
    int16_t checksum = 0;
    
    checksum = checksum - databuf[1] - databuf[2] - databuf[3] - databuf[4] - databuf[5] - databuf[6];
    databuf[7] = checksum >> 8;
    databuf[8] = checksum;
}

// void send_packet(uint8_t* databuf)
// {
//     databuf[0] = DFP_START_BYTE;
//     databuf[1] = DFP_VERSION;
//     databuf[2] = DFP_LEN;
//     databuf[3] = cmd;
//     databuf[4] = DFP_FEEDBACK;
//     switch (cmd):
//     {
//         case 
//     }



//     calculate_checksum(databuf);
//     databuf[9] = DFP_STOP_BYTE;
// }

esp_err_t execute_command(DFP_CMD cmd, uint16_t params)
{
    uint8_t databuf[DFP_PACKET_LEN] = {
        DFP_START_BYTE,
        DFP_VERSION,
        DFP_LEN,
        cmd,
        DFP_FEEDBACK,
        params >> 8,
        params & 0x0FF,
        0,
        0,
        DFP_STOP_BYTE,
    };

    calculate_checksum(&databuf[0]);
    return send_ir_data(&databuf[0], DFP_PACKET_LEN);
}

esp_err_t await_feedback()
{
    return receive_ir_data();
}
