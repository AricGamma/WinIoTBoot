/** @file
*
*  Copyright (c) 2013-2014, ARM Limited. All rights reserved.
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

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>

#include <ArmPlatform.h>

// The total number of descriptors, including the final "end-of-table" descriptor.
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS 11

// DDR attributes
#define DDR_ATTRIBUTES_CACHED           ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define DDR_ATTRIBUTES_UNCACHED         ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                                    Virtual Memory mapping. This array must be ended by a zero-filled
                                    entry

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR** VirtualMemoryMap
  )
{
  ARM_MEMORY_REGION_ATTRIBUTES  CacheAttributes;
  UINTN                         Index = 0;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;

  ASSERT (VirtualMemoryMap != NULL);

  VirtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR*)AllocatePages(EFI_SIZE_TO_PAGES (sizeof(ARM_MEMORY_REGION_DESCRIPTOR) * MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS));
  if (VirtualMemoryTable == NULL) {
      return;
  }

  if (FeaturePcdGet(PcdCacheEnable) == TRUE) {
      CacheAttributes = DDR_ATTRIBUTES_CACHED;
  } else {
      CacheAttributes = DDR_ATTRIBUTES_UNCACHED;
  }

  // SMB CS0 - NOR0 Flash
  VirtualMemoryTable[Index].PhysicalBase    = ARM_VE_SMB_NOR0_BASE;
  VirtualMemoryTable[Index].VirtualBase     = ARM_VE_SMB_NOR0_BASE;
  VirtualMemoryTable[Index].Length          = SIZE_256KB * 255;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;
  // Environment Variables region
  VirtualMemoryTable[++Index].PhysicalBase  = ARM_VE_SMB_NOR0_BASE + (SIZE_256KB * 255);
  VirtualMemoryTable[Index].VirtualBase     = ARM_VE_SMB_NOR0_BASE + (SIZE_256KB * 255);
  VirtualMemoryTable[Index].Length          = SIZE_64KB * 4;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // SMB CS2 & CS3 - Off-chip (motherboard) peripherals
  VirtualMemoryTable[++Index].PhysicalBase  = ARM_VE_SMB_PERIPH_BASE;
  VirtualMemoryTable[Index].VirtualBase     = ARM_VE_SMB_PERIPH_BASE;
  VirtualMemoryTable[Index].Length          = ARM_VE_SMB_PERIPH_SZ;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // Juno OnChip non-secure ROM
  VirtualMemoryTable[++Index].PhysicalBase  = ARM_JUNO_NON_SECURE_ROM_BASE;
  VirtualMemoryTable[Index].VirtualBase     = ARM_JUNO_NON_SECURE_ROM_BASE;
  VirtualMemoryTable[Index].Length          = ARM_JUNO_NON_SECURE_ROM_SZ;
  VirtualMemoryTable[Index].Attributes      = CacheAttributes;

  // Juno OnChip peripherals
  VirtualMemoryTable[++Index].PhysicalBase  = ARM_JUNO_PERIPHERALS_BASE;
  VirtualMemoryTable[Index].VirtualBase     = ARM_JUNO_PERIPHERALS_BASE;
  VirtualMemoryTable[Index].Length          = ARM_JUNO_PERIPHERALS_SZ;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // Juno OnChip non-secure SRAM
  VirtualMemoryTable[++Index].PhysicalBase  = ARM_JUNO_NON_SECURE_SRAM_BASE;
  VirtualMemoryTable[Index].VirtualBase     = ARM_JUNO_NON_SECURE_SRAM_BASE;
  VirtualMemoryTable[Index].Length          = ARM_JUNO_NON_SECURE_SRAM_SZ;
  VirtualMemoryTable[Index].Attributes      = CacheAttributes;

  // Juno SOC peripherals
  VirtualMemoryTable[++Index].PhysicalBase  = ARM_JUNO_SOC_PERIPHERALS_BASE;
  VirtualMemoryTable[Index].VirtualBase     = ARM_JUNO_SOC_PERIPHERALS_BASE;
  VirtualMemoryTable[Index].Length          = ARM_JUNO_SOC_PERIPHERALS_SZ;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // DDR
  VirtualMemoryTable[++Index].PhysicalBase  = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].VirtualBase     = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].Length          = PcdGet64 (PcdSystemMemorySize);
  VirtualMemoryTable[Index].Attributes      = CacheAttributes;

  // End of Table
  VirtualMemoryTable[++Index].PhysicalBase  = 0;
  VirtualMemoryTable[Index].VirtualBase     = 0;
  VirtualMemoryTable[Index].Length          = 0;
  VirtualMemoryTable[Index].Attributes      = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT((Index + 1) <= MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
}
