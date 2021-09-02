/***********************************************************************
*
*  FILE        : tb_rx65n_main.c
*  DATE        : 2019-09-18
*  DESCRIPTION : Main Program
*
*  NOTE:THIS IS A TYPICAL EXAMPLE.
*
***********************************************************************/

extern "C" {
#include "task_function.h"
#include "r_smc_entry.h"
#include "rl78g23fpbdef.h"
}

// Please open the following e2 studio's views
// 1. Debugger Console View
// 2. Visual Expression View

void main_task(void *pvParameters)
{
    INTERNAL_NOT_USED(pvParameters);

    LED0 = LED_ON;

    for (;;)
    {
        printf( "Hello World\r\n" );

        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
        LED0 = ~LED0;

        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
        LED0 = ~LED0;
    }

    /* vTaskDelete(NULL); */
}
