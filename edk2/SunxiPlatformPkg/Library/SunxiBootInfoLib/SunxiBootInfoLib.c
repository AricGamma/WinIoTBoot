/** @file
  Implement EFI Random Number Generator runtime services via Rng Lib.
  
  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi/UefiBaseType.h>
#include <Pi/PiPeiCis.h>

#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <Library/HobLib.h>
#include <Guid/SunxiBootInfoHob.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiBootInfoLib.h>

SUNXI_BOOTINFO_HOB      *BootInfoHob = NULL;

UINT32 SunxiLibGetBootMode()
{
  SUNXI_BOOTINFO_HOB      *Hob;
  Hob =  GetFirstGuidHob (&gSunxiBootInfoHobGuid);
  if (Hob == NULL) {
    DEBUG((DEBUG_WARN, "--%s:No BootInfo Guid found, this protocol will not install. \n",__FUNCTION__));
    return EFI_NOT_FOUND;
  }

  return Hob->WorkMode;
}

UINT32 SunxiLibGetBootInfo(SUNXI_BOOTINFO_HOB **Hob)
{
  *Hob =  GetFirstGuidHob (&gSunxiBootInfoHobGuid);
  if (Hob == NULL) {
    DEBUG((DEBUG_WARN, "--%s:No BootInfo Guid found, this protocol will not install. \n",__FUNCTION__));
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

UINT32 SunxiGetBootWorkMode(VOID)
{
  ASSERT(BootInfoHob != NULL);
  return BootInfoHob->WorkMode;
}

UINT32 SunxiGetBootStorage(VOID)
{
  ASSERT(BootInfoHob != NULL);
  return BootInfoHob->StorageType;
}

UINTN SunxiGetBootDramPram(VOID)
{
  ASSERT(BootInfoHob != NULL);
  return (UINTN)(BootInfoHob->DramPara);
}

VOID SunxiSetBurnStorage(IN UINT32 StorageType)
{
  ASSERT(BootInfoHob != NULL);
  BootInfoHob->BurnStorageType= StorageType;
}

UINT32 SunxiGetBurnStorage(VOID)
{
  ASSERT(BootInfoHob != NULL);
  return BootInfoHob->BurnStorageType;
}

/**
  Initialize the state information for the RngDxe

  @param  ImageHandle   of the loaded driver
  @param  SystemTable   PoINT32er to the System Table

  @retval EFI_SUCCESS           Protocol registered
  @retval EFI_OUT_OF_RESOURCES  Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR      Hardware problems

**/
EFI_STATUS
EFIAPI
SunxiBootInfoConstructor (
VOID
  )
{
  EFI_STATUS Status = EFI_SUCCESS;

  DEBUG((DEBUG_INFO, "++%a\n",__func__));

  BootInfoHob = GetFirstGuidHob (&gSunxiBootInfoHobGuid);

  if (BootInfoHob == NULL) {
    DEBUG((DEBUG_WARN, "--%s:No BootInfo Guid found, this protocol will not install. \n",__FUNCTION__));
    return EFI_NOT_FOUND;
  }
 
  return Status;
}
