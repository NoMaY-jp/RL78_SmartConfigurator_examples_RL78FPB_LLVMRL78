#ifndef C_CPP_INTELLISENSE_HELPER_H
#define C_CPP_INTELLISENSE_HELPER_H

#ifdef __INTELLISENSE__
#define __CDT_PARSER__ /* This might be a bad practice... */

/*
Tips to make IntelliSense work with code written for LLVM-RL78 compiler
*/

typedef char * __builtin_va_list;
#pragma diag_suppress 928

void __NOP(void);
void __DI(void);
void __EI(void);

#define __near
#define __far
#define __saddr
#define __callt
#define __sectop( secname )  ( ( void * ) 0U )
#define __secend( secname )  ( ( void * ) 0U )

/* To make IntelliSense work better for r_bsp module with -frenesas-extensions option */
#if defined( __CCRL__ )
#undef __CCRL__
#endif

#endif /* __INTELLISENSE__ */

#endif /* C_CPP_INTELLISENSE_HELPER_H */
