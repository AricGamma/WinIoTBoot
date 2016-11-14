/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Martin.Zheng <martinzheng@allwinnertech.com>
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

#ifndef __MMC_DEF__
#define __MMC_DEF__

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DmaLib.h>

typedef unsigned long ulong;
typedef UINT32 uint;
typedef UINT8  u8;
typedef UINT16 u16;
typedef UINT32 u32;
typedef UINT64 u64;
typedef INT32  s32;

#define __be32_to_cpu(x)  ((0x000000ff&((x)>>24)) | (0x0000ff00&((x)>>8)) |       \
               (0x00ff0000&((x)<< 8)) | (0xff000000&((x)<<24)))

#define readl(addr)         (*((volatile UINT32 *)(addr)))          /* word input */
#define writel(value,addr)  (*((volatile UINT32 *)(addr))  = (value))   /* word output */

#define BOOT0_SDMMC_START_ADDR                  (16)
#define UEFI_SDMMC_START_ADDR                   (38192)
#define MMC_LOGICAL_OFFSET            (20 * 1024 * 1024/512)

#define MAX_MMC_NUM     4
#define MMC_TRANS_BY_DMA
//#define MMC_DMA_MAP
//#define MMC_DEBUG
#define USE_EMMC_BOOT_PART

#ifdef SUNXI_MMCDBG
#define MMCINFO(fmt...) AsciiPrint("[mmc]: "fmt)//err and info
#define MMCDBG(fmt...)  AsciiPrint("[mmc]: "fmt)//dbg
#define MMCMSG(fmt...)  AsciiPrint(fmt)//data or register and so on
#else
#define MMCINFO(fmt...) AsciiPrint("[mmc]: "fmt)
#define MMCDBG(fmt...)
#define MMCMSG(fmt...)
#endif

/* 刷新标记位 */
#define  CACHE_FLUSH_I_CACHE_REGION       0  /* 清除I-cache中代表主存中一块区域的cache行      */
#define  CACHE_FLUSH_D_CACHE_REGION       1  /* 清除D-cache中代表主存中一块区域的cache行      */
#define  CACHE_FLUSH_CACHE_REGION       2  /* 清除D-cache和I-cache中代表主存中一块区域的cache行 */
#define  CACHE_CLEAN_D_CACHE_REGION       3  /* 清理D-cache中代表主存中一块区域的cache行      */
#define  CACHE_CLEAN_FLUSH_D_CACHE_REGION   4  /* 清理并清除D-cache中代表主存中一块区域的cache行  */
#define  CACHE_CLEAN_FLUSH_CACHE_REGION     5  /* 清理并清除D-cache，接下来解除I-cache        */

extern void OSAL_CacheRangeFlush(void*Address, UINT32 Length, UINT32 Flags);

#define DRIVER_VER  "2014-06-05 14:18:30"


#endif
