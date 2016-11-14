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
#include <Library/SysConfigLib.h>

#include "string.h"
#include "mmc_def.h"
#include "mmc.h"

#include "cpu.h"
#include "clock.h"
#include "ccmu.h"

//#define SUNXI_MMCDBG
//#undef SUNXI_MMCDBG
//#define MMCINFO(fmt...) printf("[mmc]: "fmt)
#ifndef CONFIG_ARCH_SUN7I
#define MMC_REG_FIFO_OS   (0x200)
#else
#define MMC_REG_FIFO_OS   (0x100)
#endif

#ifdef SUNXI_MMCDBG
//#define MMCDBG(fmt...)  printf("[mmc]: "fmt)

static void dumphex32(char* name, char* base, int len)
{
  __u32 i;

  MMCMSG("dump %s registers:", name);
  for (i=0; i<len; i+=4) {
    if (!(i&0xf))
      MMCMSG("\n0x%p : ", base + i);
    MMCMSG("0x%08x ", readl(base + i));
  }
  MMCMSG("\n");
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
#define dumpmmcreg(fmt...)
#define  dumphex32(fmt...)
#endif /* SUNXI_MMCDBG */

#define MMC_CLK_400K          0
#define MMC_CLK_25M           1
#define MMC_CLK_50M           2
#define MMC_CLK_50MDDR        3
#define MMC_CLK_50MDDR_8BIT   4
#define MMC_CLK_100M          5
#define MMC_CLK_200M          6
#define MMC_CLK_MOD_NUM       7

struct sunxi_mmc_clk_dly {
  u32 mode;
  u32 oclk_dly;
  u32 sclk_dly;
};

struct sunxi_mmc {
  volatile    u32 gctrl;         /* (0x00) SMC Global Control Register */
  volatile  u32 clkcr;         /* (0x04) SMC Clock Control Register */
  volatile  u32 timeout;       /* (0x08) SMC Time Out Register */
  volatile  u32 width;         /* (0x0C) SMC Bus Width Register */
  volatile  u32 blksz;         /* (0x10) SMC Block Size Register */
  volatile  u32 bytecnt;       /* (0x14) SMC Byte Count Register */
  volatile  u32 cmd;           /* (0x18) SMC Command Register */
  volatile  u32 arg;           /* (0x1C) SMC Argument Register */
  volatile  u32 resp0;         /* (0x20) SMC Response Register 0 */
  volatile  u32 resp1;         /* (0x24) SMC Response Register 1 */
  volatile  u32 resp2;         /* (0x28) SMC Response Register 2 */
  volatile  u32 resp3;         /* (0x2C) SMC Response Register 3 */
  volatile  u32 imask;         /* (0x30) SMC Interrupt Mask Register */
  volatile  u32 mint;          /* (0x34) SMC Masked Interrupt Status Register */
  volatile  u32 rint;          /* (0x38) SMC Raw Interrupt Status Register */
  volatile  u32 status;        /* (0x3C) SMC Status Register */
  volatile  u32 ftrglevel;     /* (0x40) SMC FIFO Threshold Watermark Register */
  volatile  u32 funcsel;       /* (0x44) SMC Function Select Register */
  volatile  u32 cbcr;          /* (0x48) SMC CIU Byte Count Register */
  volatile  u32 bbcr;          /* (0x4C) SMC BIU Byte Count Register */
  volatile  u32 dbgc;          /* (0x50) SMC Debug Enable Register */
  volatile  u32 res0[9];       /* (0x54~0x74) */
  volatile  u32 hwrst;         /* (0x78) SMC eMMC Hardware Reset Register */
  volatile  u32 res1;          /* (0x7c) */
  volatile  u32 dmac;          /* (0x80) SMC IDMAC Control Register */
  volatile  u32 dlba;          /* (0x84) SMC IDMAC Descriptor List Base Address Register */
  volatile  u32 idst;          /* (0x88) SMC IDMAC Status Register */
  volatile  u32 idie;          /* (0x8C) SMC IDMAC Interrupt Enable Register */
  volatile  u32 chda;          /* (0x90) */
  volatile  u32 cbda;          /* (0x94) */
  volatile  u32 res2[26];      /* (0x98~0xff) */
  volatile  u32 fifo;          /* (0x100) SMC FIFO Access Address */
};

struct sunxi_mmc_des {
  u32     :1,
  dic   :1, /* disable interrupt on completion */
  last_des  :1, /* 1-this data buffer is the last buffer */
  first_des :1, /* 1-data buffer is the first buffer,
             0-data buffer contained in the next descriptor is 1st buffer */
  des_chain :1, /* 1-the 2nd address in the descriptor is the next descriptor address */
  end_of_ring :1, /* 1-last descriptor flag when using dual data buffer in descriptor */
        :24,
  card_err_sum  :1, /* transfer error flag */
  own   :1; /* des owner:1-idma owns it, 0-host owns it */
#if defined CONFIG_SUN4I
#define SDXC_DES_NUM_SHIFT 12
#define SDXC_DES_BUFFER_MAX_LEN (1 << SDXC_DES_NUM_SHIFT)
  u32 data_buf1_sz  :13,
    data_buf2_sz  :13,
            :6;
#elif defined CONFIG_ARCH_SUN7I
#define SDXC_DES_NUM_SHIFT 15
#define SDXC_DES_BUFFER_MAX_LEN (1 << SDXC_DES_NUM_SHIFT)
  u32 data_buf1_sz  :16,
    data_buf2_sz  :16;
#else
#define SDXC_DES_NUM_SHIFT 15
#define SDXC_DES_BUFFER_MAX_LEN (1 << SDXC_DES_NUM_SHIFT)
  u32 data_buf1_sz  :16,
    data_buf2_sz  :16;
#endif
  u32 buf_addr_ptr1;
  u32 buf_addr_ptr2;
};

struct sunxi_mmc_host {
  unsigned mmc_no;
  unsigned hclkbase;
#ifndef CONFIG_ARCH_SUN7I
  unsigned hclkrst;
#endif
  unsigned mclkbase;
  unsigned database;
#if defined(CONFIG_ARCH_SUN9IW1P1)
  unsigned commreg;
#endif
  unsigned fatal_err;
  unsigned mod_clk;
  struct sunxi_mmc *reg;
  struct sunxi_mmc_des* pdes;
  /*sample delay and output deley setting*/
  struct sunxi_mmc_clk_dly mmc_clk_dly[MMC_CLK_MOD_NUM];
};

/* support 4 mmc hosts */
struct mmc mmc_dev[4];
struct sunxi_mmc_host mmc_host[4];

static int mmc_resource_init(int sdc_no)
{
  struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];
  MMCDBG("init mmc %d resource\n", sdc_no);
  switch (sdc_no) {
    case 0:
      mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC0_BASE;
      mmchost->mclkbase = CCM_SDC0_SCLK_CTRL;
      break;
    case 1:
      mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC1_BASE;
      mmchost->mclkbase = CCM_SDC1_SCLK_CTRL;
      break;
    case 2:
      mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC2_BASE;
      mmchost->mclkbase = CCM_SDC2_SCLK_CTRL;
      break;
    case 3:
      mmchost->reg = (struct sunxi_mmc *)SUNXI_MMC3_BASE;
      mmchost->mclkbase = CCM_SDC3_SCLK_CTRL;
      break;
    default:
      MMCINFO("Wrong mmc number %d\n", sdc_no);
      break;
  }
#ifdef CONFIG_ARCH_SUN9IW1P1
  mmchost->hclkbase = CCM_AHB0_GATE0_CTRL;
  mmchost->hclkrst  = CCM_AHB0_RST_REG0;
  mmchost->commreg  = SUNXI_MMC_COMMON_BASE + sdc_no*4;
#elif defined CONFIG_ARCH_SUN7I
  mmchost->hclkbase = CCM_AHB_GATE0_CTRL;
#else
  mmchost->hclkbase = CCM_AHB1_GATE0_CTRL;
  mmchost->hclkrst  = CCM_AHB1_RST_REG0;
#endif
  mmchost->database = (unsigned int)mmchost->reg + MMC_REG_FIFO_OS;
  mmchost->mmc_no = sdc_no;

#ifdef CONFIG_ARCH_SUN9IW1P1
  mmchost->mmc_clk_dly[MMC_CLK_25M].mode      = MMC_CLK_25M;
  mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly  = 0;
  mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly  = 5;
  
  mmchost->mmc_clk_dly[MMC_CLK_50M].mode      = MMC_CLK_50M;
  mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly  = 5;
  mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly  = 4;
#else
  mmchost->mmc_clk_dly[MMC_CLK_25M].mode      = MMC_CLK_25M;
  mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly  = 0;
  mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly  = 5;
  
  mmchost->mmc_clk_dly[MMC_CLK_50M].mode      = MMC_CLK_50M;
  mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly  = 3;
  mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly  = 4;
#endif

  return 0;
}

static int mmc_clk_io_on(int sdc_no)
{
  int rval;
  int ret = 0;
  struct sunxi_mmc_host* mmchost = &mmc_host[sdc_no];

  if(sdc_no == 0)
  {
    gpio_request_simple("card0_boot_para", NULL);
    /*************************25M dly*************************************/
    ret = script_parser_fetch("card0_boot_para","sdc_odly_25M", &rval, 1);
    if(ret < 0)
      MMCINFO("get sdc_odly_25M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
    else{
      if((rval>7)||(rval<0)){
        MMCINFO("input sdc_odly_25M wrong ,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
      }else{      
        mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly = rval;
        MMCINFO("get sdc_odly_25M ok, odly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
      }
    }
    
    ret = script_parser_fetch("card0_boot_para","sdc_sdly_25M", &rval, 1);
    if(ret < 0)
      MMCINFO("get sdc_sdly_25M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
    else{
      if((rval>7)||(rval<0)){
        MMCINFO("input sdc_sdly_25M wrong ,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
      }else{
        mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly = rval;
        MMCINFO("get sdc_sdly_25M ok, sdly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
      }
    }

      /*************************50M dly*************************************/
    ret = script_parser_fetch("card0_boot_para","sdc_odly_50M", &rval, 1);
    if(ret < 0)
        MMCINFO("get sdc_odly_50M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
    else{
      if((rval>7)||(rval<0)){
        MMCINFO("input sdc_odly_50M wrong, use default dly %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
      }else{    
        mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly = rval;
        MMCINFO("get sdc_odly_50M ok, odly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
      }
    }
    
    ret = script_parser_fetch("card0_boot_para","sdc_sdly_50M", &rval, 1);
    if(ret < 0)
        MMCINFO("get sdc_sdly_50M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
    else{
      if((rval>7)||(rval<0)){
        MMCINFO("input sdc_sdly_50M wrong, use default dly %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
      }else{
        mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly = rval;
        MMCINFO("get sdc_sdly_50M ok, sdly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
      }
    }
  
    ret = script_parser_fetch("card0_boot_para","sdc_f_max", &rval, 1);
    if(ret < 0)
      MMCINFO("get sdc_f_max fail,use default sdc_f_max %d\n",mmc_dev[sdc_no].f_max);
    else{
      if((rval>mmc_dev[sdc_no].f_max)||(rval<mmc_dev[sdc_no].f_min)){
        MMCINFO("input sdc_f_max wrong ,use default sdc_f_max %d\n",mmc_dev[sdc_no].f_max);
      }else{
        mmc_dev[sdc_no].f_max = rval;
        MMCINFO("get sdc_f_max ok, sdc_f_max = %d\n", mmc_dev[sdc_no].f_max);
      }
    }
  
  }
  else // if(sdc_no == 2)
  {
    gpio_request_simple("card2_boot_para", NULL);
    
    /*************************25M dly*************************************/
    ret = script_parser_fetch("card2_boot_para","sdc_odly_25M", &rval, 1);
    if(ret < 0)
      MMCINFO("get sdc_odly_25M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
    else{
      if((rval>7)||(rval<0)){
        MMCINFO("input sdc_odly_25M wrong ,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
      }else{      
        mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly = rval;
        MMCINFO("get sdc_odly_25M ok, odly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly);
      }
    }
    
    ret = script_parser_fetch("card2_boot_para","sdc_sdly_25M", &rval, 1);
    if(ret < 0)
      MMCINFO("get sdc_sdly_25M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
    else{
      if((rval>7)||(rval<0)){
        MMCINFO("input sdc_sdly_25M wrong ,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
      }else{
        mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly = rval;
        MMCINFO("get sdc_sdly_25M ok, sdly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly);
      }
    }

    /*************************50M dly*************************************/
    ret = script_parser_fetch("card2_boot_para","sdc_odly_50M", &rval, 1);
    if(ret < 0)
      MMCINFO("get sdc_odly_50M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
    else{
      if((rval>7)||(rval<0)){
        MMCINFO("input sdc_odly_50M wrong, use default dly %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
      }else{    
        mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly = rval;
        MMCINFO("get sdc_odly_50M ok, odly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly);
      }
    }
    
    ret = script_parser_fetch("card2_boot_para","sdc_sdly_50M", &rval, 1);
    if(ret < 0)
      MMCINFO("get sdc_sdly_50M fail,use default dly %d\n",mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
    else{
      if((rval>7)||(rval<0)){
        MMCINFO("input sdc_sdly_50M wrong, use default dly %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
      }else{
        mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly = rval;
        MMCINFO("get sdc_sdly_50M ok, sdly = %d\n", mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly);
      }
    }
    
    ret = script_parser_fetch("card2_boot_para","sdc_f_max", &rval, 1);
    if(ret < 0)
        MMCINFO("get sdc_f_max fail,use default sdc_f_max %d\n",mmc_dev[sdc_no].f_max);
    else{
      if((rval>mmc_dev[sdc_no].f_max)||(rval<mmc_dev[sdc_no].f_min)){
        MMCINFO("input sdc_f_max wrong ,use default sdc_f_max %d\n",mmc_dev[sdc_no].f_max);
      }else{
        mmc_dev[sdc_no].f_max = rval;
        MMCINFO("get sdc_f_max ok, sdc_f_max = %d\n", mmc_dev[sdc_no].f_max);
      }
    }
    
  }
#if defined(CONFIG_ARCH_SUN8IW1P1) || defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW6P1) ||(defined CONFIG_ARCH_SUN8IW7P1) || defined(CONFIG_ARCH_SUN8IW8P1)
  /* config ahb clock */
  rval = readl(mmchost->hclkbase);
  rval |= (1 << (8 + sdc_no));
  writel(rval, mmchost->hclkbase);

  rval = readl(mmchost->hclkrst);
  rval |= (1 << (8 + sdc_no));
  writel(rval, mmchost->hclkrst);
#elif defined(CONFIG_ARCH_SUN7I) || defined(CONFIG_ARCH_SUN5I)
  /* config ahb clock */
  rval = readl(mmchost->hclkbase);
  rval |= (1 << (8 + sdc_no));
  writel(rval, mmchost->hclkbase);
#elif defined(CONFIG_ARCH_SUN9IW1P1)
  /* config ahb clock */
  rval = readl(mmchost->hclkbase);
  rval |= (1 << 8);
  writel(rval, mmchost->hclkbase);

  rval = readl(mmchost->hclkrst);
  rval |= (1 << 8);
  writel(rval, mmchost->hclkrst);

  rval = readl(mmchost->commreg);
  rval |= (1<<16)|(1<<18);
  writel(rval, mmchost->commreg);
#else
  #error The platform is not seleted
#endif
  /* config mod clock */
  writel(0x80000000, mmchost->mclkbase);
  mmchost->mod_clk = 24000000;
  dumphex32("ccmu", (char*)SUNXI_CCM_BASE, 0x100);
  dumphex32("gpio", (char*)SUNXI_PIO_BASE, 0x100);
  dumphex32("mmc", (char*)mmchost->reg, 0x100);

  return 0;
}

static int mmc_update_clk(struct mmc *mmc)
{
  struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
  unsigned int cmd;
  unsigned timeout = 1000;

  cmd = (1U << 31) | (1 << 21) | (1 << 13);
  writel(cmd, &mmchost->reg->cmd);
  while((readl(&mmchost->reg->cmd)&0x80000000) && --timeout){
    MicroSecondDelay(1000);
  }
  if (!timeout){
    MMCINFO("mmc %d,update clk failed\n",mmchost->mmc_no);
    dumphex32("mmc", (char*)mmchost->reg, 0x100);
    return -1;
  }

  writel(readl(&mmchost->reg->rint), &mmchost->reg->rint);
  return 0;
}

static int mmc_config_clock(struct mmc *mmc, unsigned clk)
{
  struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
  unsigned rval = readl(&mmchost->reg->clkcr);
  unsigned int clkdiv = 0;
  u32 sdly = 0;
  u32 odly = 0;

  /* Disable Clock */
  rval &= ~(1 << 16);
  writel(rval, &mmchost->reg->clkcr);
  if(mmc_update_clk(mmc))
    return -1;

  //disable mclk first
  writel(0,mmchost->mclkbase);
  MMCDBG("mmc %d mclkbase 0x%x\n",mmchost->mmc_no,readl(mmchost->mclkbase));
  if (clk <=400000) {
    mmchost->mod_clk = 400000;
    writel(0x0002000f, mmchost->mclkbase);
    MMCDBG("mmc %d mclkbase 0x%x\n",mmchost->mmc_no,readl(mmchost->mclkbase));
    MMCDBG("Get round clk %d\n",400000);
    mmc->clock = 400000;
  } else {
    u32 pllclk;
    u32 n,m;
#if (defined(CONFIG_ARCH_SUN7I)|| defined(CONFIG_ARCH_SUN8IW1P1) || defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW6P1) ||(defined CONFIG_ARCH_SUN8IW7P1) ||  defined(CONFIG_ARCH_SUN8IW8P1) )
    pllclk = sunxi_clock_get_pll6() * 1000000;
#elif defined(CONFIG_ARCH_SUN9IW1P1)
    pllclk = sunxi_clock_get_pll4_periph1() * 1000000;
#elif defined(CONFIG_ARCH_SUN5I)
    pllclk = sunxi_clock_get_pll5() * 1000000;

#else
  #error the platform is not config
#endif
    clkdiv = pllclk / clk - 1;
      if (clkdiv < 16) {
      n = 0;
      m = clkdiv;
    } else if (clkdiv < 32) {
      n = 1;
      m = clkdiv>>1;
    } else {
      n = 2;
      m = clkdiv>>2;
    }
    mmchost->mod_clk = clk;
#if defined(CONFIG_ARCH_SUN5I)
    if (clk <= 26000000)
      writel(0x02500000 | (n << 16) | m, mmchost->mclkbase);
    else
      writel(0x02500300 | (n << 16) | m, mmchost->mclkbase);
#else
    if (clk <= 26000000){
      sdly = mmchost->mmc_clk_dly[MMC_CLK_25M].sclk_dly;
      odly = mmchost->mmc_clk_dly[MMC_CLK_25M].oclk_dly;
      writel(0x01000000 |(sdly << 20)|(odly << 8)| (n << 16) | m, mmchost->mclkbase);
    }
    else{
      sdly = mmchost->mmc_clk_dly[MMC_CLK_50M].sclk_dly;
      odly = mmchost->mmc_clk_dly[MMC_CLK_50M].oclk_dly;
      writel(0x01000000 | (sdly << 20) | (odly << 8) | (n << 16) | m, mmchost->mclkbase);
    }
#endif
    MMCDBG("init mmc %d pllclk %d, clk %d, mclkbase %x\n",mmchost->mmc_no,
    pllclk, mmchost->mod_clk, readl(mmchost->mclkbase));
    MMCDBG("Get round clk %d\n",pllclk/(n<<1)/(m+1));
    mmc->clock = pllclk/(n<<1)/(m+1);
  }
  //re-enable mclk
  writel(readl(mmchost->mclkbase)|(1<<31),mmchost->mclkbase);
  MMCDBG("mmc %d mclkbase 0x%x\n",mmchost->mmc_no,readl(mmchost->mclkbase));
  /*
   * CLKCREG[7:0]: divider
   * CLKCREG[16]:  on/off
   * CLKCREG[17]:  power save
   */
  /* Change Divider Factor */
  rval &= ~(0xFF);
  writel(rval, &mmchost->reg->clkcr);
  if(mmc_update_clk(mmc)){
    MMCINFO("mmc %d disable clock failed\n",mmchost->mmc_no);
    return -1;
  }
  /* Re-enable Clock */
  rval |= (3 << 16);
  writel(rval, &mmchost->reg->clkcr);
  if(mmc_update_clk(mmc)){
    MMCINFO("mmc %d re-enable clock failed\n",mmchost->mmc_no);
    return -1;
  }
  
  return 0;
}

static void mmc_set_ios(struct mmc *mmc)
{
  struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;

  MMCDBG("mmc %d ios: bus: %d, clock: %d\n", mmchost->mmc_no,mmc->bus_width, mmc->clock);

  if (mmc->clock && mmc_config_clock(mmc, mmc->clock)) {
    MMCINFO("[mmc]: mmc %d  update clock failed\n",mmchost->mmc_no);
    mmchost->fatal_err = 1;
    return;
  }
  /* Change bus width */
  if (mmc->bus_width == 8)
    writel(2, &mmchost->reg->width);
  else if (mmc->bus_width == 4)
    writel(1, &mmchost->reg->width);
  else
    writel(0, &mmchost->reg->width);
}

static int mmc_core_init(struct mmc *mmc)
{
  struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
#ifndef CONFIG_SUN7I
  /* Reset controller */
  writel(0x40000007, &mmchost->reg->gctrl);
  while(readl(&mmchost->reg->gctrl)&0x7);
  /* release eMMC reset signal */
  writel(1, &mmchost->reg->hwrst);
  writel(0, &mmchost->reg->hwrst);
  MicroSecondDelay(1000);
  writel(1, &mmchost->reg->hwrst);
#else
  writel(0x7, &mmchost->reg->gctrl);
  while(readl(&mmchost->reg->gctrl)&0x7);
#endif
  return 0;
}

static int mmc_trans_data_by_cpu(struct mmc *mmc, struct mmc_data *data)
{
  struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
  unsigned i;
  unsigned byte_cnt = data->blocksize * data->blocks;
  unsigned *buff;
  unsigned timeout = 1000;

  if (data->flags & MMC_DATA_READ) {
    buff = (unsigned int *)data->dest;
    for (i=0; i<(byte_cnt>>2); i++) {
      while(--timeout && (readl(&mmchost->reg->status)&(1 << 2))){
        MicroSecondDelay(1000);
      }
      if (timeout <= 0)
        goto out;
      buff[i] = readl(mmchost->database);
      timeout = 1000;
    }
  } else {
    buff = (unsigned int *)data->src;
    for (i=0; i<(byte_cnt>>2); i++) {
      while(--timeout && (readl(&mmchost->reg->status)&(1 << 3))){
        MicroSecondDelay(1000);
      }
      if (timeout <= 0)
        goto out;
      writel(buff[i], mmchost->database);
      timeout = 1000;
    }
  }

out:
  if (timeout <= 0){
    MMCINFO("transfer by cpu failed\n");
    return -1;
  }

  return 0;
}

static int mmc_trans_data_by_dma(struct mmc *mmc, struct mmc_data *data)
{
  struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
  struct sunxi_mmc_des *pdes = mmchost->pdes;
  unsigned byte_cnt = data->blocksize * data->blocks;
  unsigned char *buff;
  unsigned des_idx = 0;
  unsigned buff_frag_num = 0;
  unsigned remain;
  unsigned i, rval;

  buff = data->flags & MMC_DATA_READ ?
      (unsigned char *)data->dest : (unsigned char *)data->src;
  buff_frag_num = byte_cnt >> SDXC_DES_NUM_SHIFT;
  remain = byte_cnt & (SDXC_DES_BUFFER_MAX_LEN-1);
  if (remain)
    buff_frag_num ++;
  else
    remain = SDXC_DES_BUFFER_MAX_LEN;

  //flush_cache((unsigned long)buff, (unsigned long)byte_cnt);
  OSAL_CacheRangeFlush(buff, (unsigned long)byte_cnt, CACHE_CLEAN_FLUSH_D_CACHE_REGION);
  for (i=0; i < buff_frag_num; i++, des_idx++) {
    memset((void*)&pdes[des_idx], 0, sizeof(struct sunxi_mmc_des));
    pdes[des_idx].des_chain = 1;
    pdes[des_idx].own = 1;
    pdes[des_idx].dic = 1;
    if (buff_frag_num > 1 && i != buff_frag_num-1)
      pdes[des_idx].data_buf1_sz = SDXC_DES_BUFFER_MAX_LEN;
    else
      pdes[des_idx].data_buf1_sz = remain;

    pdes[des_idx].buf_addr_ptr1 = (u32)buff + i * SDXC_DES_BUFFER_MAX_LEN;
    if (i==0)
      pdes[des_idx].first_des = 1;

    if (i == buff_frag_num-1) {
      pdes[des_idx].dic = 0;
      pdes[des_idx].last_des = 1;
      pdes[des_idx].end_of_ring = 1;
      pdes[des_idx].buf_addr_ptr2 = 0;
    } else {
      pdes[des_idx].buf_addr_ptr2 = (u32)&pdes[des_idx+1];
    }
//    MMCDBG("frag %d, remain %d, des[%d](%08x): "
//      "[0] = %08x, [1] = %08x, [2] = %08x, [3] = %08x\n",
//      i, remain, des_idx, (u32)&pdes[des_idx],
//      (u32)((u32*)&pdes[des_idx])[0], (u32)((u32*)&pdes[des_idx])[1],
//      (u32)((u32*)&pdes[des_idx])[2], (u32)((u32*)&pdes[des_idx])[3]);
  }
  //flush_cache((unsigned long)pdes, sizeof(struct sunxi_mmc_des) * (des_idx+1));
  OSAL_CacheRangeFlush(pdes, sizeof(struct sunxi_mmc_des) * (des_idx+1), CACHE_CLEAN_FLUSH_D_CACHE_REGION);
  /*
   * GCTRLREG
   * GCTRL[2] : DMA reset
   * GCTRL[5] : DMA enable
   *
   * IDMACREG
   * IDMAC[0] : IDMA soft reset
   * IDMAC[1] : IDMA fix burst flag
   * IDMAC[7] : IDMA on
   *
   * IDIECREG
   * IDIE[0]  : IDMA transmit interrupt flag
   * IDIE[1]  : IDMA receive interrupt flag
   */
  rval = readl(&mmchost->reg->gctrl);
  writel(rval|(1 << 5)|(1 << 2), &mmchost->reg->gctrl); /* dma enable */
  writel((1 << 0), &mmchost->reg->dmac); /* idma reset */
  writel((1 << 1) | (1 << 7), &mmchost->reg->dmac); /* idma on */
  rval = readl(&mmchost->reg->idie) & (~3);
  if (data->flags & MMC_DATA_WRITE)
    rval |= (1 << 0);
  else
    rval |= (1 << 1);
  writel(rval, &mmchost->reg->idie);
  writel((unsigned long)pdes, &mmchost->reg->dlba);
  writel((2U<<28)|(7<<16)|8, &mmchost->reg->ftrglevel);

  return 0;
}

static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
      struct mmc_data *data)
{
  struct sunxi_mmc_host* mmchost = (struct sunxi_mmc_host *)mmc->priv;
  unsigned int cmdval = 0x80000000;
  signed int timeout = 0;
  int error = 0;
  unsigned int status = 0;
  unsigned int usedma = 0;
  unsigned int bytecnt = 0;

  if (mmchost->fatal_err){
    MMCINFO("mmc %d Found fatal err,so no send cmd\n",mmchost->mmc_no);
    return -1;
  }
  if (cmd->resp_type & MMC_RSP_BUSY)
    MMCDBG("mmc %d mmc cmd %d check rsp busy\n", mmchost->mmc_no,cmd->cmdidx);
  if (cmd->cmdidx == 12)
    return 0;
  /*
   * CMDREG
   * CMD[5:0] : Command index
   * CMD[6] : Has response
   * CMD[7] : Long response
   * CMD[8] : Check response CRC
   * CMD[9] : Has data
   * CMD[10]  : Write
   * CMD[11]  : Steam mode
   * CMD[12]  : Auto stop
   * CMD[13]  : Wait previous over
   * CMD[14]  : About cmd
   * CMD[15]  : Send initialization
   * CMD[21]  : Update clock
   * CMD[31]  : Load cmd
   */
  if (!cmd->cmdidx)
    cmdval |= (1 << 15);
  if (cmd->resp_type & MMC_RSP_PRESENT)
    cmdval |= (1 << 6);
  if (cmd->resp_type & MMC_RSP_136)
    cmdval |= (1 << 7);
  if (cmd->resp_type & MMC_RSP_CRC)
    cmdval |= (1 << 8);
  if (data) {
    if ((u32)data->dest & 0x3) {
      MMCINFO("mmc %d dest is not 4 byte align\n",mmchost->mmc_no);
      error = -1;
      goto out;
    }

    cmdval |= (1 << 9) | (1 << 13);
    if (data->flags & MMC_DATA_WRITE)
      cmdval |= (1 << 10);
    if (data->blocks > 1)
      cmdval |= (1 << 12);
    writel(data->blocksize, &mmchost->reg->blksz);
    writel(data->blocks * data->blocksize, &mmchost->reg->bytecnt);
  }

  MMCDBG("mmc %d, cmd %d(0x%08x), arg 0x%08x\n", mmchost->mmc_no, cmd->cmdidx, cmdval|cmd->cmdidx, cmd->cmdarg);
  writel(cmd->cmdarg, &mmchost->reg->arg);
  if (!data)
    writel(cmdval|cmd->cmdidx, &mmchost->reg->cmd);

  /*
   * transfer data and check status
   * STATREG[2] : FIFO empty
   * STATREG[3] : FIFO full
   */
  if (data) {
    int ret = 0;

    bytecnt = data->blocksize * data->blocks;
    MMCDBG("mmc %d trans data %d bytes\n",mmchost->mmc_no, bytecnt);
#ifdef CONFIG_MMC_SUNXI_USE_DMA
    if (bytecnt > 64) {
#else
    if (0) {
#endif
      usedma = 1;
      writel(readl(&mmchost->reg->gctrl)&(~0x80000000), &mmchost->reg->gctrl);
      ret = mmc_trans_data_by_dma(mmc, data);
      writel(cmdval|cmd->cmdidx, &mmchost->reg->cmd);
    } else {
      writel(readl(&mmchost->reg->gctrl)|0x80000000, &mmchost->reg->gctrl);
      writel(cmdval|cmd->cmdidx, &mmchost->reg->cmd);
      ret = mmc_trans_data_by_cpu(mmc, data);
    }
    if (ret) {
      MMCINFO("mmc %d Transfer failed\n",mmchost->mmc_no);
      error = readl(&mmchost->reg->rint) & 0xbfc2;
      if(!error)
        error = 0xffffffff;
      goto out;
    }
  }

  timeout = 1000;
  do {
    status = readl(&mmchost->reg->rint);
    if (!timeout-- || (status & 0xbfc2)) {
      error = status & 0xbfc2;
      if(!error)
        error = 0xffffffff;//represet software timeout
      MMCINFO("mmc %d cmd %d timeout, err %x\n",mmchost->mmc_no, cmd->cmdidx, error);
      goto out;
    }
    MicroSecondDelay(1);
  } while (!(status&0x4));

  if (data) {
    unsigned done = 0;
    timeout = usedma ? (50*bytecnt/25) : 0xfffff;//0.04us(25M)*2(4bit width)*25()
    if(timeout < 0xfffff){
      timeout = 0xfffff;
    }
    MMCDBG("mmc %d cacl timeout %x\n",mmchost->mmc_no, timeout);
    do {
      status = readl(&mmchost->reg->rint);
      if (!timeout-- || (status & 0xbfc2)) {
        error = status & 0xbfc2;
        if(!error)
          error = 0xffffffff;//represet software timeout
        MMCINFO("mmc %d data timeout %x\n",mmchost->mmc_no, error);
        goto out;
      }
      if (data->blocks > 1)
        done = status & (1 << 14);
      else
        done = status & (1 << 3);
      MicroSecondDelay(1);
    } while (!done);
  }

  if (cmd->resp_type & MMC_RSP_BUSY) {
    timeout = 500*1000;
    do {
      status = readl(&mmchost->reg->status);
      if (!timeout--) {
        error = -1;
        MMCINFO("mmc %d busy timeout\n",mmchost->mmc_no);
        goto out;
      }
      MicroSecondDelay(1);
    } while (status & (1 << 9));
  }

  if (cmd->resp_type & MMC_RSP_136) {
    cmd->response[0] = readl(&mmchost->reg->resp3);
    cmd->response[1] = readl(&mmchost->reg->resp2);
    cmd->response[2] = readl(&mmchost->reg->resp1);
    cmd->response[3] = readl(&mmchost->reg->resp0);
    MMCDBG("mmc %d mmc resp 0x%08x 0x%08x 0x%08x 0x%08x\n",
      mmchost->mmc_no,
      cmd->response[3], cmd->response[2],
      cmd->response[1], cmd->response[0]);
  } else {
    cmd->response[0] = readl(&mmchost->reg->resp0);
    MMCDBG("mmc %d mmc resp 0x%08x\n",mmchost->mmc_no, cmd->response[0]);
  }
out:
  if (data && usedma) {
    /* IDMASTAREG
     * IDST[0] : idma tx int
     * IDST[1] : idma rx int
     * IDST[2] : idma fatal bus error
     * IDST[4] : idma descriptor invalid
     * IDST[5] : idma error summary
     * IDST[8] : idma normal interrupt sumary
     * IDST[9] : idma abnormal interrupt sumary
     */
    status = readl(&mmchost->reg->idst);
    writel(status, &mmchost->reg->idst);
    writel(0, &mmchost->reg->idie);
    writel(0, &mmchost->reg->dmac);
    writel(readl(&mmchost->reg->gctrl)&(~(1 << 5)), &mmchost->reg->gctrl);
  }
  if (error) {
    writel(0x7, &mmchost->reg->gctrl);
    while(readl(&mmchost->reg->gctrl)&0x7){
    }

    mmc_update_clk(mmc);
    MMCINFO("mmc %d mmc cmd %d err 0x%08x\n",mmchost->mmc_no, cmd->cmdidx, error);
  }
  writel(0xffffffff, &mmchost->reg->rint);
  //writel(readl(&mmchost->reg->gctrl)|(1 << 1), &mmchost->reg->gctrl);


  if (error)
    return -1;
  else
    return 0;
}

int sunxi_mmc_init(int sdc_no)
{
  struct mmc *mmc;
 
  int ret;
  
  MMCINFO("mmc driver ver %a\n", DRIVER_VER);

  memset(&mmc_dev[sdc_no], 0, sizeof(struct mmc));
  memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));
  mmc = &mmc_dev[sdc_no];

  AsciiStrCpy(mmc->name, "SUNXI SD/MMC");
  mmc->priv = &mmc_host[sdc_no];
  mmc->send_cmd = mmc_send_cmd;
  mmc->set_ios = mmc_set_ios;
  mmc->init = mmc_core_init;
  mmc->control_num = sdc_no;

  mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34
    | MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30
    | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_34_35
    | MMC_VDD_35_36;
  mmc->host_caps = MMC_MODE_4BIT;
  mmc->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS| MMC_MODE_HC;

  if(sdc_no == 0){
       mmc->f_min = 400000;
#if defined CONFIG_ARCH_SUN9IW1P1
       mmc->f_max = 48000000;
#else
       mmc->f_max = 52000000;
#endif
  }else if (sdc_no == 2){
       mmc->f_min = 400000;
#if defined CONFIG_ARCH_SUN9IW1P1
       mmc->f_max = 48000000;
#else
       mmc->f_max = 52000000;
#endif
  }
    //mmc_host[sdc_no].pdes = malloc(64 * 1024);
  DmaAllocateBuffer(EfiBootServicesData,64,(void **)&mmc_host[sdc_no].pdes);
  if(mmc_host[sdc_no].pdes == NULL){
    MMCINFO("get mem for descriptor failed\n");
    return -1;
  }
  mmc_resource_init(sdc_no);
  mmc_clk_io_on(sdc_no);
  
  MMCINFO("PC Bias: 0x%08x 0x%08x\n", (0x6000800+0x308), *(volatile unsigned int *)(0x6000800+0x308));
  
  //while((*(volatile unsigned int *)0) != 1);
  ret = mmc_register(sdc_no, mmc);  

  if (ret < 0){   
    MMCINFO("mmc register failed\n"); 
    return -1;  
  } 
  
  return mmc->block_dev.lba;
  
}

int sunxi_mmc_exit(int sdc_no)
{
//  mmc_clk_io_onoff(sdc_no, 0);
  mmc_unregister(sdc_no);
  memset(&mmc_dev[sdc_no], 0, sizeof(struct mmc));
  DmaFreeBuffer(64,(void *)mmc_host[sdc_no].pdes);
  memset(&mmc_host[sdc_no], 0, sizeof(struct sunxi_mmc_host));

  MMCDBG("sunxi mmc%d exit\n",sdc_no);
  return 0;
}
