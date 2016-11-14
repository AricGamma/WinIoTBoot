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

#ifndef __SUNXI_VARIABLE_GUID_H__
#define __SUNXI_VARIABLE_GUID_H__

#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>

#define SUNXI_HOB_VARIABLE_GUID  \
  { 0x646c7865, 0x722e, 0xcaf8, { 0xfa, 0x3d, 0x04, 0xbd, 0xa9, 0xae, 0xaa, 0x78 } };
  


extern EFI_GUID gSunxiVariableGuid;

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

  ///
  /// The base address of memory allocated by this HOB. Type
  /// EFI_PHYSICAL_ADDRESS is defined in AllocatePages() in the UEFI 2.0
  /// specification.
  ///
  EFI_PHYSICAL_ADDRESS        SunxiVariableBase;

  /// 
  /// The length in bytes of memory allocated by this HOB.
  /// 
  UINT32                      SunxiVariableSize;

  //buffer for sunxi env partition
  EFI_PHYSICAL_ADDRESS        SunxiEnvBase;
  UINT32                      SunxiEnvSize;
  UINT32                      SunxiEnvNeedImport;
  
} SUNXI_VARIABLE_HOB;

#endif
