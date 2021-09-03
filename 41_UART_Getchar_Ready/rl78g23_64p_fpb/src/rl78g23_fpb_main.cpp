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
// 3. This example uses MCU's UART1 (TxD1/RxD1 = Arduino's D1/D0)
//    and the UART is configured with 9600 baud.

U_CONFIG_UART_PRINTF_IMPL(  UART1, 128 /* transmit buffer size */ );
U_CONFIG_UART_GETCHAR_IMPL( UART1, 128 /* receive ring buffer size */ );

void main_task(void *pvParameters)
{
    INTERNAL_NOT_USED(pvParameters);

    char c;
    int i;

    for (;;)
    {
        Printf( UART1, "Please enter a character:\r\n" );

        for (i = 0; i < 3; i++)
        {
            vTaskDelay( pdMS_TO_TICKS( 1000 ) );

            if (Is_Getchar_Ready( UART1 ))
            {
                c = Getchar( UART1 );
                Printf( UART1, "\r\nYeah, %c is entered.\r\n\r\n", c );

                vTaskDelay( pdMS_TO_TICKS( 1000 ) );

                break;
            }
        }
    }

    /* vTaskDelete(NULL); */
}
