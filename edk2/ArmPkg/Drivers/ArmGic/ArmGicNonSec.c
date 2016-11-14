/** @file
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
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

#include <Uefi.h>
#include <Library/IoLib.h>
#include <Library/ArmGicLib.h>


VOID
EFIAPI
ArmGicEnableInterruptInterface (
  IN  INTN          GicInterruptInterfaceBase
  )
{  
  /*
  * Enable the CPU interface in Non-Secure world
  * Note: The ICCICR register is banked when Security extensions are implemented
  */
  MmioWrite32 (GicInterruptInterfaceBase + ARM_GIC_ICCICR, 0x1);
}

VOID
EFIAPI
ArmGicEnableDistributor (
  IN  INTN          GicDistributorBase
  )
{
  /*
   * Enable GIC distributor in Non-Secure world.
   * Note: The ICDDCR register is banked when Security extensions are implemented
   */
  MmioWrite32 (GicDistributorBase + ARM_GIC_ICDDCR, 0x1);
}
