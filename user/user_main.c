#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop(os_event_t *events);

static void ICACHE_FLASH_ATTR
gethost_callback (const char* name, ip_addr_t* ipaddr, void* arg)
{
    struct espconn* pespconn = (struct espconn*) arg;
    os_printf("user_esp_platform_dns_found %d.%d.%d.%d\n",
        *((uint8*)&ipaddr->addr), *((uint8*)&ipaddr->addr+1),
        *((uint8*)&ipaddr->addr+2), *((uint8*)&ipaddr->addr+3));

    espconn_connect_callback(connect_callback);
    if ( espconn_connect(pespconn) == 0 )
    {
        os_printf("espconn_connect OK\n");
    }
    else
    {
        os_printf("espconn_connect ERROR\n");
    }
}

static void ICACHE_FLASH_ATTR
connect_callback (void)
{
    os_printf("Connect callback is executed\n");
}

//Main code function
static void ICACHE_FLASH_ATTR
loop(os_event_t *events)
{
    os_delay_us(100000);

    system_os_post(user_procTaskPrio, 0, 0 );
}

//Init function 
void ICACHE_FLASH_ATTR
user_init()
{
    char ssid[32] = SSID;
    char password[64] = SSID_PASSWORD;
    struct station_config stationConf;

    //Set output debugger bandwidth
    uart_div_modify(0, UART_CLK_FREQ / 115200);

    //Set station mode
    wifi_set_opmode( 0x1 );

    //Set ap settings
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    if ( wifi_station_set_config(&stationConf) )
    {
        os_printf("Wi-fi configured\n");
    }
    else
    {
        os_printf("ERROR:Wi-fi NOT configured\n");
    }

    struct espconn connection;
    ip_addr_t ipaddr;
    ipaddr.addr = 3105625166U; // hardcoded IP of golink.besaba.com   
    int rtr_val;
    if ( (rtr_val = espconn_gethostbyname(&connection, HOSTNAME, &ipaddr, 
            gethost_callback)) != ESPCONN_OK )
    {
        if (rtr_val == ESPCONN_INPROGRESS)
        {
            os_printf("ERROR:espconn_gethostbyname INPROGRESS\n");
        } else if (rtr_val == ESPCONN_ARG)
        {
            os_printf("ERROR:espconn_gethostbyname ARG\n");
        } else 
        {
            os_printf("ERROR:espconn_gethostbyname UNKNOWN ERROR\n");
        }
    }

    //Start os task
    system_os_task(loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

    system_os_post(user_procTaskPrio, 0, 0 );
}

