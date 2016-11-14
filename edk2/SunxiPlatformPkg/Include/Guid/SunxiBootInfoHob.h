/** @file
*
*  Copyright (c) 2011-2013, ARM Limited. All rights reserved.
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

#ifndef __SUNXI_BOOTINFO_GUID_H__
#define __SUNXI_BOOTINFO_GUID_H__

#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>


#define SUNXI_HOB_BOOTINFO_GUID  \
  { 0x76BECAC8, 0x6A8B, 0x5CBA, { 0x4c, 0x1f, 0xf0, 0xef, 0xe1, 0xea, 0x1b, 0xff } };
  


extern EFI_GUID gSunxiBootInfoHobGuid;

///
/// Describes all memory ranges used during the HOB producer 
/// phase that exist outside the HOB list. This HOB type 
/// describes how memory is used, not the physical attributes of memory.
///
typedef struct {
  ///
  /// The Guid HOB header. Header.HobType = EFI_HOB_TYPE_GUID_EXTENSION
  ///                 and  Header.Name    = gArmGlobalVariableGuid
  ///
  EFI_HOB_GUID_TYPE            Header;

  UINT32                       StorageType;
  UINT32                       WorkMode;
  UINT32                       BurnStorageType;
  UINT64                       FrameBufferBase;
  UINT64                       FrameBufferSize;
  UINT64                       SystemMemoryBase;
  UINT64                       SystemMemorySize;
  UINT64                       ParkSharedBase;
  UINT64                       ParkSharedSize;
  UINT32                       DramPara[32];
  
} SUNXI_BOOTINFO_HOB;

#endif
