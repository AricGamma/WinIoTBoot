/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Jerry Wang <wangflord@allwinnertech.com>
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
#include <Library/SysConfigLib.h>
#include <Sunxi_type/Sunxi_type.h>
//#include <Sunxi_type/list.h>
#include <Sunxi_type/PrivateDef.h>




/*
 * The ALLOC_CACHE_ALIGN_BUFFER macro is used to allocate a buffer on the
 * stack that meets the minimum architecture alignment requirements for DMA.
 * Such a buffer is useful for DMA operations where flushing and invalidating
 * the cache before and after a read and/or write operation is required for
 * correct operations.
 *
 * When called the macro creates an array on the stack that is sized such
 * that:
 *
 * 1) The beginning of the array can be advanced enough to be aligned.
 *
 * 2) The size of the aligned portion of the array is a multiple of the minimum
 *    architecture alignment required for DMA.
 *
 * 3) The aligned portion contains enough space for the original number of
 *    elements requested.
 *
 * The macro then creates a pointer to the aligned portion of this array and
 * assigns to the pointer the address of the first element in the aligned
 * portion of the array.
 *
 * Calling the macro as:
 *
 *     ALLOC_CACHE_ALIGN_BUFFER(uint32_t, buffer, 1024);
 *
 * Will result in something similar to saying:
 *
 *     uint32_t    buffer[1024];
 *
 * The following differences exist:
 *
 * 1) The resulting buffer is guaranteed to be aligned to the value of
 *    ARCH_DMA_MINALIGN.
 *
 * 2) The buffer variable created by the macro is a pointer to the specified
 *    type, and NOT an array of the specified type.  This can be very important
 *    if you want the address of the buffer, which you probably do, to pass it
 *    to the DMA hardware.  The value of &buffer is different in the two cases.
 *    In the macro case it will be the address of the pointer, not the address
 *    of the space reserved for the buffer.  However, in the second case it
 *    would be the address of the buffer.  So if you are replacing hard coded
 *    stack buffers with this macro you need to make sure you remove the & from
 *    the locations where you are taking the address of the buffer.
 *
 * Note that the size parameter is the number of array elements to allocate,
 * not the number of bytes.
 *
 * This macro can not be used outside of function scope, or for the creation
 * of a function scoped static buffer.  It can not be used to create a cache
 * line aligned global buffer.
 */

#define ROUND(a,b)    (((a) + (b) - 1) & ~((b) - 1))
#define DIV_ROUND(n,d)    (((n) + ((d)/2)) / (d))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define roundup(x, y)   ((((x) + ((y) - 1)) / (y)) * (y))


#define PAD_COUNT(s, pad) (((s) - 1) / (pad) + 1)
#define PAD_SIZE(s, pad) (PAD_COUNT(s, pad) * pad)
#define ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, pad)    \
  char __##name[ROUND(PAD_SIZE((size) * sizeof(type), pad), align)  \
          + (align - 1)];         \
                  \
  type *name = (type *) ALIGN((uintptr_t)__##name, align)
#define ALLOC_ALIGN_BUFFER(type, name, size, align)   \
  ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, 1)
#define ALLOC_CACHE_ALIGN_BUFFER_PAD(type, name, size, pad)   \
  ALLOC_ALIGN_BUFFER_PAD(type, name, size, ARCH_DMA_MINALIGN, pad)
#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size)      \
  ALLOC_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

/*
 * DEFINE_CACHE_ALIGN_BUFFER() is similar to ALLOC_CACHE_ALIGN_BUFFER, but it's
 * purpose is to allow allocating aligned buffers outside of function scope.
 * Usage of this macro shall be avoided or used with extreme care!
 */
#define DEFINE_ALIGN_BUFFER(type, name, size, align)      \
  static char __##name[roundup(size * sizeof(type), align)] \
      __aligned(align);       \
                  \
  static type *name = (type *)__##name
#define DEFINE_CACHE_ALIGN_BUFFER(type, name, size)     \
  DEFINE_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)


#define ALIGN(x,a)    __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)  (((x)+(mask))&~(mask))
#define __weak        __attribute__((weak))

#define ARCH_DMA_MINALIGN 64

#define ___swab32(x) \
  ((__u32)( \
    (((__u32)(x) & (__u32)0x000000ffUL) << 24) | \
    (((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
    (((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
    (((__u32)(x) & (__u32)0xff000000UL) >> 24) ))

#define __be32_to_cpu(x) ___swab32((u32)(x))

#define LOG2(x) (((x & 0xaaaaaaaa) ? 1 : 0) + ((x & 0xcccccccc) ? 2 : 0) + \
     ((x & 0xf0f0f0f0) ? 4 : 0) + ((x & 0xff00ff00) ? 8 : 0) + \
     ((x & 0xffff0000) ? 16 : 0))


//#define SUNXI_MMCDBG

#ifdef SUNXI_MMCDBG
#define MMCINFO(fmt...) AsciiPrint("[mmc info]: "fmt)//err and info
#define MMCDBG(fmt...)  AsciiPrint("[mmc dbg]: "fmt)//dbg
#define MMCPRINT(fmt...)  AsciiPrint(fmt)//data or register and so on
#else
#define MMCINFO(fmt...) AsciiPrint("[mmc info]: "fmt)
#define MMCDBG(fmt...)
#define MMCPRINT(fmt...)
#endif

#define mdelay(ms) MicroSecondDelay((ms)*1000)
#define udelay(us) MicroSecondDelay(us)

/* 刷新标记位 */
#define  CACHE_FLUSH_I_CACHE_REGION                             0  /* 清除I-cache中代表主存中一块区域的cache行                  */
#define  CACHE_FLUSH_D_CACHE_REGION                             1  /* 清除D-cache中代表主存中一块区域的cache行                  */
#define  CACHE_FLUSH_CACHE_REGION                               2  /* 清除D-cache和I-cache中代表主存中一块区域的cache行 */
#define  CACHE_CLEAN_D_CACHE_REGION                             3  /* 清理D-cache中代表主存中一块区域的cache行                  */
#define  CACHE_CLEAN_FLUSH_D_CACHE_REGION               4  /* 清理并清除D-cache中代表主存中一块区域的cache行    */
#define  CACHE_CLEAN_FLUSH_CACHE_REGION                 5  /* 清理并清除D-cache，接下来解除I-cache                              */

extern void OSAL_CacheRangeFlush(void*Address, UINT32 Length, UINT32 Flags);


#define MMC_MSG_EN  (1U)
#define MMCMSG(d, fmt, args...) do {if ((d)->msglevel & MMC_MSG_EN)  AsciiPrint("[mmc]: "fmt,##args); } while(0)
#define DRIVER_VER  "2016-01-27 19:51:00"


//secure storage relate
#define MAX_SECURE_STORAGE_MAX_ITEM             32
#define SDMMC_SECURE_STORAGE_START_ADD  (6*1024*1024/512)//6M
#define SDMMC_ITEM_SIZE                                 (4*1024/512)//4K
#define MAX_MMC_NUM                    4
//#define UEFI_SDMMC_START_ADDR                   (38192)
#define UEFI_SDMMC_START_ADDR                   (512)


#endif
