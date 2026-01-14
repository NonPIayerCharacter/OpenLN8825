#include <string.h>
#include "usr_app.h"
#include "osal/osal.h"
#include "utils/debug/art_assert.h"
#include "utils/debug/log.h"
#include "utils/art_string.h"
#include "wifi/wifi.h"
#include "netif/ethernetif.h"
#include "wifi_manager.h"
#include "lwip/tcpip.h"
#include "hal/hal_sleep.h"
#include "drv_adc_measure.h"
#include "utils/system_parameter.h"
#include "hal/hal_adc.h"
#include "ln_kv_api.h"
#include "ln_nvds.h"

#define WIFI_TEMP_CALIBRATE               (1)

#define USR_APP_TASK_STACK_SIZE           (8*256) //Byte

#if WIFI_TEMP_CALIBRATE
static OS_Thread_t g_temp_cal_thread;
#define TEMP_APP_TASK_STACK_SIZE          (4*256) //Byte
#endif

static OS_Thread_t g_usr_app_thread;

/* declaration */
static void usr_app_task_entry(void* params);
static void temp_cal_app_task_entry(void* params);
float g_wifi_temperature = 0.0f;

void Main_Init();
void Main_OnEverySecond();

void temp_cal_app_task_entry(void* params)
{
    uint8_t cnt = 0;
    int8_t cap_comp = 0;
    uint16_t adc_val = 0;
    int16_t curr_adc = 0;

    if(NVDS_ERR_OK == ln_nvds_get_xtal_comp_val((uint8_t*)&cap_comp))
    {
        if((uint8_t)cap_comp == 0xFF)
        {
            cap_comp = 0;
        }
    }

    drv_adc_init();

    wifi_temp_cal_init(drv_adc_read(INTL_ADC_CHAN_0), cap_comp);

    while(1)
    {
        OS_MsDelay(1000);

        adc_val = drv_adc_read(INTL_ADC_CHAN_0);
        wifi_do_temp_cal_period(adc_val);

        curr_adc = (adc_val & 0xFFF);

        cnt++;
        if((cnt % 10) == 0)
        {
            g_wifi_temperature = (25 + (curr_adc - 770) / 2.54f);
        }
    }
}

#if 0//WIFI_TEMP_CALIBRATE
void temp_cal_app_task_entry(void* params)
{
    int8_t cap_comp = 0;

    if(NVDS_ERR_OK == ln_nvds_get_xtal_comp_val((uint8_t*)&cap_comp))
    {
        if((uint8_t)cap_comp == 0xFF)
        {
            cap_comp = 0;
        }
    }

    drv_adc_init();
    OS_MsDelay(1);
    wifi_temp_cal_init(drv_adc_read(INTL_ADC_CHAN_0), cap_comp);

    while(1)
    {
        OS_MsDelay(1000);
        wifi_do_temp_cal_period(drv_adc_read(INTL_ADC_CHAN_0));
    }
}
#endif


void usr_app_task_entry(void *params)
{
    hal_sleep_set_mode(ACTIVE);

    LOG(LOG_LVL_INFO, "wlib version string: %s\r\n",     wifi_lib_version_string_get());
    LOG(LOG_LVL_INFO, "wlib version number: 0x%08x\r\n", wifi_lib_version_number_get());
    LOG(LOG_LVL_INFO, "wlib build time    : %s\r\n",     wifi_lib_build_time_get());

    LOG(LOG_LVL_INFO, "SDK  version string: %s\r\n",     LN_SDK_VERSION_STRING);
    LOG(LOG_LVL_INFO, "SDK  version number: 0x%08x\r\n", LN_SDK_VERSION);
    LOG(LOG_LVL_INFO, "SDK  build time    : %s\r\n",     LN_SDK_BUILD_DATE_TIME);

    wifi_manager_init();
    Main_Init();

    while(1)
    {
        OS_MsDelay(1000);
        Main_OnEverySecond();
    }
}

void creat_usr_app_task(void)
{
    if(OS_OK != OS_ThreadCreate(&g_usr_app_thread, "UsrAPP", usr_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, USR_APP_TASK_STACK_SIZE))
    {
        ART_ASSERT(1);
    }

#if  WIFI_TEMP_CALIBRATE
    if(OS_OK != OS_ThreadCreate(&g_temp_cal_thread, "TempAPP", temp_cal_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, TEMP_APP_TASK_STACK_SIZE))
    {
        ART_ASSERT(1);
    }
#endif
}

void* os_malloc(size_t size)
{
    return OS_Malloc(size);
}

void os_free(void* ptr)
{
    OS_Free(ptr);
}


