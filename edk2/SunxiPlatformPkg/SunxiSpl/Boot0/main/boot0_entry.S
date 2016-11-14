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
#include <AutoGen.h>
#include <Chipset/ArmV7.h>

#include <boot0_include/config.h>
#include <boot0_include/arm_a8.h>

.text
.align 3

GCC_ASM_IMPORT(Boot0Main)
GCC_ASM_EXPORT(_ModuleEntryPoint)

StartupAddr: .word    Boot0Main

ASM_PFX(_ModuleEntryPoint):

_ModuleEntryPoint:
	mrs r0, cpsr
	bic r0, r0, #ARMV7_MODE_MASK
	orr r0, r0, #ARMV7_SVC_MODE
	orr r0, r0, #( ARMV7_IRQ_MASK | ARMV7_FIQ_MASK )    @// After reset, ARM automaticly disables IRQ and FIQ, and runs in SVC mode.
	bic r0, r0, #(1<<9)                                 @// set little-endian
	msr cpsr_c, r0

	mrc	p15, 0, r0, c1, c0, 0
#ifdef CONFIG_ARCH_SUN8IW10P1
	orr	r0, r0, #0x00002000	@ set bits 13 (--V-)
#else
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
#endif
	bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00000800	@ set bit 11 (Z---) BTB
	bic	r0, r0, #0x00001000	@ clear bit 12 (I) I-cache
	mcr	p15, 0, r0, c1, c0, 0

	ldr sp, =CONFIG_BOOT0_STACK_BOTTOM
//	bl  clear_bss
//	bl  cpu_init_s 
//      ldr   r4, StartupAddr
//	bl    r4
        bl    Boot0Main

#if 0
clear_bss:
	ldr	r0, =__bss_start
	ldr	r1, =__bss_end

	mov	r2, #0x00000000		/* clear			    */

clbss_l:
	str	r2, [r0]		/* clear loop...		    */
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l

	mov pc, lr
#endif

