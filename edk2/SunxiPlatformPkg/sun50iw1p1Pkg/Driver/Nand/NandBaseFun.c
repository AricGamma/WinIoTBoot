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

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h> 
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/SysConfigLib.h>
#include <Library/NandLib.h>
#include <Protocol/Cpu.h>

#define get_wvalue(addr)  (*((volatile unsigned long  *)(addr)))
#define put_wvalue(addr, v) (*((volatile unsigned long  *)(addr)) = (unsigned long)(v))


__u32 NAND_GetNdfcVersion(void);
void * NAND_Malloc(unsigned int Size);
void NAND_Free(void *pAddr, unsigned int Size);
static __u32 boot_mode = 0;




int NAND_set_boot_mode(__u32 boot)
{
  boot_mode = boot;
  return 0;
}


int NAND_Print(const CHAR8 * FormatString, ...)
{
  static CHAR16 _buf[1024];
  VA_LIST Marker;

  if(boot_mode)
    return 0;
  
  ZeroMem(_buf,sizeof(_buf));

  VA_START(Marker, FormatString);
  UnicodeVSPrintAsciiFormat(_buf, sizeof(_buf),FormatString, Marker);
  VA_END(Marker);

  Print(_buf);
  return 0;
    
}

void Nand_CacheRangeFlush(void*Address, UINT32 Length, UINT32 Flags)
{
  STATIC EFI_CPU_ARCH_PROTOCOL  *Cpu = NULL;
  EFI_STATUS    Status;
  
  // Ensure the Cpu architectural protocol is already installed
  if( Cpu == NULL) {
    Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&Cpu);
    ASSERT_EFI_ERROR(Status);
  }

  
  if(Address == NULL || Length == 0)
  {
    return;
  }
    
  switch(Flags)
  {
    case CACHE_FLUSH_I_CACHE_REGION:
      break;
      
    case CACHE_FLUSH_D_CACHE_REGION:
      Cpu->FlushDataCache(Cpu,(EFI_PHYSICAL_ADDRESS)(UINT32)(Address),(UINT64)Length,EfiCpuFlushTypeWriteBack);
      break;
      
    case CACHE_FLUSH_CACHE_REGION:
      break;
      
    case CACHE_CLEAN_D_CACHE_REGION:
      Cpu->FlushDataCache(Cpu,(EFI_PHYSICAL_ADDRESS)(UINT32)(Address),(UINT64)Length,EfiCpuFlushTypeInvalidate);
      break;
      
    case CACHE_CLEAN_FLUSH_D_CACHE_REGION:
      Cpu->FlushDataCache(Cpu,(EFI_PHYSICAL_ADDRESS)(UINT32)(Address),(UINT64)Length,EfiCpuFlushTypeWriteBackInvalidate);
      break;        
      
    case CACHE_CLEAN_FLUSH_CACHE_REGION:
      break;
        
    default:
      break;
  }
        
  return;
}


__s32 NAND_CleanFlushDCacheRegion(__u32 buff_addr, __u32 len)
{
  //flush_cache(buff_addr, len);
  Nand_CacheRangeFlush((void*)(buff_addr), len, CACHE_CLEAN_FLUSH_D_CACHE_REGION);
  //InvalidateDataCacheRange((void*)(buff_addr), len);
    
  return 0;
}

__u32 NAND_DMASingleMap(__u32 rw, __u32 buff_addr, __u32 len)
{
  return buff_addr;
}

__u32 NAND_DMASingleUnmap(__u32 rw, __u32 buff_addr, __u32 len)
{
  return buff_addr;
}

__s32 NAND_AllocMemoryForDMADescs(__u32 *cpu_addr, __u32 *phy_addr)
{
  void *p = NULL;

#if 0

    __u32 physical_addr  = 0;
  //void *dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *handle, gfp_t gfp);
  p = (void *)dma_alloc_coherent(NULL, PAGE_SIZE,
        (dma_addr_t *)&physical_addr, GFP_KERNEL);

  if (p == NULL) {
    AsciiPrint("NAND_AllocMemoryForDMADescs(): alloc dma des failed\n");
    return -1;
  } else {
    *cpu_addr = (__u32)p;
    *phy_addr = physical_addr;
    AsciiPrint("NAND_AllocMemoryForDMADescs(): cpu: 0x%x    physic: 0x%x\n",
      *cpu_addr, *phy_addr);
  }

#else

  p = (void *)NAND_Malloc(1024);

  if (p == NULL) {
    AsciiPrint("NAND_AllocMemoryForDMADescs(): alloc dma des failed\n");
    return -1;
  } else {
    *cpu_addr = (__u32)p;
    *phy_addr = (__u32)p;
    AsciiPrint("NAND_AllocMemoryForDMADescs(): cpu: 0x%x    physic: 0x%x\n",
      *cpu_addr, *phy_addr);
  }

#endif

  return 0;
}

__s32 NAND_FreeMemoryForDMADescs(__u32 *cpu_addr, __u32 *phy_addr)
{
  
#if 0

  //void dma_free_coherent(struct device *dev, size_t size, void *cpu_addr, dma_addr_t handle);
  dma_free_coherent(NULL, PAGE_SIZE, (void *)(*cpu_addr), *phy_addr);

#else
  if((void *)(*cpu_addr) != NULL)
    NAND_Free((void *)(*cpu_addr), 1024);

#endif

  *cpu_addr = 0;
  *phy_addr = 0;

  return 0;
}

int NAND_WaitDmaFinish(void)
{
  return 0;
}

__u32 _get_pll4_periph1_clk(void)
{
#if 0
  return 24000000;
#else
  __u32 n,div1,div2;
  __u32 rval;

  rval = get_wvalue((0x06000000 + 0xC)); //read pll4-periph1 register

  n = (0xff & (rval >> 8));
  div1 = (0x1 & (rval >> 16));
  div2 = (0x1 & (rval >> 18));

  rval = 24000000 * n / (div1+1) / (div2+1);;
  //AsciiPrint("pll4 clock is %d Hz\n", rval);
  
  return rval; //24000000 * n / (div1+1) / (div2+1);
#endif
}

__s32 _get_ndfc_clk_v2(__u32 nand_index, __u32 *pdclk, __u32 *pcclk)
{
  __u32 sclk0_reg_adr, sclk1_reg_adr;
  __u32 sclk_src, sclk_src_sel;
  __u32 sclk_pre_ratio_n, sclk_ratio_m;
  __u32 reg_val, sclk0, sclk1;
  
  if (nand_index == 0) {
    sclk0_reg_adr = (0x06000400 + 0x0); //CCM_NAND0_CLK0_REG;
    sclk1_reg_adr = (0x06000400 + 0x4); //CCM_NAND0_CLK1_REG;
  } else if (nand_index == 1) {
    sclk0_reg_adr = (0x06000400 + 0x8); //CCM_NAND1_CLK0_REG;
    sclk1_reg_adr = (0x06000400 + 0xC); //CCM_NAND1_CLK1_REG;
  } else {
    AsciiPrint("_get_ndfc_clk_v2 error, wrong nand index: %d\n", nand_index);
    return -1;  
  }

  // sclk0
  reg_val = get_wvalue(sclk0_reg_adr);
  sclk_src_sel     = (reg_val>>24) & 0x3;
  sclk_pre_ratio_n = (reg_val>>16) & 0x3;;
  sclk_ratio_m     = (reg_val) & 0xf;
  if (sclk_src_sel == 0)
    sclk_src = 24;
  else
    sclk_src = _get_pll4_periph1_clk()/1000000;
  sclk0 = (sclk_src >> sclk_pre_ratio_n) / (sclk_ratio_m+1);

  // sclk1
  reg_val = get_wvalue(sclk1_reg_adr);
  sclk_src_sel     = (reg_val>>24) & 0x3;
  sclk_pre_ratio_n = (reg_val>>16) & 0x3;;
  sclk_ratio_m     = (reg_val) & 0xf;
  if (sclk_src_sel == 0)
    sclk_src = 24;
  else
    sclk_src = _get_pll4_periph1_clk()/1000000;
  sclk1 = (sclk_src >> sclk_pre_ratio_n) / (sclk_ratio_m+1);

  if (nand_index == 0) {
    //AsciiPrint("Reg 0x06000400: 0x%x\n", *(volatile __u32 *)(0x06000400));
    //AsciiPrint("Reg 0x06000404: 0x%x\n", *(volatile __u32 *)(0x06000404));
  } else {
    //AsciiPrint("Reg 0x06000408: 0x%x\n", *(volatile __u32 *)(0x06000400));
    //AsciiPrint("Reg 0x0600040C: 0x%x\n", *(volatile __u32 *)(0x06000404));
  }
  //AsciiPrint("NDFC%d:  sclk0(2*dclk): %d MHz   sclk1(cclk): %d MHz\n", nand_index, sclk0, sclk1);

  *pdclk = sclk0/2;
  *pcclk = sclk1;

  return 0;
}

__s32 _change_ndfc_clk_v2(__u32 nand_index, __u32 dclk_src_sel, __u32 dclk, __u32 cclk_src_sel, __u32 cclk)
{
  u32 reg_val;
  u32 sclk0_src_sel, sclk0, sclk0_src, sclk0_pre_ratio_n, sclk0_src_t, sclk0_ratio_m;
  u32 sclk1_src_sel, sclk1, sclk1_src, sclk1_pre_ratio_n, sclk1_src_t, sclk1_ratio_m;
  u32 sclk0_reg_adr, sclk1_reg_adr;

  if (nand_index == 0) {
    sclk0_reg_adr = (0x06000400 + 0x0); //CCM_NAND0_CLK0_REG;
    sclk1_reg_adr = (0x06000400 + 0x4); //CCM_NAND0_CLK1_REG;
  } else if (nand_index == 1) {
    sclk0_reg_adr = (0x06000400 + 0x8); //CCM_NAND1_CLK0_REG;
    sclk1_reg_adr = (0x06000400 + 0xC); //CCM_NAND1_CLK1_REG;
  } else {
    AsciiPrint("_change_ndfc_clk_v2 error, wrong nand index: %d\n", nand_index);
    return -1;
  }

  /*close dclk and cclk*/
  if ((dclk == 0) && (cclk == 0))
  {
    reg_val = get_wvalue(sclk0_reg_adr);
    reg_val &= (~(0x1U<<31));
    put_wvalue(sclk0_reg_adr, reg_val);

    reg_val = get_wvalue(sclk1_reg_adr);
    reg_val &= (~(0x1U<<31));
    put_wvalue(sclk1_reg_adr, reg_val);

    AsciiPrint("_change_ndfc_clk_v2, close sclk0 and sclk1\n");
    return 0;
  }

  sclk0_src_sel = dclk_src_sel;
  sclk0 = dclk*2; //set sclk0 to 2*dclk.
  sclk1_src_sel = cclk_src_sel;
  sclk1 = cclk;

  if(sclk0_src_sel == 0x0) {
    //osc pll
    sclk0_src = 24;
  } else {
    //pll4
    sclk0_src = _get_pll4_periph1_clk()/1000000;
  }

  if(sclk1_src_sel == 0x0) {
    //osc pll
    sclk1_src = 24;
  } else {
    //pll4
    sclk1_src = _get_pll4_periph1_clk()/1000000;
  }

  //////////////////// sclk0: 2*dclk
  //sclk0_pre_ratio_n
  sclk0_pre_ratio_n = 3;
  if(sclk0_src > 4*16*sclk0)
    sclk0_pre_ratio_n = 3;
  else if (sclk0_src > 2*16*sclk0)
    sclk0_pre_ratio_n = 2;
  else if (sclk0_src > 1*16*sclk0)
    sclk0_pre_ratio_n = 1;
  else
    sclk0_pre_ratio_n = 0;

  sclk0_src_t = sclk0_src>>sclk0_pre_ratio_n;

  //sclk0_ratio_m
  sclk0_ratio_m = (sclk0_src_t/(sclk0)) - 1;
  if( sclk0_src_t%(sclk0) )
    sclk0_ratio_m +=1;


  //////////////// sclk1: cclk
  //sclk1_pre_ratio_n
  sclk1_pre_ratio_n = 3;
  if(sclk1_src > 4*16*sclk1)
    sclk1_pre_ratio_n = 3;
  else if (sclk1_src > 2*16*sclk1)
    sclk1_pre_ratio_n = 2;
  else if (sclk1_src > 1*16*sclk1)
    sclk1_pre_ratio_n = 1;
  else
    sclk1_pre_ratio_n = 0;

  sclk1_src_t = sclk1_src>>sclk1_pre_ratio_n;

  //sclk1_ratio_m
  sclk1_ratio_m = (sclk1_src_t/(sclk1)) - 1;
  if( sclk1_src_t%(sclk1) )
    sclk1_ratio_m +=1;

  /////////////////////////////// close clock
  reg_val = get_wvalue(sclk0_reg_adr);
  reg_val &= (~(0x1U<<31));
  put_wvalue(sclk0_reg_adr, reg_val);

  reg_val = get_wvalue(sclk1_reg_adr);
  reg_val &= (~(0x1U<<31));
  put_wvalue(sclk1_reg_adr, reg_val);


  ///////////////////////////////configure
  //sclk0 <--> 2*dclk
  reg_val = get_wvalue(sclk0_reg_adr);
  //clock source select
  reg_val &= (~(0x3<<24));
  reg_val |= (sclk0_src_sel&0x3)<<24;
  //clock pre-divide ratio(N)
  reg_val &= (~(0x3<<16));
  reg_val |= (sclk0_pre_ratio_n&0x3)<<16;
  //clock divide ratio(M)
  reg_val &= ~(0xf<<0);
  reg_val |= (sclk0_ratio_m&0xf)<<0;
  put_wvalue(sclk0_reg_adr, reg_val);

  //sclk1 <--> cclk
  reg_val = get_wvalue(sclk1_reg_adr);
  //clock source select
  reg_val &= (~(0x3<<24));
  reg_val |= (sclk1_src_sel&0x3)<<24;
  //clock pre-divide ratio(N)
  reg_val &= (~(0x3<<16));
  reg_val |= (sclk1_pre_ratio_n&0x3)<<16;
  //clock divide ratio(M)
  reg_val &= ~(0xf<<0);
  reg_val |= (sclk1_ratio_m&0xf)<<0;
  put_wvalue(sclk1_reg_adr, reg_val);


  /////////////////////////////// open clock
  reg_val = get_wvalue(sclk0_reg_adr);
  reg_val |= 0x1U<<31;
  put_wvalue(sclk0_reg_adr, reg_val);

  reg_val = get_wvalue(sclk1_reg_adr);
  reg_val |= 0x1U<<31;
  put_wvalue(sclk1_reg_adr, reg_val);
  
  
  //AsciiPrint("NAND_SetClk for nand index %d \n", nand_index);
  if (nand_index == 0) {
    //AsciiPrint("Reg 0x06000400: 0x%x\n", *(volatile __u32 *)(0x06000400));
    //AsciiPrint("Reg 0x06000404: 0x%x\n", *(volatile __u32 *)(0x06000404));
  } else {
    //AsciiPrint("Reg 0x06000408: 0x%x\n", *(volatile __u32 *)(0x06000400));
    //AsciiPrint("Reg 0x0600040C: 0x%x\n", *(volatile __u32 *)(0x06000404));
  }

  return 0;
}

__s32 _open_ndfc_ahb_gate_and_reset_v2(__u32 nand_index)
{
  __u32 reg_val=0;
  
  /*
    1. release ahb reset and open ahb clock gate for ndfc version 2.
  */
  if (nand_index == 0) {
    // reset
    reg_val = *(volatile __u32 *)(0x06000400 + 0x1A0);
    reg_val &= (~(0x1U<<13));
    reg_val |= (0x1U<<13);
    *(volatile __u32 *)(0x06000400 + 0x1A0) = reg_val;
    // ahb clock gate
    reg_val = *(volatile __u32 *)(0x06000400 + 0x180);
    reg_val &= (~(0x1U<<13));
    reg_val |= (0x1U<<13);
    *(volatile __u32 *)(0x06000400 + 0x180) = reg_val;
  } else if (nand_index == 1) {
    // reset
    reg_val = *(volatile __u32 *)(0x06000400 + 0x1A0);
    reg_val &= (~(0x1U<<12));
    reg_val |= (0x1U<<12);
    *(volatile __u32 *)(0x06000400 + 0x1A0) = reg_val;
    // ahb clock gate
    reg_val = *(volatile __u32 *)(0x06000400 + 0x180);
    reg_val &= (~(0x1U<<12));
    reg_val |= (0x1U<<12);
    *(volatile __u32 *)(0x06000400 + 0x180) = reg_val;
  } else {
    AsciiPrint("_open_ndfc_ahb_gate_and_reset_v2, wrong nand index: %d\n", nand_index);
    return -1;
  }
  
  return 0;
}

__u32 temp_reg = 0;
__s32 _cfg_ndfc_gpio_v2(__u32 nand_index)
{
  __u32 cfg;

  if (nand_index == 0) {
    *(volatile __u32 *)(0x06000800 + 0x48) = 0x22222222;
    *(volatile __u32 *)(0x06000800 + 0x4c) = 0x22222222;
    cfg = *(volatile __u32 *)(0x06000800 + 0x50);
    cfg &= (~0xfff);
    cfg |= 0x222;
    *(volatile __u32 *)(0x06000800 + 0x50) = cfg;

    //pull-up/down
    *(volatile __u32 *)(0x06000800 + 0x64) = 0x00005140;
    cfg = *(volatile __u32 *)(0x06000800 + 0x68);
    cfg &= (~0xfff);
    cfg |= 0x014;
    *(volatile __u32 *)(0x06000800 + 0x68) = cfg;

    temp_reg = *(volatile __u32 *)(0x06000800 + 0x308);
    *(volatile __u32 *)(0x06000800 + 0x308) = 0xa;

    AsciiPrint("NAND_PIORequest, nand_index: 0x%x\n", nand_index);
    AsciiPrint("Reg 0x06000848: 0x%x\n", *(volatile __u32 *)(0x06000848));
    AsciiPrint("Reg 0x0600084c: 0x%x\n", *(volatile __u32 *)(0x0600084c));
    AsciiPrint("Reg 0x06000850: 0x%x\n", *(volatile __u32 *)(0x06000850));
    AsciiPrint("Reg 0x06000864: 0x%x\n", *(volatile __u32 *)(0x06000864));
    AsciiPrint("Reg 0x06000868: 0x%x\n", *(volatile __u32 *)(0x06000868));
    AsciiPrint("Reg 0x06000b08: 0x%x\n", *(volatile __u32 *)(0x06000b08));    
  } else if(nand_index == 1) {
    *(volatile __u32 *)(0x06000800 + 0x120) = 0x22222222;
    *(volatile __u32 *)(0x06000800 + 0x124) = 0x22222222;
    cfg = *(volatile __u32 *)(0x06000800 + 0x128);
    cfg &= (~0xfff);
    cfg |= 0x222;
    *(volatile __u32 *)(0x06000800 + 0x128) = cfg;

    //pull-up/down
    *(volatile __u32 *)(0x06000800 + 0x13c) = 0x00005140;
    *(volatile __u32 *)(0x06000800 + 0x140) = 0x014;

    AsciiPrint("NAND_PIORequest, nand_index: 0x%x\n", nand_index);
    AsciiPrint("Reg 0x06000920: 0x%x\n", *(volatile __u32 *)(0x06000920));
    AsciiPrint("Reg 0x06000924: 0x%x\n", *(volatile __u32 *)(0x06000924));
    AsciiPrint("Reg 0x06000928: 0x%x\n", *(volatile __u32 *)(0x06000928));
    AsciiPrint("Reg 0x0600093c: 0x%x\n", *(volatile __u32 *)(0x0600093c));
    AsciiPrint("Reg 0x06000940: 0x%x\n", *(volatile __u32 *)(0x06000940));
  } else {
    AsciiPrint("NAND_PIORequest error, wrong nand_index: 0x%x\n", nand_index);
    return -1;
  }
  
  return 0;
}

int NAND_ClkRequest(__u32 nand_index)
{
  __s32 ret = 0;
  __u32 ndfc_version = NAND_GetNdfcVersion();
  
  if (ndfc_version == 2) {
    if ((nand_index != 0) && (nand_index != 1)) {
      AsciiPrint("NAND_ClkRequest, wrong nand index %d for ndfc version %d\n",
          nand_index, ndfc_version);
      return -1;
    }
    
    // 1. release ahb reset and open ahb clock gate
    _open_ndfc_ahb_gate_and_reset_v2(nand_index);
    
    // 2. configure ndfc's sclk0
    ret = _change_ndfc_clk_v2(nand_index, 1, 10, 1, 10*2);
    if (ret<0) {
      AsciiPrint("NAND_ClkRequest, set dclk failed!\n");
      return -1;
    }
    
  } else {
    AsciiPrint("NAND_ClkRequest, wrong ndfc version, %d\n", ndfc_version);
    return -1;
  }

  return 0;
}


void NAND_ClkRelease(__u32 nand_index)
{
  return ;
}

/*
**********************************************************************************************************************
*
*             NAND_GetCmuClk
*
*  Description:
*
*
*  Parameters:
*
*
*  Return value:
*
*
**********************************************************************************************************************
*/
int NAND_SetClk(__u32 nand_index, __u32 nand_clk0, __u32 nand_clk1)
{
  __u32 ndfc_version = NAND_GetNdfcVersion();
  __u32 dclk_src_sel, dclk, cclk_src_sel, cclk;
  __s32 ret = 0;
    
  if (ndfc_version == 2) { 
    if ((nand_index != 0) && (nand_index != 1)) {
      AsciiPrint("NAND_ClkRequest, wrong nand index %d for ndfc version %d\n",
          nand_index, ndfc_version);
      return -1;
    }
    
    ////////////////////////////////////////////////
    dclk_src_sel = 1;
    dclk = nand_clk0;
    cclk_src_sel = 1;
    cclk = nand_clk1;
    ////////////////////////////////////////////////
  
    ret = _change_ndfc_clk_v2(nand_index, dclk_src_sel, dclk, cclk_src_sel, cclk);
    if (ret < 0) {
      AsciiPrint("NAND_SetClk, change ndfc clock failed\n");
      return -1;
    }   
    
  } else {
    AsciiPrint("NAND_SetClk, wrong ndfc version, %d\n", ndfc_version);
    return -1;    
  }

  return 0;
}

int NAND_GetClk(__u32 nand_index, __u32 *pnand_clk0, __u32 *pnand_clk1)
{
  __s32 ret;
  __u32 ndfc_version = NAND_GetNdfcVersion();
  
  if (ndfc_version == 2) {
    
    //NAND_Print("NAND_GetClk for nand index %d \n", nand_index);
    ret = _get_ndfc_clk_v2(nand_index, pnand_clk0, pnand_clk1);
    if (ret < 0) {
      AsciiPrint("NAND_GetClk, failed!\n");
      return -1;
    }
    
  } else {
    AsciiPrint("NAND_SetClk, wrong ndfc version, %d\n", ndfc_version);
    return -1;    
  }
  
  return 0;
}

void NAND_PIORequest(__u32 nand_index)
{
  __s32 ret = 0;
  __u32 ndfc_version = NAND_GetNdfcVersion();
  
  if (ndfc_version == 2) {
    
    AsciiPrint("NAND_PIORequest for nand index %d \n", nand_index);
    ret = _cfg_ndfc_gpio_v2(nand_index);
    if (ret < 0) {
      AsciiPrint("NAND_PIORequest, failed!\n");
      return;
    }
    
  } else {
    AsciiPrint("NAND_PIORequest, wrong ndfc version, %d\n", ndfc_version);
    return;   
  }
  
  return;
}

__s32 NAND_PIOFuncChange_DQSc(__u32 nand_index, __u32 en)
{
  __u32 ndfc_version;
  __u32 cfg;
  
  ndfc_version = NAND_GetNdfcVersion();
  if (ndfc_version == 1) {
    AsciiPrint("NAND_PIOFuncChange_EnDQScREc: invalid ndfc version!\n");
    return 0;
  }
  
  if (ndfc_version == 2) {
    
    if (nand_index == 0) {
      cfg = *(volatile __u32 *)(0x06000800 + 0x50); 
      cfg &= (~(0x7U<<8));
      cfg |= (0x3U<<8);
      *(volatile __u32 *)(0x06000800 + 0x50) = cfg;
    } else {
      cfg = *(volatile __u32 *)(0x06000800 + 0x128); 
      cfg &= (~(0x7U<<8));
      cfg |= (0x3U<<8);
      *(volatile __u32 *)(0x06000800 + 0x128) = cfg;
    }   
  } 
  
  return 0;
}
__s32 NAND_PIOFuncChange_REc(__u32 nand_index, __u32 en)
{
  __u32 ndfc_version;
  __u32 cfg;
  
  ndfc_version = NAND_GetNdfcVersion();
  if (ndfc_version == 1) {
    AsciiPrint("NAND_PIOFuncChange_EnDQScREc: invalid ndfc version!\n");
    return 0;
  }
  
  if (ndfc_version == 2) {
    
    if (nand_index == 0) {
      cfg = *(volatile __u32 *)(0x06000800 + 0x50); 
      cfg &= (~(0x7U<<4));
      cfg |= (0x3U<<4);
      *(volatile __u32 *)(0x06000800 + 0x50) = cfg;
    } else {
      cfg = *(volatile __u32 *)(0x06000800 + 0x128); 
      cfg &= (~(0x7U<<4));
      cfg |= (0x3U<<4);
      *(volatile __u32 *)(0x06000800 + 0x128) = cfg;
    }   
  } 
  
  return 0;
}

void NAND_PIORelease(__u32 nand_index)
{
  *(volatile __u32 *)(0x06000800 + 0x308)  = temp_reg;
  AsciiPrint("Reg 0x06000b08: 0x%x\n", *(volatile __u32 *)(0x06000b08));    
  return;
}

void NAND_Memset(void* pAddr, unsigned char value, unsigned int len)
{
  SetMem(pAddr,len,value);
}

void NAND_Memcpy(void* pAddr_dst, void* pAddr_src, unsigned int len)
{
  CopyMem(pAddr_dst, pAddr_src, len);
}

#if 0
#define NAND_MEM_BASE  0x59000000

void * NAND_Malloc(unsigned int Size)
{
  __u32 mem_addr;

  mem_addr = NAND_MEM_BASE+malloc_size;

  malloc_size += Size;
  if(malloc_size%4)
    malloc_size += (4-(malloc_size%4));

  //NAND_Print("NAND_Malloc: 0x%x\n", NAND_MEM_BASE + malloc_size);

  if(malloc_size>0x4000000)
    return NULL;
  else
    return (void *)mem_addr;
}

void NAND_Free(void *pAddr, unsigned int Size)
{
  //free(pAddr);
}

#else
void * NAND_Malloc(unsigned int Size)
{
  return AllocatePool(Size);
}

void NAND_Free(void *pAddr, unsigned int Size)
{
  FreePool(pAddr);
}
#endif




void  OSAL_IrqUnLock(unsigned int  p)
{
    ;
}
void  OSAL_IrqLock  (unsigned int *p)
{
    ;
}

int NAND_WaitRbReady(void)
{
  return 0;
}

void *NAND_IORemap(unsigned int base_addr, unsigned int size)
{
  return (void *)base_addr;
}

__u32 NAND_VA_TO_PA(__u32 buff_addr)
{
  return buff_addr;
}

__u32 NAND_GetIOBaseAddrCH0(void)
{
  return 0x01c03000;
}

__u32 NAND_GetIOBaseAddrCH1(void)
{
  return 0x01c04000;
}

__u32 NAND_GetNdfcVersion(void)
{
  return 2;
}

__u32 NAND_GetNdfcDmaMode(void)
{
  /*
    0: General DMA;
    1: MBUS DMA
    
    Only support MBUS DMA!!!!   
  */
  return 1;   
}

int NAND_PhysicLockInit(void)
{
  return 0;
}

int NAND_PhysicLock(void)
{
  return 0;
}

int NAND_PhysicUnLock(void)
{
  return 0;
}

int NAND_PhysicLockExit(void)
{
  return 0;
}

__u32 NAND_GetMaxChannelCnt(void)
{
  return 1; //1; //2
}

__u32 NAND_GetPlatform(void)
{
  return 80;
}

unsigned int dma_chan = 0;  

/* request dma channel and set callback function */
int nand_request_dma(void)
{
  AsciiPrint("uboot nand_request_dma: current platform do not support general dma!\n");
  return -1;
}
int NAND_ReleaseDMA(__u32 nand_index)
{
  AsciiPrint("uboot nand_request_dma: current platform do not support general dma!\n");
  return 0;

}

int nand_dma_config_start(__u32 write, __u32 addr,__u32 length)
{
  AsciiPrint("uboot nand_dma_config_start: current platform do not support general dma!\n");
  return -1;
}
__u32 NAND_GetNandExtPara(__u32 para_num)
{
  int nand_para;
  script_parser_value_type_t ret;

  if (para_num == 0) {
    ret = script_parser_fetch("nand0_para", "nand_p0", &nand_para, 1);
    if(ret!=SCRIPT_PARSER_OK)
    {
      AsciiPrint("NAND_GetNandExtPara: get nand_p0 fail, %x\n", nand_para);
      return 0xffffffff;
    }
    else
      return nand_para;
  } else if (para_num == 1) {
    ret = script_parser_fetch("nand0_para", "nand_p1", &nand_para, 1);
    if(ret!=SCRIPT_PARSER_OK)
    {
      AsciiPrint("NAND_GetNandExtPara: get nand_p1 fail, %x\n", nand_para);
      return 0xffffffff;
    }
    else
      return nand_para;
  } else {
    AsciiPrint("NAND_GetNandExtPara: wrong para num: %d\n", para_num);
    return 0xffffffff;
  }
}

__u32 NAND_GetNandIDNumCtrl(void)
{
  int id_number_ctl;
  script_parser_value_type_t ret;

  ret = script_parser_fetch("nand0_para", "id_number_ctl", &id_number_ctl, 1);
  if(ret!=SCRIPT_PARSER_OK) {
    AsciiPrint("nand : get id_number_ctl fail, %x\n",id_number_ctl);
    return 0x0;
  } else {
    AsciiPrint("nand : get id_number_ctl from script, %x\n",id_number_ctl);
    return id_number_ctl;
  }
}

__u32 NAND_GetNandCapacityLevel(void)
{
  int CapacityLevel;
  script_parser_value_type_t ret;

  ret = script_parser_fetch("nand0_para", "nand_capacity_level", &CapacityLevel, 1);
  if(ret!=SCRIPT_PARSER_OK) {
    AsciiPrint("nand : get CapacityLevel fail, %x\n",CapacityLevel);
    return 0x0;
  } else {
    AsciiPrint("nand : get CapacityLevel from script, %x\n",CapacityLevel);
    return CapacityLevel;
  }
}

static void dumphex32(char *name, char *base, int len)
{
  __u32 i;

  if ((unsigned int)base&0xf0000000) {
    AsciiPrint("dumphex32: err para in uboot, %a 0x%x\n", name, (unsigned int)base);
    return ;
  }

  AsciiPrint("dump %a registers:", name);
  for (i=0; i<len*4; i+=4) {
    if (!(i&0xf))
      AsciiPrint("\n0x%p : ", base + i);
    AsciiPrint("0x%08x ", *((volatile unsigned int *)(base + i)));
  }
  AsciiPrint("\n");
}

void NAND_DumpReg(void)
{
  dumphex32("nand0 reg", (char*)0x01c03000, 136);
  dumphex32("nand1 reg", (char*)0x01c04000, 136);
  dumphex32("gpio reg", (char*)0x06000848, 20);
  dumphex32("clk reg0",  (char*)0x06000400, 10);
  dumphex32("clk reg1",  (char*)0x06000000, 10);
  //dumphex32("dma reg part0", (char*)0x01c02000, 8);
  //dumphex32("dma reg part1", (char*)0x01c02100, 50);
}

void NAND_ShowEnv(__u32 type, char *name, __u32 len, __u32 *val)
{
  int i;

  if (len && (val==NULL)) {
    AsciiPrint("uboot:NAND_ShowEnv, para error!\n");
    return ;
  }

  if (type == 0)
  {
    AsciiPrint("uboot:%a: ", name);
    for (i=0; i<len; i++)
    {
      if (i && (i%8==0))
        AsciiPrint("\n");
      AsciiPrint("%x ", val[i]);
    }
    AsciiPrint("\n");
  }
  else
  {
    AsciiPrint("uboot:NAND_ShowEnv, type error, %d!\n", type);
  }

  return ;
}

int NAND_GetVoltage(void)
{
  return 0;
}

int NAND_ReleaseVoltage(void)
{
  int ret = 0;
  return ret;
}

