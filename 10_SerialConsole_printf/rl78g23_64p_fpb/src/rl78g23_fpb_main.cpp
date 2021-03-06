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

// Please do the following steps
// 1. Change the COM Port number of USB Serial Port connected to
//    RL78/G23-64p FPB to COM9 by using Windows Device Manager.
// 2. Push 'Launch in Run mode' button of launch bar of e2 studio
//    to run the program. (This step includes launching Renesas
//    Flash Programmer V3.08. Please install it before the step.)
// 3. Open Terminal view of e2 studio then open terminal choosing
//    'Serial Terminal' mode with COM9 and 2000000 baud.

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
