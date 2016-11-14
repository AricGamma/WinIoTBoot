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

#ifndef __SUNXI_SCRIPT_PARSE_HOB_GUID_H__
#define __SUNXI_SCRIPT_PARSE_HOB_GUID_H__

#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>

#define SUNXI_SCRIPT_PARSE_HOB_GUID  \
  { 0x5f5f3e33, 0xeb9c, 0x475d, { 0xb0, 0x67, 0x87, 0xe6, 0xd5, 0x2a, 0x95, 0x45 } };

extern EFI_GUID gSunxiScriptParseHobGuid;

///
/// Describes all memory ranges used during the HOB producer 
/// phase that exist outside the HOB list. This HOB type 
/// describes how memory is used, not the physical attributes of memory.
///
typedef struct {
  ///
  /// The Guid HOB header. Header.HobType = EFI_HOB_TYPE_GUID_EXTENSION
  ///                 and  Header.Name    = gSunxiScriptParseGuid
  ///
  EFI_HOB_GUID_TYPE            Header;

  ///
  /// The base address of memory allocated by this HOB. Type
  /// EFI_PHYSICAL_ADDRESS is defined in AllocatePages() in the UEFI 2.0
  /// specification.
  ///
  EFI_PHYSICAL_ADDRESS        SunxiScriptParseBase;

  /// 
  /// The length in bytes of memory allocated by this HOB.
  /// 
  UINT32                      SunxiScriptParseSize;
} SUNXI_SCRIPT_PARSE_HOB;

#endif
