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

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Library includes. */
#include "string.h"

/* Hardware specifics. */
#if ( configINCLUDE_PLATFORM_H_INSTEAD_OF_IODEFINE_H == 0 )
    #include "iodefine.h"
    #include "iodefine_ext.h"
#elif ( configINCLUDE_PLATFORM_H_INSTEAD_OF_IODEFINE_H == 1 )
    #include "platform.h"
#elif ( configINCLUDE_PLATFORM_H_INSTEAD_OF_IODEFINE_H == 2 )
    /* Nothing to be included here. */
#else
    #error Invalid configINCLUDE_PLATFORM_H_INSTEAD_OF_IODEFINE_H setting - configINCLUDE_PLATFORM_H_INSTEAD_OF_IODEFINE_H must be set to 0, 1, 2, or left undefined.
#endif

/* Miscellaneous checks for this port. */
#if ( defined( configKERNEL_INTERRUPT_PRIORITY ) && !defined( configMAX_SYSCALL_INTERRUPT_PRIORITY ) ) || \
    ( !defined( configKERNEL_INTERRUPT_PRIORITY ) && defined( configMAX_SYSCALL_INTERRUPT_PRIORITY ) ) || \
    ( defined( configKERNEL_INTERRUPT_PRIORITY ) && ( configKERNEL_INTERRUPT_PRIORITY != 3 ) ) || \
    ( defined( configMAX_SYSCALL_INTERRUPT_PRIORITY ) && ( configMAX_SYSCALL_INTERRUPT_PRIORITY != 3 ) )
    #error Invalid configKERNEL_INTERRUPT_PRIORITY and/or configMAX_SYSCALL_INTERRUPT_PRIORITY setting - both must be set to 3, or left undefined.
#endif

/* The interrupt priority used by the kernel itself for the tick interrupt and
the yield interrupt (in case of Renesas RL78, the BRK software interrupt).  This
would normally be the lowest priority.  In case of Renesas RL78 port, the value
is limited to the lowest priority '3' only. */
#ifndef configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY         3
#endif

/* The highest interrupt priority from which interrupt safe FreeRTOS API calls
can be made.  Interrupts that use a priority above this (in case of Renesas RL78,
lower numeric values) will not be effected by anything the kernel is doing.  In
case of Renesas RL78 port, the value is limited to the lowest priority '3' only.
DO NOT CALL INTERRUPT SAFE FREERTOS API FROM ANY INTERRUPT THAT USE A PRIORITY
ABOVE THIS! (higher priorities are lower numeric values.) */
#ifndef configMAX_SYSCALL_INTERRUPT_PRIORITY
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    3
#endif

/* The critical nesting value is initialised to a non zero value to ensure
interrupts don't accidentally become enabled before the scheduler is started. */
#define portINITIAL_CRITICAL_NESTING  ( ( uint16_t ) 10 )

/* Initial PSW value allocated to a newly created task.
 *   11000110
 *   ||||||||-------------- Carry Flag cleared
 *   |||||||--------------- In-service priority 0 Flags set to low priority
 *   |||||----------------- In-service priority 1 Flags set to low priority
 *   ||||------------------ Register bank Select 0 Flag cleared
 *   |||------------------- Auxiliary Carry Flag cleared
 *   ||-------------------- Register bank Select 1 Flag cleared
 *   |--------------------- Zero Flag set
 *   ---------------------- Global Interrupt Flag set (enabled)
 */
#define portPSW		  ( 0xc6UL )
#define portPSW_ISP	  ( portPSW & 0x06 /* 0b00000110 */ )

/* Macros to get/set PSW's ISP bits. (Operator '&' or '|' may change Zero Flag.) */
#define portPSW_REG           ( * ( /* volatile is unnecessary in the use case here */ uint8_t * ) 0xffffa )
#define portCNV_PSW_ISP(val)  ( ( ( val ) < 0 ) ? 0 : ( ( val ) > 3 ? ( 3  << 1 ) : ( val ) << 1 ) )
#define portGET_PSW_ISP()     ( portPSW_REG & 0x06 /* 0b00000110 */ )
#define portSET_PSW_ISP(val)  ( portPSW_REG = ( portPSW_REG & 0xF9 /* 0b11111001 */ ) | ( val ) )

/* The address of the pxCurrentTCB variable, but don't know or need to know its
type. */
typedef void TCB_t;
extern volatile TCB_t * volatile pxCurrentTCB;

/* Each task maintains a count of the critical section nesting depth.  Each time
a critical section is entered the count is incremented.  Each time a critical
section is exited the count is decremented - with interrupts only being
re-enabled if the count is zero.

usCriticalNesting will get set to zero when the scheduler starts, but must
not be initialised to zero as that could cause problems during the startup
sequence. */
volatile uint16_t usCriticalNesting = portINITIAL_CRITICAL_NESTING;

/* Each ISR which uses interrupt dedicated stack maintains a count of the interrupt
nesting depth.  Each time an ISR is entered the count is incremented.  Each time
an ISR is exited the count is decremented.  The stack is switched to interrupt stack
from task stacks when the count changes from zero to one.  The stack is switched
back to task stacks from interrupt stack when the count changes from one to zero.
The count is held in the uxInterruptNesting variable.  The stack pointer value of
interrupted task stack is held in the uxSavedTaskStackPointer variable.  The value
is saved to the variable when the stack is switched to interrupt stack from task
stacks and restored from the variable when the stack is switched back to task stacks
from interrupt stack. */
volatile UBaseType_t uxInterruptNesting = 0;

/* Stores the task stack pointer when a switch is made to use the interrupt stack. */
volatile UBaseType_t uxSavedTaskStackPointer = 0;

/*-----------------------------------------------------------*/

/*
 * Sets up the periodic ISR used for the RTOS tick using the interval timer.
 * The application writer can define configSETUP_TICK_INTERRUPT() (in
 * FreeRTOSConfig.h) such that their own tick interrupt configuration is used
 * in place of prvSetupTimerInterrupt().
 */
#ifndef configSETUP_TICK_INTERRUPT
	/* The user has not provided their own tick interrupt configuration so use
	the definition in this file (which uses the interval timer). */
	static void prvSetupTimerInterrupt( void );
#endif

/*
 * Defined in portasm.S, this function starts the scheduler by loading the
 * context of the first task to run.
 */
extern void vPortStartFirstTask( void );

/*
 * Used to catch tasks that attempt to return from their implementing function.
 * This function should be located on the 16-bit program area so that procedure
 * link table (plt) is not used.
 */
static void prvTaskExitError( void ) __attribute__( ( noinline, section( ".lowtext" ), used ) );

/*
 * Force an assert.
 */
static void prvForceAssert( void ) __attribute__( ( noinline ) );

/*
 * The actual tick ISR handler.
 */

extern void vPortTickISR( void );

/*
 * The second entry point for the tick ISR handler.  This is the function that
 * jumps to the external asm routine vPortTickISR().  This function is intended
 * to be similar source code with CC-RL port.
 */
static __inline void prvTickISR( void ) __attribute__( ( always_inline ) );

/*
 * The first entry point for the tick ISR handler.  This is the function that
 * calls the inline asm function prvTickISR().
 */
void vTickISR( void ) __attribute__( ( interrupt ) );

/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See the header file portable.h.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
extern void ( prvTaskExitError_stacking_pvParameters )( void );
uint32_t *pulLocal;

	/* Stack type and pointers to the stack type are both 2 bytes. */

	/* Prevent the debugger from messing up unwinding of the stack, leaving
	space for the first two bytes of the 32-bit value. */
	pxTopOfStack--;
	pulLocal = ( uint32_t * ) pxTopOfStack;
	*pulLocal = ( uint32_t ) ( uintptr_t ) 0x0;
	pxTopOfStack--;

	/* The parameter is passed in on the stack. */
	*pxTopOfStack = ( StackType_t ) pvParameters;
	pxTopOfStack--;

	/* The return address, leaving space for the first two bytes of the
	32-bit value.  See the comments above the prvTaskExitError() prototype
	at the top of this file.  GNURL78 always uses 16-bit function pointer
	which is same size with default data pointer without __far qualifier.
	Unfortunately GNURL78 doesn't allow to use a cast from 16-bit function
	pointer to 32-bit function pointer with compiler warning therefore
	uintptr_t cast is used instead.
	Using prvTaskExitError_stacking_pvParameters in stead of prvTaskExitError
	is a workaround to prevent rl78-elf-gdb from messing up. */
	pxTopOfStack--;
	pulLocal = ( uint32_t * ) pxTopOfStack;
	*pulLocal = ( uint32_t ) ( uintptr_t ) prvTaskExitError_stacking_pvParameters;
	pxTopOfStack--;

	/* The start address / PSW value is also written in as a 32-bit value,
	so leave a space for the second two bytes. */
	pxTopOfStack--;

	/* Task function start address combined with the PSW.  The intermediate
	uintptr_t cast is used because of the above reason due to GNURL78. */
	pulLocal = ( uint32_t * ) pxTopOfStack;
	*pulLocal = ( ( uint32_t ) ( uintptr_t ) pxCode ) | ( portPSW << 24UL );
	pxTopOfStack--;

	/* Initial values for the AX, BC, DE and HL register of bank 0. */
	*pxTopOfStack = ( StackType_t ) 0x1111;
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0xBCBC;
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0xDEDE;
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x4444;
	pxTopOfStack--;

	/* ES and CS registers. */
	*pxTopOfStack = ( StackType_t ) 0x0F00;
	pxTopOfStack--;

	/* The other two register banks...register bank 3 is dedicated for use
	by interrupts so is not saved as part of the task context. */
	memset( pxTopOfStack - 8 + 1, 0x55, 8 * sizeof( StackType_t ) );
	pxTopOfStack -= 8;

	/* Finally the critical section nesting count is set to zero when the task
	first starts. */
	*pxTopOfStack = ( StackType_t ) portNO_CRITICAL_SECTION_NESTING;

	/* Return a pointer to the top of the stack that has been generated so it
	can	be stored in the task control block for the task. */
	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

static void prvTaskExitError( void )
{
	/* A function that implements a task must not exit or attempt to return to
	 * its caller as there is nothing to return to.  If a task wants to exit it
	 * should instead call vTaskDelete( NULL ).
	 *
	 * Artificially force an assert() to be triggered if configASSERT() is
	 * defined, then stop here so application writers can catch the error. */

	/* pvParameters is a workaround to prevent rl78-elf-gdb from messing up. */
	void * volatile pvParameters = 0;

	/* Just avoid compiler warning. */
	( void ) pvParameters;

	/* Defining the label prvTaskExitError_stacking_pvParameters here is
	 * a workaround to prevent rl78-elf-gdb from messing up. */
	portASM("  _prvTaskExitError_stacking_pvParameters:  ");

	/* It might be better to avoid inlined function call in this function. */
	prvForceAssert();
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
	/* Setup the hardware to generate the tick.  Interrupts are disabled when
	this function is called. */
#ifdef configSETUP_TICK_INTERRUPT
	configSETUP_TICK_INTERRUPT();
#else
	prvSetupTimerInterrupt();
#endif

	/* Restore the context of the first task that is going to run. */
	vPortStartFirstTask();

	/* Execution should not reach here as the tasks are now running! */
	return pdTRUE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* Not implemented in ports where there is nothing to return to.
	 * Artificially force an assert. */
	prvForceAssert();
}
/*-----------------------------------------------------------*/

static void prvForceAssert( void )
{
	configASSERT( usCriticalNesting == ~0U );
	portDISABLE_INTERRUPTS();

	for( ; ; )
	{
	}
}
/*-----------------------------------------------------------*/

#ifndef configSETUP_TICK_INTERRUPT
static void prvSetupTimerInterrupt( void )
{
#if( defined( configITL_CLOCK_HZ ) )

/* RL78 second generation. */
const uint32_t ulClockHz = configITL_CLOCK_HZ; /* Using the IHP clock. */
const uint32_t ulCompareMatch = ( ulClockHz / configTICK_RATE_HZ ) - 1UL;

#else

/* RL78 first generation. */
const uint16_t usClockHz = 15000UL; /* Internal clock. */
const uint16_t usCompareMatch = ( usClockHz / configTICK_RATE_HZ ) - 1UL;

	/* Use the internal 15K clock. */
	OSMC = ( uint8_t ) 0x16;

#endif

	#if( defined( configITL_CLOCK_HZ ) && defined( INTITL_vect ) )
	{
		/* Supply the 32-bit interval timer clock. */
		TML32EN = 1U;

		/* Stop the 32-bit interval timer */
		ITLCTL0 = 0x00U;
		/* Mask INTITL interrupt */
		ITLMKF0 |= 0x01; //_01_ITL_CHANNEL0_COUNT_MATCH_MASK;
		ITLS0 &= (uint16_t)~ 0x01; //_01_ITL_CHANNEL0_COUNT_MATCH_DETECTE;
		ITLMK = 1U;    /* disable INTITL interrupt */
		ITLIF = 0U;    /* clear INTITL interrupt flag */
		/* Set INTITL low priority */
		ITLPR1 = 1U;
		ITLPR0 = 1U;
		/* 32-bit interval timer used as 32-bit timer */
		ITLCTL0 |= 0x80; //_80_ITL_MODE_32BIT;
		ITLCSEL0 &= 0xF8; //_F8_ITL_CLOCK_FITL0_CLEAR;
		ITLCSEL0 |= 0x01; //_01_ITL_CLOCK_FITL0_FIHP;
		ITLFDIV00 &= 0xF8; //_F8_ITL_ITL000_FITL0_CLEAR;
		ITLFDIV00 |= 0x00; //_00_ITL_ITL000_FITL0_1;
		ITLCMP00 = ( uint16_t ) ( ulCompareMatch & 0xFFFF ); //_7CFF_ITL_ITLCMP00_VALUE; //IHP = 32MHz
		ITLCMP01 = ( uint16_t ) ( ( ulCompareMatch >> 16 ) & 0xFFFF ); //_0000_ITL_ITLCMP01_VALUE; //IHP = 32MHz

		/* Clear INTITL interrupt request and enable operation */
		ITLIF = 0U;    /* clear INTITL interrupt flag */
		ITLMK = 0U;    /* enable INTITL interrupt */

		/* Start the 32-bit interval timer */
		ITLS0 &= (uint16_t)~0x01; //_01_ITL_CHANNEL0_COUNT_MATCH_DETECTE;
		ITLMKF0 &= (uint16_t)~0x01; //~_01_ITL_CHANNEL0_COUNT_MATCH_MASK;
		ITLEN00 = 1U;
	}
	#elif( defined( INTIT_vect ) && ( INTIT_vect == 0x38 ) )
	{
		/* Supply the interval timer clock. */
		RTCEN = ( uint8_t ) 1U;

		/* Disable INTIT interrupt. */
		ITMK = ( uint8_t ) 1;

		/* Disable ITMC operation. */
		ITMC = ( uint8_t ) 0x0000;

		/* Clear INIT interrupt. */
		ITIF = ( uint8_t ) 0;

		/* Set interval and enable interrupt operation. */
		ITMC = usCompareMatch | 0x8000U;

		/* Enable INTIT interrupt. */
		ITMK = ( uint8_t ) 0;
	}
	#elif( defined( INTIT_vect ) && ( INTIT_vect == 0x3C ) )
	{
		/* Supply the interval timer clock. */
		TMKAEN = ( uint8_t ) 1U;

		/* Disable INTIT interrupt. */
		TMKAMK = ( uint8_t ) 1;

		/* Disable ITMC operation. */
		ITMC = ( uint8_t ) 0x0000;

		/* Clear INIT interrupt. */
		TMKAIF = ( uint8_t ) 0;

		/* Set interval and enable interrupt operation. */
		ITMC = usCompareMatch | 0x8000U;

		/* Enable INTIT interrupt. */
		TMKAMK = ( uint8_t ) 0;
	}
	#else

		#error Neither INTITL nor INTIT is available for the tick interrupt.

	#endif
}
#endif /* ifndef configSETUP_TICK_INTERRUPT */
/*-----------------------------------------------------------*/

static __inline void prvTickISR( void )
{
	/* Jump to the external asm routine vPortTickISR(). */
	portASM("  BR !_vPortTickISR  ");
}
/*-----------------------------------------------------------*/

void vTickISR( void )
{
	/* In case of GNURL78, CS register isn't changed by compiled code and is
	fixed to CS == 0.  But it can be changed by assembly code and interrupts
	may occur while CS != 0.  The prolog code of interrupt function doesn't
	save its value to stack unless -msave-cs-in-interrupts isn't specified.
	So the following code is added after the prolog code for convenience. */
	portASM("  MOV A, CS  ");
	portASM("  MOV [SP], A  ");
	portASM("  MOV CS, #0  ");

	/* In case of GNURL78, it is necessary to be careful when using C code
	here because GNURL78 may generate other type of prolog code for this
	interrupt function which isn't compatible with the type of prolog code
	used in portasm.S. So the following inline asm code is used instead of
	C code for safety. */
#if defined( configITL_CLOCK_HZ )
	/* The 32bit interval timer needs to clear ITLS0. */
	portASM("  CLRB !0x036B  "); /* ITLS0 = 0; */
#endif

	/* Call the inline asm function prvTickISR(). */
	prvTickISR();

	/* The following code is intended to remove the epilog code. */
	for( ; ; );
}
/*-----------------------------------------------------------*/

#define portRESET_ISP() \
	do \
	{ \
		/* In case of RL78, lower numeric value means higher interrupt priority and \
		interrputs that are lower (including equal) numeric value than ISP value are \
		accepted. */ \
		portSET_PSW_ISP( portCNV_PSW_ISP( configKERNEL_INTERRUPT_PRIORITY ) ); \
	} while(0)
/*-----------------------------------------------------------*/

#if( configASSERT_DEFINED == 1 )
#define portRAISE_ISP() \
	do \
	{ \
		if( portGET_PSW_ISP() > portCNV_PSW_ISP( configMAX_SYSCALL_INTERRUPT_PRIORITY - 1 ) ) \
		{ \
			/* In case of RL78, lower numeric value means higher interrupt priority and \
			interrputs that are lower (including equal) numeric value than ISP value are \
			accepted. */ \
			portSET_PSW_ISP( portCNV_PSW_ISP( configMAX_SYSCALL_INTERRUPT_PRIORITY - 1 ) ); \
		} \
	} while(0)
#else
#define portRAISE_ISP() \
	do \
	{ \
		/* In case of RL78, lower numeric value means higher interrupt priority and \
		interrputs that are lower (including equal) numeric value than ISP value are \
		accepted. */ \
		portSET_PSW_ISP( portCNV_PSW_ISP( configMAX_SYSCALL_INTERRUPT_PRIORITY - 1 ) ); \
	} while(0)
#endif
/*-----------------------------------------------------------*/

void vPortResetISP( void )
{
	portRESET_ISP();
}
/*-----------------------------------------------------------*/

void vPortRaiseISP( void )
{
	portRAISE_ISP();
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
	portRAISE_ISP();

	/* Now interrupts are disabled ulCriticalNesting can be accessed
	 * directly.  Increment ulCriticalNesting to keep a count of how many
	 * times portENTER_CRITICAL() has been called. */
	usCriticalNesting++;
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
	if( usCriticalNesting > portNO_CRITICAL_SECTION_NESTING )
	{
		/* Decrement the nesting count as we are leaving a critical section. */
		usCriticalNesting--;

		/* If the nesting level has reached zero then interrupts should be
		 * re-enabled. */
		if( usCriticalNesting == portNO_CRITICAL_SECTION_NESTING )
		{
			portRESET_ISP();
		}
	}
}
/*-----------------------------------------------------------*/

#if( configASSERT_DEFINED == 1 )
void vPortValidateInterruptPriority( void )
{
	/* The following assertion will fail if a service routine (ISR) for
	 * an interrupt that has been assigned a priority above
	 * configMAX_SYSCALL_INTERRUPT_PRIORITY calls an ISR safe FreeRTOS API
	 * function.  ISR safe FreeRTOS API functions must *only* be called
	 * from interrupts that have been assigned a priority at or below
	 * configMAX_SYSCALL_INTERRUPT_PRIORITY.
	 *
	 * Numerically low interrupt priority numbers represent logically high
	 * interrupt priorities, therefore the priority of the interrupt must
	 * be set to a value equal to or numerically *higher* than
	 * configMAX_SYSCALL_INTERRUPT_PRIORITY.
	 *
	 * Interrupts that use the FreeRTOS API must not be left at their
	 * default priority of zero as that is the highest possible priority,
	 * which is guaranteed to be above configMAX_SYSCALL_INTERRUPT_PRIORITY,
	 * and therefore also guaranteed to be invalid.
	 *
	 * FreeRTOS maintains separate thread and ISR API functions to ensure
	 * interrupt entry is as fast and simple as possible. */
	configASSERT( portGET_PSW_ISP() >= portCNV_PSW_ISP( configMAX_SYSCALL_INTERRUPT_PRIORITY - 1 ) );
}
#endif
/*-----------------------------------------------------------*/

BaseType_t xPortIsInsideInterrupt( void )
{
	BaseType_t xReturn;

	/* Check the value of ISP bits of PSW of the currently executing context.
	 * When ISP value is equal to the initial value, the context is not interrupt.
	 * When ISP value is not equal to the initial value, the context is interrupt. */
	if( portGET_PSW_ISP() == portPSW_ISP )
	{
		xReturn = pdFALSE;
	}
	else
	{
		xReturn = pdTRUE;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

