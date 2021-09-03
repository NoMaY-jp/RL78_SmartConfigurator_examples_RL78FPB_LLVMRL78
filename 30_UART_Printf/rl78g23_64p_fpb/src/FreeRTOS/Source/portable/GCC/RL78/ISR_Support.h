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


	 r8    =    0xffef0
	 r9    =    0xffef1
	r10    =    0xffef2
	r11    =    0xffef3
	r12    =    0xffef4
	r13    =    0xffef5
	r14    =    0xffef6
	r15    =    0xffef7
	r16    =    0xffee8
	r17    =    0xffee9
	r18    =    0xffeea
	r19    =    0xffeeb
	r20    =    0xffeec
	r21    =    0xffeed
	r22    =    0xffeee
	r23    =    0xffeef
	r24    =    0xffee0
	r25    =    0xffee1
	r26    =    0xffee2
	r27    =    0xffee3
	r28    =    0xffee4
	r29    =    0xffee5
	r30    =    0xffee6
	r31    =    0xffee7

; Variables used by scheduler
;------------------------------------------------------------------------------
	.extern    _pxCurrentTCB
	.extern    _usCriticalNesting
	.extern    _uxInterruptNesting
	.extern    _uxSavedTaskStackPointer
	.extern    _stack

; Define for stack symbol
;------------------------------------------------------------------------------
__STACK_ADDR_START =    _stack

; Defines for macro arguments
;------------------------------------------------------------------------------
INT_REGS           =    1
TASK_CTX           =    0
SKIP_INT_PROLOG    =    1
SAVE_ALL           =    0
SKIP_INT_EPILOG    =    1
RESTORE_ALL        =    0

;------------------------------------------------------------------------------
;   portSAVE_CONTEXT MACRO and portSAVE_CONTEXT_EX MACRO
;   Saves the context of the general purpose registers, CS and ES registers,
;   the usCriticalNesting Value and the Stack Pointer of the active Task
;   onto the task stack.
;   The former macro definition is for the backward compatibility. The latter
;   macro definition has the following two areguments.
;   mode: TASK_CTX or INT_REGS
;   option: SAVE_ALL or SKIP_INT_PROLOG
;------------------------------------------------------------------------------
.macro portSAVE_CONTEXT
	portSAVE_CONTEXT_EX TASK_CTX, SAVE_ALL
	.endm

.macro portSAVE_CONTEXT_EX mode, option
;	Save the necessary general purpose registers.
	.if  ( \option == SAVE_ALL )
		SEL       RB0
		PUSH      AX                    ; Save general purpose registers to stack.
		PUSH      BC
		PUSH      DE
		PUSH      HL
		MOV       A, CS                 ; Save CS register.
		MOV       X, A
		MOV       A, ES                 ; Save ES register.
		PUSH      AX
;	In case of GNURL78, CS register isn't changed by compiled code and is
;	fixed to CS == 0.  But it can be changed by assembly code and interrupts
;	may occur while CS != 0.  So the following code is added.
		MOV       CS, #0
	.endif
;	Save the other register banks - only necessary in the GCC port.
	SEL       RB1
	PUSH      AX
	PUSH      BC
	PUSH      DE
	PUSH      HL
	.if ( \mode == TASK_CTX )
		SEL       RB2
		PUSH      AX
		PUSH      BC
		PUSH      DE
		PUSH      HL
    .endif
;	Registers in bank 3 are for ISR use only so don't need saving.
	SEL       RB0
	.if ( \mode == TASK_CTX )
		MOVW      AX, !_usCriticalNesting ; Save the usCriticalNesting value.
		PUSH      AX
		MOVW      HL, !_pxCurrentTCB    ; Save the task stack pointer.
		MOVW      AX, SP
		MOVW      [HL], AX
    .endif
;	Switch stack pointers.  Interrupts which call FreeRTOS API functions
;	ending with FromISR can't be nested.  On the other hand, high priority
;	interrupts which don't call any FreeRTOS API functions can be nested.
;	Additionally uxInterruptNesting has to be modified in the same DI
;	period so that the next switching of the stack is perfomed correctly.
;	Don't enable nested interrupts from the beginning of interrupt until
;	the completion of switching the stack from task stacks to interrupt
;	stack.  If it is enabled before switching the stack to interrupt stack,
;	each task stack needs additional space for nested interrupt.
	.if ( \mode == TASK_CTX )
		MOVW      SP, #__STACK_ADDR_START - 4 ; Set stack pointer.
		ONEB      !_uxInterruptNesting  ; Change value: 0 --> 1.
	.else
		CMP0      !_uxInterruptNesting
		BNZ       $.L1\@                ; Skip when interrupt stack is
		                                ; already in use.
		MOVW      AX, SP                ; Save the stack pointer.
		MOVW      !_uxSavedTaskStackPointer, AX
		MOVW      SP, #__STACK_ADDR_START - 4 ; Set stack pointer.
.L1\@:
		INC       !_uxInterruptNesting  ; Change value: 0,1,2,3 --> 1,2,3,4.
	.endif
	.endm
;------------------------------------------------------------------------------

;------------------------------------------------------------------------------
;   portRESTORE_CONTEXT MACRO and portRESTORE_CONTEXT_EX MACRO
;   Restores the task Stack Pointer then use this to restore usCriticalNesting,
;   general purpose registers and the CS and ES registers of the selected task
;   from the task stack.
;   The former macro definition is for the backward compatibility. The latter
;   macro definition has the following two areguments.
;   mode: TASK_CTX or INT_REGS
;   option: RESTORE_ALL or SKIP_INT_PROLOG
;------------------------------------------------------------------------------
.macro portRESTORE_CONTEXT
	portRESTORE_CONTEXT_EX TASK_CTX, RESTORE_ALL
    .endm

.macro portRESTORE_CONTEXT_EX mode, option
;	Don't enable nested interrupts from the completion of switching the
;	stack from interrupt stack to task stacks until the RETI or RETB
;	instruction is executed.  If it is enabled after switching the stack,
;	each task stack needs additional space for nested interrupts.
;	Additionally uxInterruptNesting has to be modified in the same DI
;	period so that the next switching of the stack is perfomed correctly.
	DI
	SEL       RB0
	.if ( \mode == TASK_CTX )
		CLRB      !_uxInterruptNesting  ; Change value: 1,0(first task) --> 0,0.
		MOVW      HL, !_pxCurrentTCB    ; Restore the stack pointer.
		MOVW      AX, [HL]
		MOVW      SP, AX
		POP       AX                    ; Restore usCriticalNesting value.
		MOVW      !_usCriticalNesting, AX
;	Restore the alternative register banks - only necessary in the GCC
;	port.  Register bank 3 is dedicated for interrupts use so is not saved or
;	restored.
		SEL       RB2
		POP       HL
		POP       DE
		POP       BC
		POP       AX
	.else
		DEC       !_uxInterruptNesting  ; Change value: 1,2,3,4 --> 0,1,2,3.
		BNZ       $.L1\@                ; Skip when interrupt stack is
		                                ; still in use.
		MOVW      AX, !_uxSavedTaskStackPointer
		MOVW      SP, AX                ; Restore the stack pointer.
.L1\@:
	.endif
	SEL       RB1
	POP       HL
	POP       DE
	POP       BC
	POP       AX
;	Restore the necessary general purpose registers.
	.if ( \option == RESTORE_ALL )
		SEL       RB0
		POP       AX
		MOV       ES, A                 ; Restore the ES register.
		MOV       A, X
		MOV       CS, A                 ; Restore the CS register.
		POP       HL                    ; Restore general purpose registers.
		POP       DE
		POP       BC
		POP       AX
	.endif
	.endm
;------------------------------------------------------------------------------
