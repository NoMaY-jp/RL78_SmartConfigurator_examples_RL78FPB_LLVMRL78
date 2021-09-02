/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* If configINCLUDE_PLATFORM_H_INSTEAD_OF_IODEFINE_H is set to 0 then iodefine.h
 * (CC-RL/LLVM-RL78/GNURL78) and additional iodefine_ext.h (LLVM-RL78/GNURL78) are
 * included and used in FreeRTOS Kernel's Renesas RL78 port but such header files
 * aren't included in case of ICCRL78 because such header files of the compiler
 * has different header file names depending on device group names.  If the macro
 * is set to 1 then platform.h is included and used in the port.  If the macro is
 * set to 2 then neither iodefine.h/iodefine_ext.h nor platform.h are included.
 * If the macro is undefined, it is set to 2 internally for backward compatibility.
 * When the RL78 Smart Configurator is used, platform.h has to be used. */
#ifndef configINCLUDE_PLATFORM_H_INSTEAD_OF_IODEFINE_H
    #define configINCLUDE_PLATFORM_H_INSTEAD_OF_IODEFINE_H    2
#endif

/* Type definitions. */

#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  uint16_t
#define portBASE_TYPE   short
#define portPOINTER_SIZE_TYPE uint16_t

typedef portSTACK_TYPE StackType_t;
typedef short BaseType_t;
typedef unsigned short UBaseType_t;

#if ( configUSE_16_BIT_TICKS == 1 )
	typedef uint16_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffff
#else
	typedef uint32_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif
/*-----------------------------------------------------------*/

/* Inline assembler specifics. */
#if ( defined( __STDC_VERSION__ ) && ( __STDC_VERSION__ >= 199901L ) )
	#define portASM( ... )    __asm volatile ( __VA_ARGS__ )
#endif

/* Workaround to reduce errors/warnings caused by e2 studio CDT's INDEXER and CODAN. */
#ifdef __CDT_PARSER__
	#ifndef __asm
		#define __asm    asm
	#endif
	#ifndef __attribute__
		#define __attribute__( ... )
	#endif
	#define portCDT_NO_PARSE( token )
#else
	#define portCDT_NO_PARSE( token )    token
#endif
/*-----------------------------------------------------------*/

/* Interrupt control macros. */

/* These macros should not be called directly, but through the
 * taskENTER_CRITICAL() and taskEXIT_CRITICAL() macros.  A check is performed
 * if configASSERT() is defined to ensure the ISP value was found to be 2
 * when an ISR safe FreeRTOS API function was executed.  ISR safe FreeRTOS API
 * functions are those that end in FromISR.  FreeRTOS maintains a separate
 * interrupt API to ensure API function and interrupt entry is as fast and as
 * simple as possible. */

void vPortRaiseISP( void );
#define portDISABLE_INTERRUPTS()  vPortRaiseISP()

void vPortResetISP( void );
#define portENABLE_INTERRUPTS()   vPortResetISP()

#ifdef configASSERT
	void vPortValidateInterruptPriority( void );
	#define portASSERT_IF_INTERRUPT_PRIORITY_INVALID()  vPortValidateInterruptPriority()
#endif
/*-----------------------------------------------------------*/

/* Critical section control macros. */
#define portNO_CRITICAL_SECTION_NESTING		( ( uint16_t ) 0 )

void vPortEnterCritical( void );
#define portENTER_CRITICAL()  vPortEnterCritical()

void vPortExitCritical( void );
#define portEXIT_CRITICAL()   vPortExitCritical()
/*-----------------------------------------------------------*/

/* Task utilities. */
#define portYIELD()	__brk()
#define portYIELD_FROM_ISR( xHigherPriorityTaskWoken ) do { if( xHigherPriorityTaskWoken ) vTaskSwitchContext(); } while( 0 )
#define portNOP()	__nop()
/*-----------------------------------------------------------*/

/* Hardwware specifics. */
#define portBYTE_ALIGNMENT	2
#define portSTACK_GROWTH	( -1 )
#define portTICK_PERIOD_MS	( ( TickType_t ) 1000 / configTICK_RATE_HZ )
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )
/*-----------------------------------------------------------*/

/* Interrupt utilities dedicated for RL78 port. */

#define portCALL_ISR_C_HANDLER_USING_FREERTOS_API( vFunction ) \
do { \
	portASM("  PUSH AX  "); \
	portASM("  PUSH BC  "); \
	portASM("  PUSH DE  "); \
	portASM("  PUSH HL  "); \
	portASM("  MOV A, CS  "); \
	portASM("  MOV X, A  "); \
	portASM("  MOV A, ES  "); \
	portASM("  PUSH AX  "); \
	portASM("  MOV CS, #0  "); \
	portASM("  MOVW BC, #%0  " portCDT_NO_PARSE( :: ) "g" ( vFunction )  ); \
	/* vPortCall_ISR_C_Handler_Using_FreeRTOS_API() enables nested interrupts. */ \
	portASM("  BR !_vPortCall_ISR_C_Handler_Using_FreeRTOS_API  "); \
} while( 0 )

#define portCALL_ISR_C_HANDLER_WITHOUT_USING_FREERTOS_API( vFunction ) \
do { \
	portASM("  PUSH AX  "); \
	portASM("  PUSH BC  "); \
	portASM("  PUSH DE  "); \
	portASM("  PUSH HL  "); \
	portASM("  MOV A, CS  "); \
	portASM("  MOV X, A  "); \
	portASM("  MOV A, ES  "); \
	portASM("  PUSH AX  "); \
	portASM("  MOV CS, #0  "); \
	portASM("  MOVW BC, #%0  " portCDT_NO_PARSE( :: ) "g" ( vFunction )  ); \
	/* vPortCall_ISR_C_Handler_Without_Using_FreeRTOS_API() enables nested interrupts. */ \
	portASM("  BR !_vPortCall_ISR_C_Handler_Without_Using_FreeRTOS_API  "); \
} while( 0 )

/* Functions for interrupt utilities. */

__near void vPortCall_ISR_C_Handler_Using_FreeRTOS_API( void );
__near void vPortCall_ISR_C_Handler_Without_Using_FreeRTOS_API( void );
/*-----------------------------------------------------------*/

/* Checks whether the current execution context is interrupt.
 * Return pdTRUE if the current execution context is interrupt,
 * pdFALSE otherwise. */
extern BaseType_t xPortIsInsideInterrupt( void );
/*-----------------------------------------------------------*/

#define portMEMORY_BARRIER()    portASM( "" portCDT_NO_PARSE( ::: ) "memory" )

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

