/**
 * @file     bootram_port.c
 * @author   BSP Team
 * @brief    This file provides bootram port function.
 * @version  0.0.0.1
 * @date     2020-12-29
 *
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */

/* Includes ------------------------------------------------------------------*/

#include "ramcode_port.h"
#include "serial/serial.h"
#include "hal/flash.h"
#include "hal/hal_syscon.h"
#include "hal/hal_sleep.h"

#include "hal/hal_gpio.h"
#include "hal/hal_uart.h"
#include "hal/flash.h"
#include "hal/hal_dma.h"
#include "hal/hal_efuse.h"
/*****************************************  variables *********************************************/

Serial_t m_ConsoleSerial;

// buffer for sector alignment.

#define UART_RX_BUF_LEN (1024*(16*2+2))

//static uint8_t uart_rx_buf[UART_RX_BUF_LEN];
static uint8_t temp_4k_buffer[SIZE_4KB] = {0};

volatile uint8_t  uart_rx_over_flag = 0;
volatile uint32_t cur_pos           = 0;

/*****************************************  functions *********************************************/
int bootram_flash_uid(uint8_t *uid)
{
    hal_flash_read_unique_id(uid);
    return 0;
}
int bootram_flash_info(void)
{
    return FLASH_ReadID();
}

int bootram_flash_read(uint32_t offset, uint32_t len, void* buf)
{
    FLASH_Read(offset, len, buf);
    return 0;
}

int bootram_flash_write(uint32_t offset, uint32_t len, const void* buf)
{
    FLASH_Program(offset, len, (uint8_t*)buf);
    return 0;
}

int bootram_flash_erase(uint32_t offset, uint32_t len)
{
    /* Align offset down to 4KB */
    uint32_t base = (offset / SIZE_4KB) * SIZE_4KB;
    uint32_t head = offset - base;

    /* Align erase length up to 4KB */
    uint32_t erase_length =
        ((head + len + SIZE_4KB - 1) / SIZE_4KB) * SIZE_4KB;

    if (head == 0) {  // 4KB aligned.
        FLASH_Erase(base, erase_length);
    }
    else {  // NOT 4KB aligned.
        /* Read first sector */
        memset(temp_4k_buffer, 0, SIZE_4KB);
        FLASH_Read(base, SIZE_4KB, temp_4k_buffer);

        /* Erase from offset to end of first sector */
        memset(temp_4k_buffer + head, 0xFF, SIZE_4KB - head);

        /* Erase aligned region */
        FLASH_Erase(base, erase_length);

        /* Restore first sector */
        FLASH_Program(base, SIZE_4KB, temp_4k_buffer);
    }

    return 0;
}

int bootram_flash_chiperase(void)
{
    FLASH_ChipErase();
    return 0;
}

void bootram_user_reboot(void)
{
    NVIC_SystemReset();
}

void bootram_serial_init(void)
{
    serial_init(&m_ConsoleSerial, SER_PORT_UART0, 115200, NULL);
}

size_t bootram_serial_write(const void* buf, size_t size)
{
    return serial_write(&m_ConsoleSerial, (const void*)buf, size);
}

unsigned char bootram_serial_setbaudrate(uint32_t baudrate)
{
    serial_init(&m_ConsoleSerial, SER_PORT_UART0, baudrate, NULL);
    return 1;
}

int bootram_serial_flush(void)
{
    return serial_flush(&m_ConsoleSerial);
}

size_t bootram_serial_read(void* buf, size_t size)
{
    return serial_read(&m_ConsoleSerial, buf, size);
}
