#ifndef CONFIG_EXT_H
#define CONFIG_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(__cplusplus) && (__cplusplus >= 201103L))

#include <stdarg.h>

/****************************************************************************/

#define MD_WAIT_FINISH (true)
#define MD_DONT_WAIT_FINISH (false)

/****************************************************************************/

extern bool is_putchar_busy( void );
extern bool is_getchar_ready( void );

/****************************************************************************/

#define U_CONFIG_UART_PRINTF_IMPL( UARTx, size ) U_CONFIG_##UARTx##_PRINTF_IMPL( size )

#define Printf( UARTx, ... ) U_Config_##UARTx##_Printf( __VA_ARGS__ )

/****************************************************************************/

#define U_CONFIG_UART_PRINTF_IMPL_AS_READABLE_AS_POSSIBLE\
(\
    UARTx,\
\
    size,\
\
    g_UARTx_tx_buffer_size,\
    g_UARTx_tx_buffer,\
    g_UARTx_tx_ready_flag,\
    u_Config_UARTx_callback_sendend,\
    U_Config_UARTx_Printf,\
\
    U_Config_UARTx_Send_Ex,\
    U_CONFIG_UARTx_TXINTMASK,\
    R_Config_UARTx_Start,\
\
    u_Config_ext_header_file_Just_for_Balancing\
)\
    static const uint16_t g_UARTx_tx_buffer_size = size;\
    static uint8_t g_UARTx_tx_buffer[size];\
    volatile bool g_UARTx_tx_ready_flag = true;\
\
    U_CONFIG_SERIAL_CALLBACK_EXTERN_C void u_Config_UARTx_callback_sendend( void );\
    U_CONFIG_SERIAL_CALLBACK_EXTERN_C void u_Config_UARTx_callback_sendend( void )\
    {\
        g_UARTx_tx_ready_flag = true;\
    }\
\
    void U_Config_UARTx_Printf( const char * const fmt, ... )\
    {\
        extern const uint16_t g_UARTx_tx_buffer_size;\
        extern uint8_t g_UARTx_tx_buffer[];\
        va_list args;\
        int len;\
        if (1 == U_CONFIG_UARTx_TXINTMASK)\
        {\
            R_Config_UARTx_Start();\
        }\
        args = NULL;\
        va_start( args, fmt );\
        len = vsnprintf( (char *)g_UARTx_tx_buffer, g_UARTx_tx_buffer_size, fmt, args );\
        if (0 < len )\
        {\
            U_Config_UARTx_Send_Ex( g_UARTx_tx_buffer, len, MD_WAIT_FINISH );\
        }\
    }\
\
    extern void u_Config_ext_header_file_Remove_Compiler_Warning( void )

#define U_CONFIG_UART_PRINTF_IMPL_INTERMEDIATE_STEP( UARTx, size ) U_CONFIG_UART_PRINTF_IMPL_AS_READABLE_AS_POSSIBLE\
(\
    UARTx,\
\
    size,\
\
    g_##UARTx##_tx_buffer_size,\
    g_##UARTx##_tx_buffer,\
    g_##UARTx##_tx_ready_flag,\
    u_Config_##UARTx##_callback_sendend,\
    U_Config_##UARTx##_Printf,\
\
    U_Config_##UARTx##_Send_Ex,\
    U_CONFIG_##UARTx##_TXINTMASK,\
    R_Config_##UARTx##_Start,\
\
    u_Config_ext_header_file_Just_for_Balancing\
)

#define U_CONFIG_UART0_PRINTF_IMPL( size ) U_CONFIG_UART_PRINTF_IMPL_INTERMEDIATE_STEP( UART0, size )
#define U_CONFIG_UART1_PRINTF_IMPL( size ) U_CONFIG_UART_PRINTF_IMPL_INTERMEDIATE_STEP( UART1, size )
#define U_CONFIG_UART2_PRINTF_IMPL( size ) U_CONFIG_UART_PRINTF_IMPL_INTERMEDIATE_STEP( UART2, size )

#define U_CONFIG_UART_PRINTF_PROTO_AS_READABLE_AS_POSSIBLE\
(\
    UARTx,\
\
    g_UARTx_tx_ready_flag,\
    R_Config_UARTx_Send,\
\
    U_Config_UARTx_Is_Send_Busy,\
    U_Config_UARTx_Send,\
    U_Config_UARTx_Send_UWT,\
    U_Config_UARTx_Send_Ex,\
\
    U_Config_UARTx_Printf,\
\
    u_Config_ext_header_file_Just_for_Balancing\
)\
    static inline bool U_Config_UARTx_Is_Send_Busy( void )\
    {\
        extern volatile bool g_UARTx_tx_ready_flag;\
        return !g_UARTx_tx_ready_flag;\
    }\
\
    static inline void U_Config_UARTx_Send( const uint8_t * const tx_buf, uint16_t tx_num )\
    {\
        extern volatile bool g_UARTx_tx_ready_flag;\
        extern MD_STATUS R_Config_UARTx_Send( uint8_t * const tx_buf, uint16_t tx_num );\
        g_UARTx_tx_ready_flag = false;\
        R_Config_UARTx_Send( (/* absolutely... const */ uint8_t *)tx_buf, tx_num );\
    }\
\
    static inline void U_Config_UARTx_Send_UWT( const uint8_t * const tx_buf, uint16_t tx_num )\
    {\
        U_Config_UARTx_Send( tx_buf, tx_num );\
        do{}while (U_Config_UARTx_Is_Send_Busy());\
    }\
\
    static inline void U_Config_UARTx_Send_Ex( const uint8_t * const tx_buf, uint16_t tx_num, bool tx_wflag )\
    {\
        if (MD_DONT_WAIT_FINISH == tx_wflag)\
        {\
            U_Config_UARTx_Send( tx_buf, tx_num );\
        }\
        else\
        {\
            U_Config_UARTx_Send_UWT( tx_buf, tx_num );\
        }\
    }\
\
    extern void U_Config_UARTx_Printf( const char * const fmt, ... );\
\
    extern void u_Config_ext_header_file_Just_for_Balancing( void )

#define U_CONFIG_UART_PRINTF_PROTO_INTERMEDIATE_STEP( UARTx ) U_CONFIG_UART_PRINTF_PROTO_AS_READABLE_AS_POSSIBLE\
(\
    UARTx,\
\
    g_##UARTx##_tx_ready_flag,\
    R_Config_##UARTx##_Send,\
\
    U_Config_##UARTx##_Is_Send_Busy,\
    U_Config_##UARTx##_Send,\
    U_Config_##UARTx##_Send_UWT,\
    U_Config_##UARTx##_Send_Ex,\
\
    U_Config_##UARTx##_Printf,\
\
    u_Config_ext_header_file_Just_for_Balancing\
)

U_CONFIG_UART_PRINTF_PROTO_INTERMEDIATE_STEP( UART0  );
U_CONFIG_UART_PRINTF_PROTO_INTERMEDIATE_STEP( UART1  );
U_CONFIG_UART_PRINTF_PROTO_INTERMEDIATE_STEP( UART2  );

/****************************************************************************/

#define U_CONFIG_UART_GETCHAR_IMPL( UARTx, size ) U_CONFIG_##UARTx##_GETCHAR_IMPL( size )

#define Getchar( UARTx ) U_Config_##UARTx##_Getchar()
#define Is_Getchar_Ready( UARTx ) U_Config_##UARTx##_Is_Getchar_Ready()

/****************************************************************************/

#define U_CONFIG_UART_GETCHAR_IMPL_AS_READABLE_AS_POSSIBLE\
(\
    UARTx,\
\
    size,\
\
    g_UARTx_rx_buffer_size,\
    g_UARTx_rx_buffer,\
    g_UARTx_rx_data,\
    gp_UARTx_rx_buffer_data_put_address,\
    gp_UARTx_rx_buffer_data_get_address,\
    g_UARTx_rx_buffer_data_count,\
    g_UARTx_rx_is_operation_started,\
    u_Config_UARTx_rx_buffer_is_empty,\
    u_Config_UARTx_rx_buffer_is_full,\
    u_Config_UARTx_rx_buffer_get_data,\
    u_Config_UARTx_rx_buffer_put_data,\
    u_Config_UARTx_callback_receiveend,\
    U_Config_UARTx_Is_Getchar_Ready,\
    U_Config_UARTx_Getchar,\
    U_CONFIG_UARTx_RXINTMASK,\
    R_Config_UARTx_Start,\
    R_Config_UARTx_Receive,\
\
    u_Config_ext_header_file_Just_for_Balancing\
)\
    static const uint16_t g_UARTx_rx_buffer_size = size;\
    static volatile uint8_t g_UARTx_rx_buffer[size];\
    static volatile uint8_t g_UARTx_rx_data;\
    static volatile uint8_t * volatile gp_UARTx_rx_buffer_data_put_address = g_UARTx_rx_buffer;\
    static volatile uint8_t * gp_UARTx_rx_buffer_data_get_address = g_UARTx_rx_buffer;\
    static volatile uint16_t  g_UARTx_rx_buffer_data_count = 0;\
    static bool g_UARTx_rx_is_operation_started = false;\
\
    static inline bool u_Config_UARTx_rx_buffer_is_empty( void )\
    {\
        return 0 == g_UARTx_rx_buffer_data_count;\
    }\
\
    static inline bool u_Config_UARTx_rx_buffer_is_full( void )\
    {\
        return g_UARTx_rx_buffer_size <= g_UARTx_rx_buffer_data_count;\
    }\
\
    static inline void u_Config_UARTx_rx_buffer_get_data( uint8_t *pc )\
    {\
        *pc = *gp_UARTx_rx_buffer_data_get_address++;\
        if ( g_UARTx_rx_buffer + g_UARTx_rx_buffer_size <= gp_UARTx_rx_buffer_data_get_address)\
        {\
            gp_UARTx_rx_buffer_data_get_address = g_UARTx_rx_buffer;\
        }\
        g_UARTx_rx_buffer_data_count--;\
    }\
\
    static inline void u_Config_UARTx_rx_buffer_put_data( uint8_t c )\
    {\
        *gp_UARTx_rx_buffer_data_put_address++ = c;\
        if ( g_UARTx_rx_buffer + g_UARTx_rx_buffer_size <= gp_UARTx_rx_buffer_data_put_address)\
        {\
            gp_UARTx_rx_buffer_data_put_address = g_UARTx_rx_buffer;\
        }\
        g_UARTx_rx_buffer_data_count++;\
    }\
\
    U_CONFIG_SERIAL_CALLBACK_EXTERN_C void u_Config_UARTx_callback_receiveend( void );\
    U_CONFIG_SERIAL_CALLBACK_EXTERN_C void u_Config_UARTx_callback_receiveend( void )\
    {\
        if (!u_Config_UARTx_rx_buffer_is_full())\
        {\
            u_Config_UARTx_rx_buffer_put_data( g_UARTx_rx_data );\
        }\
        R_Config_UARTx_Receive( (/* obviously... volatile */ uint8_t *)&g_UARTx_rx_data, 1 );\
    }\
\
    bool U_Config_UARTx_Is_Getchar_Ready( void )\
    {\
        if (1 == U_CONFIG_UARTx_RXINTMASK)\
        {\
            R_Config_UARTx_Start();\
        }\
        if (!g_UARTx_rx_is_operation_started)\
        {\
            R_Config_UARTx_Receive( (/* obviously... volatile */ uint8_t *)&g_UARTx_rx_data, 1 );\
            g_UARTx_rx_is_operation_started = true;\
        }\
        return !u_Config_UARTx_rx_buffer_is_empty();\
    }\
\
    uint8_t U_Config_UARTx_Getchar( void )\
    {\
        uint8_t c;\
        do{}while (!U_Config_UARTx_Is_Getchar_Ready());\
        __DI();\
        {\
            u_Config_UARTx_rx_buffer_get_data( &c );\
        }\
        __EI();\
        return c;\
    }\
\
    extern void u_Config_ext_header_file_Remove_Compiler_Warning( void )

#define U_CONFIG_UART_GETCHAR_IMPL_INTERMEDIATE_STEP( UARTx, size ) U_CONFIG_UART_GETCHAR_IMPL_AS_READABLE_AS_POSSIBLE\
(\
    UARTx,\
\
    size,\
\
    g_##UARTx##_rx_buffer_size,\
    g_##UARTx##_rx_buffer,\
    g_##UARTx##_rx_data,\
    gp_##UARTx##_rx_buffer_data_put_address,\
    gp_##UARTx##_rx_buffer_data_get_address,\
    g_##UARTx##_rx_buffer_data_count,\
    g_##UARTx##_rx_is_operation_started,\
    u_Config_##UARTx##_rx_buffer_is_empty,\
    u_Config_##UARTx##_rx_buffer_is_full,\
    u_Config_##UARTx##_rx_buffer_get_data,\
    u_Config_##UARTx##_rx_buffer_put_data,\
    u_Config_##UARTx##_callback_receiveend,\
    U_Config_##UARTx##_Is_Getchar_Ready,\
    U_Config_##UARTx##_Getchar,\
    U_CONFIG_##UARTx##_RXINTMASK,\
    R_Config_##UARTx##_Start,\
    R_Config_##UARTx##_Receive,\
\
    u_Config_ext_header_file_Just_for_Balancing\
)

#define U_CONFIG_UART0_GETCHAR_IMPL( size ) U_CONFIG_UART_GETCHAR_IMPL_INTERMEDIATE_STEP( UART0, size )
#define U_CONFIG_UART1_GETCHAR_IMPL( size ) U_CONFIG_UART_GETCHAR_IMPL_INTERMEDIATE_STEP( UART1, size )
#define U_CONFIG_UART2_GETCHAR_IMPL( size ) U_CONFIG_UART_GETCHAR_IMPL_INTERMEDIATE_STEP( UART2, size )

#define U_CONFIG_UART_GETCHAR_PROTO_AS_READABLE_AS_POSSIBLE\
(\
    UARTx,\
\
    U_Config_UARTx_Is_Getchar_Ready,\
    U_Config_UARTx_Getchar,\
\
    u_Config_ext_header_file_Just_for_Balancing\
)\
    extern bool U_Config_UARTx_Is_Getchar_Ready( void );\
    extern uint8_t U_Config_UARTx_Getchar( void );\
\
    extern void u_Config_ext_header_file_Just_for_Balancing( void )

#define U_CONFIG_UART_GETCHAR_PROTO_INTERMEDIATE_STEP( UARTx ) U_CONFIG_UART_GETCHAR_PROTO_AS_READABLE_AS_POSSIBLE\
(\
    UARTx,\
\
    U_Config_##UARTx##_Is_Getchar_Ready,\
    U_Config_##UARTx##_Getchar,\
\
    u_Config_ext_header_file_Just_for_Balancing\
)

U_CONFIG_UART_GETCHAR_PROTO_INTERMEDIATE_STEP( UART0  );
U_CONFIG_UART_GETCHAR_PROTO_INTERMEDIATE_STEP( UART1  );
U_CONFIG_UART_GETCHAR_PROTO_INTERMEDIATE_STEP( UART2  );

/****************************************************************************/

#define U_CONFIG_UART0_TXINTMASK  STMK0
#define U_CONFIG_UART1_TXINTMASK  STMK1
#define U_CONFIG_UART2_TXINTMASK  STMK2
#define U_CONFIG_UART0_RXINTMASK  SRMK0
#define U_CONFIG_UART1_RXINTMASK  SRMK1
#define U_CONFIG_UART2_RXINTMASK  SRMK2

/****************************************************************************/

#ifdef __cplusplus
#define U_CONFIG_SERIAL_CALLBACK_EXTERN_C extern "C"
#else
#define U_CONFIG_SERIAL_CALLBACK_EXTERN_C
#endif

#define U_CONFIG_SERIAL_CALLBACK_SENDEND( SERIALx )\
    do{\
        extern void u_Config_##SERIALx##_callback_sendend( void );\
        u_Config_##SERIALx##_callback_sendend();\
        r_Config_##SERIALx##_callback_sendend();\
    }while (0)
#define U_CONFIG_SERIAL_CALLBACK_RECEIVEEND( SERIALx )\
    do{\
        extern void u_Config_##SERIALx##_callback_receiveend( void );\
        u_Config_##SERIALx##_callback_receiveend();\
        r_Config_##SERIALx##_callback_receiveend();\
    }while (0)

#define U_CONFIG_UART0_CALLBACK_SENDEND( ) U_CONFIG_SERIAL_CALLBACK_SENDEND( UART0  )
#define U_CONFIG_UART1_CALLBACK_SENDEND( ) U_CONFIG_SERIAL_CALLBACK_SENDEND( UART1  )
#define U_CONFIG_UART2_CALLBACK_SENDEND( ) U_CONFIG_SERIAL_CALLBACK_SENDEND( UART2  )
#define U_CONFIG_UART0_CALLBACK_RECEIVEEND( )  U_CONFIG_SERIAL_CALLBACK_RECEIVEEND(  UART0  )
#define U_CONFIG_UART1_CALLBACK_RECEIVEEND( )  U_CONFIG_SERIAL_CALLBACK_RECEIVEEND(  UART1  )
#define U_CONFIG_UART2_CALLBACK_RECEIVEEND( )  U_CONFIG_SERIAL_CALLBACK_RECEIVEEND(  UART2  )

#define r_Config_UART0_callback_sendend( ... )    r_Config_UART0_callback_sendendXXX##__VA_ARGS__()
#define r_Config_UART0_callback_sendendXXXvoid()  r_Config_UART0_callback_sendend( void )
#define r_Config_UART0_callback_sendendXXX()      U_CONFIG_UART0_CALLBACK_SENDEND()

#define r_Config_UART1_callback_sendend( ... )    r_Config_UART1_callback_sendendXXX##__VA_ARGS__()
#define r_Config_UART1_callback_sendendXXXvoid()  r_Config_UART1_callback_sendend( void )
#define r_Config_UART1_callback_sendendXXX()      U_CONFIG_UART1_CALLBACK_SENDEND()

#define r_Config_UART2_callback_sendend( ... )    r_Config_UART2_callback_sendendXXX##__VA_ARGS__()
#define r_Config_UART2_callback_sendendXXXvoid()  r_Config_UART2_callback_sendend( void )
#define r_Config_UART2_callback_sendendXXX()      U_CONFIG_UART2_CALLBACK_SENDEND()

#define r_Config_UART0_callback_receiveend( ... )     r_Config_UART0_callback_receiveendXXX##__VA_ARGS__()
#define r_Config_UART0_callback_receiveendXXXvoid()   r_Config_UART0_callback_receiveend( void )
#define r_Config_UART0_callback_receiveendXXX()       U_CONFIG_UART0_CALLBACK_RECEIVEEND()

#define r_Config_UART1_callback_receiveend( ... )     r_Config_UART1_callback_receiveendXXX##__VA_ARGS__()
#define r_Config_UART1_callback_receiveendXXXvoid()   r_Config_UART1_callback_receiveend( void )
#define r_Config_UART1_callback_receiveendXXX()       U_CONFIG_UART1_CALLBACK_RECEIVEEND()

#define r_Config_UART2_callback_receiveend( ... )     r_Config_UART2_callback_receiveendXXX##__VA_ARGS__()
#define r_Config_UART2_callback_receiveendXXXvoid()   r_Config_UART2_callback_receiveend( void )
#define r_Config_UART2_callback_receiveendXXX()       U_CONFIG_UART2_CALLBACK_RECEIVEEND()

/****************************************************************************/

#endif /* (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(__cplusplus) && (__cplusplus >= 201103L)) */

#ifdef __cplusplus
}
#endif

#endif
