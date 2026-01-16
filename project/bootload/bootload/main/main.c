#include "proj_config.h"
#include "ln88xx.h"

#include "hal/flash.h"
#include "hal/cache.h"
#include "hal/hal_gpio.h"

#include "utils/debug/log.h"

#include "flash_partition_mgr.h"
#include "ln_nvds.h"
#include "ota_agent.h"

#include "utils/debug/log.h"
#include "serial.h"
#include "utils/reboot_trace/reboot_trace.h"
typedef void (*jump_func_t)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);
extern uint32_t boot_header_info_init(void);
extern Serial_t m_LogSerial;
extern void log_deinit();

typedef void(*entry_point_t)(void);

static void set_interrupt_priority(void)
{
    __NVIC_SetPriorityGrouping(4);

    NVIC_SetPriority(SysTick_IRQn,   1);
    NVIC_SetPriority(MAC_IRQn,       2);
    NVIC_SetPriority(PAOTD_IRQn,     2);
    NVIC_SetPriority(WDT_IRQn,       4);
    NVIC_SetPriority(EXTERNAL_IRQn,  4);
    NVIC_SetPriority(RTC_IRQn,       4);
    NVIC_SetPriority(DMA_IRQn,       4);
    NVIC_SetPriority(QSPI_IRQn,      4);
    NVIC_SetPriority(SDIO_FUN1_IRQn, 4);
    NVIC_SetPriority(SDIO_FUN2_IRQn, 4);
    NVIC_SetPriority(SDIO_FUN3_IRQn, 4);
    NVIC_SetPriority(SDIO_FUN4_IRQn, 4);
    NVIC_SetPriority(SDIO_FUN5_IRQn, 4);
    NVIC_SetPriority(SDIO_FUN6_IRQn, 4);
    NVIC_SetPriority(SDIO_FUN7_IRQn, 4);
    NVIC_SetPriority(SDIO_ASYNC_HOST_IRQn, 4);
    NVIC_SetPriority(SDIO_M2S_IRQn,  4);
    NVIC_SetPriority(CM4_INTR0_IRQn, 4);
    NVIC_SetPriority(CM4_INTR1_IRQn, 4);
    NVIC_SetPriority(CM4_INTR2_IRQn, 4);
    NVIC_SetPriority(CM4_INTR3_IRQn, 4);
    NVIC_SetPriority(CM4_INTR4_IRQn, 4);
    NVIC_SetPriority(CM4_INTR5_IRQn, 4);
    NVIC_SetPriority(ADC_IRQn,       4);
    NVIC_SetPriority(TIMER_IRQn,     4);
    NVIC_SetPriority(I2C0_IRQn,      4);
    NVIC_SetPriority(I2C1_IRQn,      4);
    NVIC_SetPriority(SPI0_IRQn,      4);
    NVIC_SetPriority(SPI2_IRQn,      4);
    NVIC_SetPriority(UART0_IRQn,     4);
    NVIC_SetPriority(UART1_IRQn,     4);
    NVIC_SetPriority(SPI1_IRQn,      4);
#if BLE_SUPPORT==ENABLE
    NVIC_SetPriority(GPIO_IRQn, 3);//BLE use the gpio for IRQ, need higher priority
#else
    NVIC_SetPriority(GPIO_IRQn, 4);
#endif
    NVIC_SetPriority(I2S_IRQn, 4);
}

static void jump_to_user_application(uint32_t app_offset)
{
    //Enable QSPI 4bit mode
    FLASH_QuadModeEnable(1);

    //Init Flash cache
    flash_cache_init(0);

    //Prepare for jump
    uint32_t *vec_int_base = (uint32_t *)(CACHE_FLASH_BASE + app_offset);
    jump_func_t *jump_func = (jump_func_t *)(vec_int_base + 1);
    __set_MSP(*vec_int_base);

    LOG(LOG_LVL_INFO, "Jumping to 0x%08X...\r\n", vec_int_base);
		serial_flush(&m_LogSerial);
	  uint32_t delay = 0xFFFF;
	  while(delay--);
		log_deinit();

    // Jump to user's Reset_Handler
    //(*jump_func)(CACHE_FLASH_BASE+app_offset,1,2,3);
	  entry_point_t entry_point_fun = (entry_point_t)(*(vec_int_base + 1));
    entry_point_fun();
}

int main (int argc, char* argv[])
{
    partition_info_t nvds_part_info;
    chip_reboot_cause_t reboot_cause = 0;

    reboot_cause = ln_chip_get_reboot_cause();
    
    SetSysClock();

    set_interrupt_priority();
    __enable_irq();
    boot_header_info_init();
    log_init();
    FLASH_Init();
    flash_cache_disable();

    LOG(LOG_LVL_INFO, "\r\nLightningSemi LN882x custom bootloader\r\n");
    LOG(LOG_LVL_INFO, "Reboot reason - %u\r\n", reboot_cause);
	
    if (LN_TRUE != ln_verify_partition_table()) {
				LOG(LOG_LVL_ERROR, "Failed to get partitions!\r\n");
    }
    
    if (LN_TRUE != ln_fetch_partition_info(PARTITION_TYPE_NVDS, &nvds_part_info)) {
				LOG(LOG_LVL_WARN, "No NVDS partition!\r\n");
    }

    if (NVDS_ERR_OK != ln_nvds_init(nvds_part_info.start_addr)) {
				LOG(LOG_LVL_WARN, "NVDS init failed.\r\n");
    }

    ota_port_init();
		ota_err_t err = ota_boot_upgrade_agent(jump_to_user_application);
    if(err != OTA_ERR_NONE)
    {
        // TODO:process error code
				LOG(LOG_LVL_ERROR, "ota_boot_upgrade_agent failed with %u.\r\n", err);
    }
		LOG(LOG_LVL_ERROR, "Failed to boot firmware!");
    while(1);
}



void MemManage_Handler (void)
{

}

void BusFault_Handler (void)
{

}

void UsageFault_Handler (void)
{

}

