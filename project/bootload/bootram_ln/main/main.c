#include "proj_config.h"
#include "ln88xx.h"
#include "hal/flash.h"
#include "hal/cache.h"
#include "ll/ll_syscon.h"
#include "mode_ctrl.h"
#include "utils/runtime/runtime.h"

static void set_interrupt_priority(void)
{
    __NVIC_SetPriorityGrouping(4);
    NVIC_SetPriority(SysTick_IRQn, 1);
    NVIC_SetPriority(UART0_IRQn, 4);
}

int main (int argc, char* argv[])
{    
    SetSysClock();
    set_interrupt_priority();
    __enable_irq();
    
    FLASH_Init();
    flash_cache_disable();
	
    ln_runtime_measure_init();
    bootram_ctrl_init();
    bootram_ctrl_loop();
	
    while(1){};
}


/**
 * @brief Hold there if something went wrong.
 */
void HardFault_Handler_C (unsigned int * hardfault_args)
{
    unsigned int stacked_r0;
    unsigned int stacked_r1;
    unsigned int stacked_r2;
    unsigned int stacked_r3;
    unsigned int stacked_r12;
    unsigned int stacked_lr;
    unsigned int stacked_pc;
    unsigned int stacked_psr;

    stacked_r0 = ((unsigned long) hardfault_args[0]);
    stacked_r1 = ((unsigned long) hardfault_args[1]);
    stacked_r2 = ((unsigned long) hardfault_args[2]);
    stacked_r3 = ((unsigned long) hardfault_args[3]);

    stacked_r12 = ((unsigned long) hardfault_args[4]);
    stacked_lr = ((unsigned long) hardfault_args[5]);
    stacked_pc = ((unsigned long) hardfault_args[6]);
    stacked_psr = ((unsigned long) hardfault_args[7]);

    bootram_console_printf("\n\n[Hard fault handler - all numbers in hex]\r\n");
    bootram_console_printf("R0 = %x\r\n", stacked_r0);
    bootram_console_printf("R1 = %x\r\n", stacked_r1);
    bootram_console_printf("R2 = %x\r\n", stacked_r2);
    bootram_console_printf("R3 = %x\r\n", stacked_r3);
    bootram_console_printf("R12 = %x\r\n", stacked_r12);
    bootram_console_printf("LR [R14] = %x  subroutine call return address\r\n", stacked_lr);
    bootram_console_printf("PC [R15] = %x  program counter\r\n", stacked_pc);
    bootram_console_printf("PSR = %x\r\n", stacked_psr);
    bootram_console_printf("BFAR = %x\r\n", (*((volatile unsigned long *)(0xE000ED38))));
    bootram_console_printf("CFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED28))));
    bootram_console_printf("HFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED2C))));
    bootram_console_printf("DFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED30))));
    bootram_console_printf("AFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED3C))));
    bootram_console_printf("SCB_SHCSR = %x\r\n", SCB->SHCSR);

    while (1);
}

static void Default_Handler(const char *func_name, int line)
{
    bootram_console_printf("%s, %d\r\n", func_name, line);
    while (1);
}

void NMI_Handler (void)
{
    Default_Handler(__func__, __LINE__);
}

void HardFault_Handler (void)
{
    Default_Handler(__func__, __LINE__);
}

void MemManage_Handler (void)
{
    Default_Handler(__func__, __LINE__);
}

void BusFault_Handler (void)
{
    Default_Handler(__func__, __LINE__);
}

void UsageFault_Handler (void)
{
    Default_Handler(__func__, __LINE__);
}


void DebugMon_Handler (void)
{
    Default_Handler(__func__, __LINE__);
}


