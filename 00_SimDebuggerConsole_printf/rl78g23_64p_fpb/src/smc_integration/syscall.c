#include "freertos_start.h"
#include <string.h>

#define PRINTF_BUFF_SIZE 128

unsigned char buf2[PRINTF_BUFF_SIZE];

int write(int  fileno, const unsigned char *buf, int  count);

int write(int  fileno, const unsigned char *buf, int  count)
{
    if (fileno == 1 || fileno == 2)
    {
        if (count < PRINTF_BUFF_SIZE)
        {
            memcpy( buf2, buf, count );
            buf2[count] = 0;
        }
        else
        {
            memcpy( buf2, buf, PRINTF_BUFF_SIZE - 1 );
            buf2[PRINTF_BUFF_SIZE - 1] = 0;
        }
        /* Write the string to the Debug Console. */
        vPrintString( (const char *)buf2 );
    }
    else
    {
        /* Force an assert. */
        vAssertCalled();
    }

    return count;
}
