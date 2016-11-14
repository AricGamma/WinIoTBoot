/** @file

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SUNXI_BOOTINFO_H__
#define __SUNXI_BOOTINFO_H__

typedef enum _SUNXI_BOOT_BURN_STORAGE_ID
{
  SunxiBurnStorageNand = 0,
  SunxiBurnStorageSDHC,
  SunxiBurnStorageEMMC,
  SunxiBurnStorageNor,
} SUNXI_BOOT_BURN_STORAGE_ID;


UINT32 SunxiGetBootWorkMode(VOID);
UINT32 SunxiGetBootStorage(VOID);
UINTN  SunxiGetBootDramPram(VOID);

VOID   SunxiSetBurnStorage(IN UINT32 BurnStorageType);
UINT32 SunxiGetBurnStorage(VOID);
UINT32 SunxiLibGetBootMode();


#endif
