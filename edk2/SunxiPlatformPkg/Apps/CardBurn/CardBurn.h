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

#ifndef  __CARD_BURN_H__
#define  __CARD_BURN_H__

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Sunxi_type/Sunxi_type.h>
#include <Library/SysConfigLib.h>
#include <Library/SunxiMbr.h>
#include <Library/SunxiPartitionLib.h>
#include <Library/SunxiCheckLib.h>
#include <Library/SunxiBootInfoLib.h>
#include <Library/SunxiSpriteHelperLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "SunxiSprite.h"
#include "sprite_cartoon.h"


//extern struct spare_boot_head_t* uboot_spare_head;


#if(CARDBURN_DRIVER_DEBUG_ENABLE)
  #define debug(ARG...)  DEBUG (( EFI_D_INFO,"[cardburn Info]:"ARG))
#else
  #define debug(ARG...)  
#endif

#define printf(ARG...)        AsciiPrint(ARG)
#define tick_printf(ARG...)   AsciiPrint(ARG)
#define sprite_uichar_printf(ARG...)   AsciiPrint(ARG)

#define memcpy(x,y,z)         CopyMem((x),(y),(z))
#define memcmp(x,y,z)         CompareMem((x),(y),(z))
//note : memset & SetMem para order are not the same
#define memset(x,y,z)         SetMem((x),(z),(y))


#define malloc(x)             AllocateZeroPool((x))
#define free(x)               FreePool((x))


//string 
#define strcmp(x,y)           AsciiStrCmp((x),(y))
#define strcpy(x,y)           AsciiStrCpy((x),(y))
#define strlen(x)             AsciiStrLen((x))
#define strncmp(x,y,z)        AsciiStrnCmp((x),(y),(z))

#define  STAMP_VALUE                      0x5F0A6C39
#define  FW_BURN_UDISK_MIN_SIZE         (2 * 1024)
#define  DRAM_PARA_STORE_ADDR             ((UINTN)(PcdGet64 (PcdSystemMemoryBase) + 0x00800000))

void SpriteDelayMS(UINT32 ms);
void SpriteDelayUS(UINT32 us);
int sunxi_board_restart(int next_mode);

#define mmc_write_info(x,y,z)
#endif
