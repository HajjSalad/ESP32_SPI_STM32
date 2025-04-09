/*
//  SPI Master Full Duplex
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define MOSI    23
#define MISO    19
#define SCK     18
#define SS      5

spi_device_handle_t handle;

void spi_init ()
{
    // Set the communication parameters
    const spi_bus_config_t spi_config = {
        .miso_io_num = MISO,
        .mosi_io_num = MOSI,
        .sclk_io_num = SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };

    // Set SPI device interface
    const spi_device_interface_config_t spi_device_config = {
        .clock_speed_hz = 1*1000*1000,  // Set the clock speed (1 MHz)
        .duty_cycle_pos = 128,          // Set the duty cycle, 128 = 50%
        .mode           = 0,            // SPI mode (0: CPOL = 0, CPHA = 0)
        .spics_io_num   = SS,           // Slave Select
        .queue_size     = 1,            // Number of transactions to queue
    };

    spi_bus_initialize(SPI2_HOST, &spi_config, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &spi_device_config, &handle);
}

static void spi_task()
{
    uint8_t tx_data[] = "Message from ESP32 Master";  // 25 bytes
    uint8_t rx_data[26];

    while (1) {
        spi_transaction_t t;
        memset(&t, 0, sizeof(t));

        t.length = 8 * sizeof(tx_data);  // 25 bytes * 8 bits
        t.tx_buffer = tx_data;
        t.rx_buffer = rx_data;

        esp_err_t ret = spi_device_transmit(handle, &t);
        if (ret == ESP_OK) {
            printf("Received: %.*s\n", sizeof(rx_data), rx_data);  // Print as string
        } else {
            printf("SPI Transmission Failed\n");
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}

void app_main(void)
{
    printf("Program Start\n");

    spi_init();     // Initialize spi

    // Create a task for spi to send and receive data
    xTaskCreate(spi_task, "spi_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
}
