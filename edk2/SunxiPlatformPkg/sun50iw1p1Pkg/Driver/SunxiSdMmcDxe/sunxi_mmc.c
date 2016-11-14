/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Aaron <leafy.myeh@allwinnertech.com>
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


#include "mmc_def.h"
#include "mmc.h"
#include "sunxi_mmc.h"
#include "sunxi_host_mmc.h"

#ifdef SUNXI_MMCDBG

void dumphex32(char* name, char* base, int len)
{
  __u32 i;

  MMCPRINT("dump %a registers:", name);
  for (i=0; i<len; i+=4) {
    if (!(i&0xf))
      MMCPRINT("\n0x%p : ", base + i);
    MMCPRINT("0x%08x ", readl((ulong)base + i));
  }
  MMCPRINT("\n");
}

/*
static void dumpmmcreg(struct sunxi_mmc *reg)
{
  printf("dump mmc registers:\n");
  printf("gctrl     0x%08x\n", reg->gctrl     );
  printf("clkcr     0x%08x\n", reg->clkcr     );
  printf("timeout   0x%08x\n", reg->timeout   );
  printf("width     0x%08x\n", reg->width     );
  printf("blksz     0x%08x\n", reg->blksz     );
  printf("bytecnt   0x%08x\n", reg->bytecnt   );
  printf("cmd       0x%08x\n", reg->cmd       );
  printf("arg       0x%08x\n", reg->arg       );
  printf("resp0     0x%08x\n", reg->resp0     );
  printf("resp1     0x%08x\n", reg->resp1     );
  printf("resp2     0x%08x\n", reg->resp2     );
  printf("resp3     0x%08x\n", reg->resp3     );
  printf("imask     0x%08x\n", reg->imask     );
  printf("mint      0x%08x\n", reg->mint      );
  printf("rint      0x%08x\n", reg->rint      );
  printf("status    0x%08x\n", reg->status    );
  printf("ftrglevel 0x%08x\n", reg->ftrglevel );
  printf("funcsel   0x%08x\n", reg->funcsel   );
  printf("dmac      0x%08x\n", reg->dmac      );
  printf("dlba      0x%08x\n", reg->dlba      );
  printf("idst      0x%08x\n", reg->idst      );
  printf("idie      0x%08x\n", reg->idie      );
  printf("cbcr      0x%08x\n", reg->cbcr      );
  printf("bbcr      0x%08x\n", reg->bbcr      );
}
*/
#else
//#define MMCDBG(fmt...)
//#define dumpmmcreg(fmt...)
//#define  dumphex32(fmt...)
void dumphex32(char* name, char* base, int len) {};

#endif /* SUNXI_MMCDBG */

#define MMC_CLK_400K      0
#define MMC_CLK_25M     1
#define MMC_CLK_50M     2
#define MMC_CLK_50MDDR      3
#define MMC_CLK_50MDDR_8BIT   4
#define MMC_CLK_100M      5
#define MMC_CLK_200M      6
#define MMC_CLK_MOD_NUM     7

int sunxi_mmc_init(int sdc_no)
{
  return sunxi_sun50iw1p1_mmc_init(sdc_no);
}

int sunxi_mmc_exit(int sdc_no)
{
  return sunxi_sun50iw1p1_mmc_exit(sdc_no);
}


