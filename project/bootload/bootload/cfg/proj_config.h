#ifndef _PROJ_CONFIG_H_
#define _PROJ_CONFIG_H_

#define DISABLE                                 (0)
#define ENABLE                                  (1)

#include "mem_map_ln882x.h"
/*
 * Clock settings section
 * Note:
 */
#define PLL_CLOCK                          (160000000)
#define XTAL_CLOCK                         (40000000)


/*
 * System clock config
 */
#define USE_PLL                             ENABLE

#if (USE_PLL == ENABLE)
  #define SOURCE_CLOCK                      PLL_CLOCK
#else
  #define SOURCE_CLOCK                      XTAL_CLOCK
#endif

#define SYSTEM_CLOCK                        (SOURCE_CLOCK)
#define AHBUS_CLOCK                         (SYSTEM_CLOCK)
#define APBUS0_CLOCK                        (AHBUS_CLOCK/2)
#define APBUS1_CLOCK                        (AHBUS_CLOCK/4)
#define APBUS2_CLOCK                        (AHBUS_CLOCK)
#define QSPI_CLK                            (APBUS2_CLOCK/4)


#if ((AHBUS_CLOCK % APBUS0_CLOCK) || (AHBUS_CLOCK % APBUS1_CLOCK))
  #error "AHBUS_CLOCK % APBUS0_CLOCK != 0 or AHBUS_CLOCK % APBUS1_CLOCK != 0"
#endif

/*
 * Module enable/disable control
 */
#define FLASH_XIP       DISABLE
#define FULL_ASSERT     DISABLE
#define PRINTF_OMIT     DISABLE     // when release software, set 1 to omit all printf logs



/*
 * Hardware config
 */
#define CFG_UART0_TX_BUF_SIZE      256
#define CFG_UART0_RX_BUF_SIZE      512
#define CFG_UART1_TX_BUF_SIZE      256
#define CFG_UART1_RX_BUF_SIZE      512
#define CFG_UART_BAUDRATE_LOG      115200


#endif /* _PROJ_CONFIG_H_ */


