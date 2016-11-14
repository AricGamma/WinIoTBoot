/** @file

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include <Uefi.h>
#include <Chipset/ArmV7.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include "ArmV7Lib.h"
#include "ArmLibPrivate.h"

ARM_CACHE_TYPE
EFIAPI
ArmCacheType (
  VOID
  )
{
  return ARM_CACHE_TYPE_WRITE_BACK;
}

ARM_CACHE_ARCHITECTURE
EFIAPI
ArmCacheArchitecture (
  VOID
  )
{
  UINT32 CLIDR = ReadCLIDR ();

  return (ARM_CACHE_ARCHITECTURE)CLIDR; // BugBug Fix Me
}

BOOLEAN
EFIAPI
ArmDataCachePresent (
  VOID
  )
{
  UINT32 CLIDR = ReadCLIDR ();
  
  if ((CLIDR & 0x2) == 0x2) {
    // Instruction cache exists
    return TRUE;
  }
  if ((CLIDR & 0x7) == 0x4) {
    // Unified cache
    return TRUE;
  }
  
  return FALSE;
}
  
UINTN
EFIAPI
ArmDataCacheSize (
  VOID
  )
{
  UINT32 NumSets;
  UINT32 Associativity;
  UINT32 LineSize;
  UINT32 CCSIDR = ReadCCSIDR (0);
  
  LineSize      = (1 << ((CCSIDR & 0x7) + 2));
  Associativity = ((CCSIDR >> 3) & 0x3ff) + 1;
  NumSets       = ((CCSIDR >> 13) & 0x7fff) + 1;

  // LineSize is in words (4 byte chunks)
  return  NumSets * Associativity * LineSize * 4;      
}
  
UINTN
EFIAPI
ArmDataCacheAssociativity (
  VOID
  )
{
  UINT32 CCSIDR = ReadCCSIDR (0);

  return ((CCSIDR >> 3) & 0x3ff) + 1;
}
  
UINTN
ArmDataCacheSets (
  VOID
  )
{
  UINT32 CCSIDR = ReadCCSIDR (0);
  
  return ((CCSIDR >> 13) & 0x7fff) + 1;
}

UINTN
EFIAPI
ArmDataCacheLineLength (
  VOID
  )
{
  UINT32 CCSIDR = ReadCCSIDR (0) & 7;

  // * 4 converts to bytes
  return (1 << (CCSIDR + 2)) * 4;
}
  
BOOLEAN
EFIAPI
ArmInstructionCachePresent (
  VOID
  )
{
  UINT32 CLIDR = ReadCLIDR ();
  
  if ((CLIDR & 1) == 1) {
    // Instruction cache exists
    return TRUE;
  }
  if ((CLIDR & 0x7) == 0x4) {
    // Unified cache
    return TRUE;
  }
  
  return FALSE;
}
  
UINTN
EFIAPI
ArmInstructionCacheSize (
  VOID
  )
{
  UINT32 NumSets;
  UINT32 Associativity;
  UINT32 LineSize;
  UINT32 CCSIDR = ReadCCSIDR (1);
  
  LineSize      = (1 << ((CCSIDR & 0x7) + 2));
  Associativity = ((CCSIDR >> 3) & 0x3ff) + 1;
  NumSets       = ((CCSIDR >> 13) & 0x7fff) + 1;

  // LineSize is in words (4 byte chunks)
  return  NumSets * Associativity * LineSize * 4;      
}
  
UINTN
EFIAPI
ArmInstructionCacheAssociativity (
  VOID
  )
{
  UINT32 CCSIDR = ReadCCSIDR (1);

  return ((CCSIDR >> 3) & 0x3ff) + 1;
//  return 4;
}
  
UINTN
EFIAPI
ArmInstructionCacheSets (
  VOID
  )
{
  UINT32 CCSIDR = ReadCCSIDR (1);
  
  return ((CCSIDR >> 13) & 0x7fff) + 1;
}

UINTN
EFIAPI
ArmInstructionCacheLineLength (
  VOID
  )
{
  UINT32 CCSIDR = ReadCCSIDR (1) & 7;

  // * 4 converts to bytes
  return (1 << (CCSIDR + 2)) * 4;

//  return 64;
}


VOID
ArmV7DataCacheOperation (
  IN  ARM_V7_CACHE_OPERATION  DataCacheOperation
  )
{
  UINTN     SavedInterruptState;

  SavedInterruptState = ArmGetInterruptState ();
  ArmDisableInterrupts ();
  
  ArmV7AllDataCachesOperation (DataCacheOperation);
  
  ArmDrainWriteBuffer ();
  
  if (SavedInterruptState) {
    ArmEnableInterrupts ();
  }
}


VOID
ArmV7PoUDataCacheOperation (
  IN  ARM_V7_CACHE_OPERATION  DataCacheOperation
  )
{
  UINTN     SavedInterruptState;

  SavedInterruptState = ArmGetInterruptState ();
  ArmDisableInterrupts ();
  
  ArmV7PerformPoUDataCacheOperation (DataCacheOperation);
  
  ArmDrainWriteBuffer ();
  
  if (SavedInterruptState) {
    ArmEnableInterrupts ();
  }
}

VOID
EFIAPI
ArmInvalidateDataCache (
  VOID
  )
{
  ArmV7DataCacheOperation (ArmInvalidateDataCacheEntryBySetWay);
}

VOID
EFIAPI
ArmCleanInvalidateDataCache (
  VOID
  )
{
  ArmV7DataCacheOperation (ArmCleanInvalidateDataCacheEntryBySetWay);
}

VOID
EFIAPI
ArmCleanDataCache (
  VOID
  )
{
  ArmV7DataCacheOperation (ArmCleanDataCacheEntryBySetWay);
}

VOID
EFIAPI
ArmCleanDataCacheToPoU (
  VOID
  )
{
  ArmV7PoUDataCacheOperation (ArmCleanDataCacheEntryBySetWay);
}
