/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  tangmanliang <tangmanliang@allwinnertech.com>
*  
*  This program and the accompanying materials                          
*  are licensed and made available under the terms and conditions of the BSD License         
*  which accompanies this distribution.  The full text of the license may be found at        
*  http://opensource.org/licenses/bsd-license.php                                            
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
*
**/

#include <AsmMacroIoLib.h>
#include <Base.h>
#include <Library/PcdLib.h>
#include <AutoGen.h>
#include <Chipset/ArmV7.h>

#include <boot0_include/config.h>
.text
.align 3

GCC_ASM_IMPORT(FesMain)
GCC_ASM_EXPORT(_ModuleEntryPoint)

StartupAddr: .word    FesMain

ASM_PFX(_ModuleEntryPoint):

	stmfd sp!, {lr}
//        bl  clear_bss 
	bl  FesMain 
	ldmfd sp!, {lr}
#ifndef CONFIG_ARCH_SUN8IW10P1
	mov pc, lr
#else
	ldr r0, =0x40200000
	mov r1, sp
	mov r2, #0x2000

memcpy_loop:
	ldmia   r1!, {r7-r10}
	stmia   r0!, {r7-r10}
	sub     r2, r2, #16
	cmp     r2, #0
	bne     memcpy_loop

	ldr     r0, =0x40200000
	mov     sp, r0
	mov     pc, lr
#endif

#if 0
clear_bss:
	ldr	r0, =__bss_start
	ldr	r1, =_bss_end__

	mov	r2, #0x00000000		/* clear			    */

clbss_l:
	str	r2, [r0]		/* clear loop...		    */
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l

	mov pc, lr
#endif
