#include "dfplayer.h"
#include "pins.h"

static const char *TAG = "UART";

esp_err_t send_ir_data(const char* data, size_t len)
{
    uart_write_bytes(UART_NUM, data, len);
    //     //uart_write_bytes_with_break(uart_num, "test break\n",strlen("test break\n"), 100);
    esp_err_t status = uart_wait_tx_done(UART_NUM, TX_TIMEOUT);
    if (status != ESP_OK)
        setts.setts.ir_sensor_err = SENS_UART_SEND_ERR;
    return status;
}

void receive_ir_data(bool debug)
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
            setts.setts.ir_sensor_err = SENS_UART_RECEIVE_ERR;
            ESP_LOGE(TAG, "Error receiving from UART.");
        }
        if (length > 0)
        {
            if (debug) {
                ESP_LOGI(TAG, "Received %d bytes from IR sensor: ", length);
                for (const auto c: data)
                {
                    printf("%02x ", (int)c);
                }
                printf("\n\n");
                fflush(NULL);
            }
            parse_ir_data(data, length);
        }
    }
}