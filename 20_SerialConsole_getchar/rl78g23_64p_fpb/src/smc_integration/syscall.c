#include "r_smc_entry.h"

#define TX_BUFFER_SIZE 0
#define RX_BUFFER_SIZE 128

U_CONFIG_UART_PRINTF_IMPL(  UART0, TX_BUFFER_SIZE );
U_CONFIG_UART_GETCHAR_IMPL( UART0, RX_BUFFER_SIZE );

bool is_putchar_busy( void )
{
    return U_Config_UART0_Is_Send_Busy();
}

bool is_getchar_ready( void )
{
    return U_Config_UART0_Is_Getchar_Ready();
}

int write(int fileno, const unsigned char *buf, int count);
int read(int fileno, unsigned char *buf, int count);

int write(int fileno, const unsigned char *buf, int count)
{
    taskENTER_CRITICAL();
    {
        /* Just for the UART getting started. */
        U_Config_UART0_Is_Getchar_Ready();
    }
    taskEXIT_CRITICAL();

    if (1 == fileno || 2 == fileno)
    {
        if (0 < count)
        {
            bool is_try_again = true;
            do
            {
                taskENTER_CRITICAL();
                {
                    if (!U_Config_UART0_Is_Send_Busy())
                    {
                        U_Config_UART0_Send( buf, count );
                        is_try_again = false;
                    }
                }
                taskEXIT_CRITICAL();
            } while (is_try_again);
            while (U_Config_UART0_Is_Send_Busy());
        }
    }
    else
    {
        count = -1;
    }

    return count;
}

int read(int fileno, unsigned char *buf, int count)
{
    taskENTER_CRITICAL();
    {
        /* Just for the UART getting started. */
        U_Config_UART0_Is_Getchar_Ready();
    }
    taskEXIT_CRITICAL();

    if (0 == fileno)
    {
        if (0 < count)
        {
            bool is_try_again = true;
            do
            {
                taskENTER_CRITICAL();
                {
                    if (U_Config_UART0_Is_Getchar_Ready())
                    {
                        *buf = U_Config_UART0_Getchar();
                        count = 1;
                        is_try_again = false;
                    }
                }
                taskEXIT_CRITICAL();
            } while (is_try_again);
        }
    }
    else
    {
        count = -1;
    }

    return count;
}
