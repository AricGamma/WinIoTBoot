#  
#  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
#  http://www.allwinnertech.com
#
#  Martin.Zheng <martinzheng@allwinnertech.com>
#  
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#  
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#  

#include <AsmMacroIoLib.h>
#include <Library/ArmLib.h>
.text
.align 2

GCC_ASM_EXPORT(ArmPlatformIsPrimaryCore)
GCC_ASM_EXPORT(ArmPlatformPeiBootAction)
GCC_ASM_EXPORT(ArmPlatformGetCorePosition)
GCC_ASM_EXPORT(ArmPlatformGetPrimaryCoreMpId)

GCC_ASM_IMPORT(_gPcd_FixedAtBuild_PcdArmPrimaryCore)
GCC_ASM_IMPORT(_gPcd_FixedAtBuild_PcdArmPrimaryCoreMask)
GCC_ASM_IMPORT(ArmReadMpidr)

//UINTN
//ArmPlatformGetPrimaryCoreMpId (
//  VOID
//  );
ASM_PFX(ArmPlatformGetPrimaryCoreMpId):
  LoadConstantToReg (_gPcd_FixedAtBuild_PcdArmPrimaryCore, r0)
  ldr   r0, [r0]
  bx    lr
//UINTN
//ArmPlatformIsPrimaryCore (
//  IN UINTN MpId
//  );
ASM_PFX(ArmPlatformIsPrimaryCore):
  LoadConstantToReg (_gPcd_FixedAtBuild_PcdArmPrimaryCoreMask, r1)
  ldr   r1, [r1]
  and   r0, r0, r1
  LoadConstantToReg (_gPcd_FixedAtBuild_PcdArmPrimaryCore, r1)
  ldr   r1, [r1]
  cmp   r0, r1
  moveq r0, #1
  movne r0, #0
  bx 	lr

ASM_PFX(ArmPlatformGetCorePosition):
  and  r1, r0, #0x03 //cpu core mask last 2 bits
  and  r0, r0, #(0x0f<<8) //cpu cluster mask bit 8-11
  add  r0, r1, r0, LSR #6
  mov pc, lr

ASM_PFX(ArmPlatformPeiBootAction):

  mov  	  r8, lr											//back up the lr
  
_SetupEarlyStack:
  bl    ASM_PFX(ArmReadMpidr)
  bl    ASM_PFX(ArmPlatformGetCorePosition)
  add   r0, r0, #1
  ldr   r1, =0x400   			       //1k stack per cpu
  ldr   r2, =0x10000 						 //sram A1 area base
  mla   r0, r0, r1,r2
  mov   sp, r0
  //we must set the TRE bit to enable the memory arttribute configuration from the section table.
  MRC     p15, 0, r0, c1, c0, 0       //read SCTLR
  BIC     r0, r0, #(1<<28)             //set TRE bit
  BIC     r0, r0, #(1<<1)             //enable unallignment access
  MCR     p15, 0, r0, c1, c0, 0       //write SCTLR
  ISB  
  bx    r8

ASM_FUNCTION_REMOVE_IF_UNREFERENCED 
