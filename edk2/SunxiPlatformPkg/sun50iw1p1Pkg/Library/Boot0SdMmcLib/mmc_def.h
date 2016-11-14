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
#ifndef _MMC_DEF_H_
#define _MMC_DEF_H_

#include <Sun50iW1P1/ccmu.h>
#include <Sun50iW1P1/archdef.h>
#include <boot0_include/boot0_helper.h>

//#include <Library/DebugLib.h>

#define SUNXI_MMC0_BASE       (SUNXI_SMHC0_BASE)
#define SUNXI_MMC1_BASE       (SUNXI_SMHC1_BASE)
#define SUNXI_MMC2_BASE       (SUNXI_SMHC2_BASE)


#define MAX_MMC_NUM     3

#define MMC_TRANS_BY_DMA
//#define MMC_DEBUG
#define MMC_REG_FIFO_OS   (0x200)

#define MMC_REG_BASE    SUNXI_MMC0_BASE
#define CCMU_HCLKGATE0_BASE CCMU_BUS_CLK_GATING_REG0
#define CCMU_HCLKRST0_BASE  CCMU_BUS_SOFT_RST_REG0
#define CCMU_MMC0_CLK_BASE  CCMU_SDMMC0_CLK_REG
#define CCMU_MMC2_CLK_BASE  CCMU_SDMMC2_CLK_REG


//#define CCMU_PLL5_CLK_BASE  0x01c20020
#define __mmc_be32_to_cpu(x)  ((0x000000ff&((x)>>24)) | (0x0000ff00&((x)>>8)) |       \
               (0x00ff0000&((x)<< 8)) | (0xff000000&((x)<<24)))

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


#ifndef NULL
#define NULL (void*)0
#endif

#if 1
#ifdef MMC_DEBUG
#define mmcinfo(fmt...) printf("[mmc]: "fmt)
#define mmcdbg(fmt...)  printf("[mmc]: "fmt)
#define mmcmsg(fmt...)  printf(fmt)
#else
#define mmcinfo(fmt...) printf("[mmc]: "fmt)
#define mmcdbg(fmt...)
#define mmcmsg(fmt...)
#endif
#else
#ifdef MMC_DEBUG
#define mmcinfo(fmt...) DEBUG((EFI_D_INIT,"[mmc]: "fmt));
#define mmcdbg(fmt...)  DEBUG((EFI_D_INIT,"[mmc]: "fmt));
#define mmcmsg(fmt...)  DEBUG((EFI_D_INIT,fmt));
#else
#define mmcinfo(fmt...) DEBUG((EFI_D_INIT,"[mmc]: "fmt));
#define mmcdbg(fmt...)
#define mmcmsg(fmt...)
#endif
#endif

#define DMAC_DES_BASE_IN_SRAM   (0x20000 + 0xC000)
#define DMAC_DES_BASE_IN_SDRAM    (0x42000000)
#define DRAM_START_ADDR       (0x40000000)


#define DRIVER_VER  "2015-09-24 15:58"

#endif /* _MMC_H_ */
