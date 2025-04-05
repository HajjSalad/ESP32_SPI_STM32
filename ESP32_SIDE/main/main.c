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
    };

    // Set SPI device interface
    const spi_device_interface_config_t spi_device_config = {
        .clock_speed_hz = 1000000,      // Set the clock speed (1 MHz)
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
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    char sendBuffer[30] = {0};
    char receiveBuffer[30] = {0};

    while(1)
    {
        snprintf(sendBuffer, sizeof(sendBuffer), "Hi I am ESP32");
        t.length = sizeof(sendBuffer) * 8;
        t.tx_buffer = sendBuffer;
        t.rx_buffer = receiveBuffer;
        spi_device_transmit(handle, &t);
        printf("Transmitted: %s\n", sendBuffer);
        printf("Received: %s\n", receiveBuffer);
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
