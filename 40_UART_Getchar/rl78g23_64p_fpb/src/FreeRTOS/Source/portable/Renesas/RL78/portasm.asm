;/*
; * FreeRTOS Kernel <DEVELOPMENT BRANCH>
; * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
; *
; * SPDX-License-Identifier: MIT
; *
; * Permission is hereby granted, free of charge, to any person obtaining a copy of
; * this software and associated documentation files (the "Software"), to deal in
; * the Software without restriction, including without limitation the rights to
; * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
; * the Software, and to permit persons to whom the Software is furnished to do so,
; * subject to the following conditions:
; *
; * The above copyright notice and this permission notice shall be included in all
; * copies or substantial portions of the Software.
; *
; * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
; * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
; * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
; * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
; * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
; *
; * https://www.FreeRTOS.org
; * https://github.com/FreeRTOS
; *
; */

$include "ISR_Support.h"

	.public    _vSoftwareInterruptISR
	.public    _vPortYield
	.public    _vPortStartFirstTask
	.public    _vPortTickISR
	.public    _vPortCall_ISR_C_Handler_Using_FreeRTOS_API
	.public    _vPortCall_ISR_C_Handler_Without_Using_FreeRTOS_API

	.extern    _vTaskSwitchContext
	.extern    _xTaskIncrementTick

; The BRK software interrupt doesn't chang (doesn't decrement) the ISP bits
; value therefore the following value is forcibly set inside the BRK software
; interrupt handler.
PSW_ISP_BRK_INTERRUPT_PRIORITY_MINUS_1    .set    ((3 - 1) << 1)

; FreeRTOS yield handler.  This is installed as the BRK software interrupt
; handler.  This BRK handler is called not only outside a critical section
; but also inside a critical section.  The ISP bits value of PSW depends on
; each case but the value is saved by BRK instruction as a part of PSW and
; restored by RETB instruction as a part of PSW.  Of course, FreeRTOS is
; designed carefully that such call of the yield handler inside a critical
; section doesn't make internal data structures corrupted.
	.section .text,text
	.stack _vSoftwareInterruptISR = 16  ; This is total stack size for PC/PSW,
	.stack _vPortYield = 16         ; AX, BC, DE, HL, CS/ES and usCriticalNesting.
_vSoftwareInterruptISR:		        ; This is for compatibility with RX ports.
_vPortYield:
;	The following code can be optimized using set1 and clr1 instructions as follows.
;	PUSH	AX				        ; Mask the tick interrupt (kernel interrupt)
;	MOV		A, PSW			        ; and user interrupts which call FreeRTOS
;	AND		A, #0b11111001          ; API functions ending with FromISR (i.e.
;	OR		A, #PSW_ISP_BRK_INTERRUPT_PRIORITY_MINUS_1 ; SYSCALL interrupts) while
;	MOV		PSW, A			        ; the kernel structures are being accessed
;	POP		AX				        ; and interrupt dedicated stack is being used.
	$if (PSW_ISP_BRK_INTERRUPT_PRIORITY_MINUS_1 & 0b00000010) == 0
		CLR1	PSW.1
	$else
		SET1	PSW.1
	$endif
	$if (PSW_ISP_BRK_INTERRUPT_PRIORITY_MINUS_1 & 0b00000100) == 0
		CLR1	PSW.2
	$else
		SET1	PSW.2
	$endif
	portSAVE_CONTEXT		        ; This is the alias of the below macro call.
;	portSAVE_CONTEXT_EX TASK_CTX, SAVE_ALL
							        ; Save the context of the current task.
	EI						        ; Re-enable high priority interrupts but
							        ; any FreeRTOS API functions can't be called
							        ; in high priority interrupt ISRs.
	CALL	!!_vTaskSwitchContext   ; Call the scheduler to select the next task.
	portRESTORE_CONTEXT		        ; This is the alias of the below macro call.
;	portRESTORE_CONTEXT_EX TASK_CTX, RESTORE_ALL
							        ; Restore the context of the next task to run.
	RETB


; Starts the scheduler by restoring the context of the task that will execute
; first.
	.section .textf,textf
	.stack _vPortStartFirstTask = 4 ; This stack size is for return address to caller
							        ; but this routine never return to caller.
_vPortStartFirstTask:
	CLRW	AX				        ; A workaround to prevent the debugger from
	MOVW	!__STACK_ADDR_START - 4, AX ; messing up unwinding of the stack even if
	MOVW	!__STACK_ADDR_START - 2, AX ; the program stopped in interrupt context.
	portRESTORE_CONTEXT		        ; This is the alias of the below macro call.
;	portRESTORE_CONTEXT_EX TASK_CTX, RESTORE_ALL
							        ; Restore the context of the next task to run.
	RETI					        ; An interrupt stack frame is used so the task
                                    ; is started using a RETI instruction.

; FreeRTOS tick handler.  This is installed as the interval timer interrupt
; handler.
	.section .text,text
	.stack _vPortTickISR = 2;       ; Total stack size is 16 and other part is
							        ; incorporated in caller vTickISR of port.c.
							        ; To be exact, not 'CALL' but 'BR' is used.
_vPortTickISR:
	portSAVE_CONTEXT_EX TASK_CTX, SKIP_INT_PROLOG
							        ; Save the context of the current task.
	EI						        ; Re-enable high priority interrupts but
							        ; any FreeRTOS API functions can't be called
							        ; in high priority interrupt ISRs.
	CALL	!!_xTaskIncrementTick   ; Call the timer tick function.
	CMP0	X				        ; Check the return value is zero or not(== one).
	SKZ						        ; Skip the scheduler call if the value is zero.
	CALL	!!_vTaskSwitchContext   ; Call the scheduler to select the next task.
	portRESTORE_CONTEXT		        ; This is the alias of the below macro call.
;	portRESTORE_CONTEXT_EX TASK_CTX, RESTORE_ALL
							        ; Restore the context of the next task to run.
	RETI


; Common interrupt handler.
	.section .text,text
	.stack _vPortCall_ISR_C_Handler_Using_FreeRTOS_API = 2
							        ; Total stack size is 16 and other part is
							        ; incorporated in caller side.
							        ; To be exact, not 'CALL' but 'BR' is used.
_vPortCall_ISR_C_Handler_Using_FreeRTOS_API:
	; Argument: BC is the target interrupt handler address.
	portSAVE_CONTEXT_EX TASK_CTX, SKIP_INT_PROLOG
							        ; Save the context of the current task.
	EI						        ; Re-enable high priority interrupts but
							        ; any FreeRTOS API functions can't be called
							        ; in high priority interrupt ISRs.
	MOV		CS, #0			        ; In case of CC-RL, it shouldn't be assumed
							        ; that CS == 0.
	CALL	BC				        ; Call the target interrupt handler.
	portRESTORE_CONTEXT		        ; This is the alias of the below macro call.
;	portRESTORE_CONTEXT_EX TASK_CTX, RESTORE_ALL
							        ; Restore the context of the next task to run.
	RETI


; Common interrupt handler.
	.section .text,text
	.stack _vPortCall_ISR_C_Handler_Without_Using_FreeRTOS_API = 0
							        ; Total stack size is 14 and all part is
							        ; incorporated in caller side.
							        ; To be exact, not 'CALL' but 'BR' is used.
_vPortCall_ISR_C_Handler_Without_Using_FreeRTOS_API:
	; Argument: BC is the target interrupt handler address.
	portSAVE_CONTEXT_EX INT_REGS, SKIP_INT_PROLOG
							        ; Save the content of the registers.
	EI						        ; Re-enable high priority interrupts but
							        ; any FreeRTOS API functions can't be called
							        ; in high priority interrupt ISRs.
	MOV		CS, #0			        ; In case of CC-RL, it shouldn't be assumed
							        ; that CS == 0.
	CALL	BC				        ; Call the target interrupt handler.
	portRESTORE_CONTEXT_EX INT_REGS, RESTORE_ALL
							        ; Restore the content of the registers.
	RETI


; Install the interrupt handlers

	_vPortYield    .vector    0x7E


