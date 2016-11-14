/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  WangWei <wangwei@allwinnertech.com>
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
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/SysConfigLib.h>
#include <Library/IoLib.h>
#include <Library/ArmLib.h>
#include <Library/TimerLib.h>
#include <platform.h>
#include <ccmu.h>
#include <dram.h>

#define SUNXI_WDOG_CTL    (SUNXI_TIMER_BASE+0xB0)
#define SUNXI_WDOG_CFG    (SUNXI_TIMER_BASE+0xB4)
#define SUNXI_WDOG_MODE   (SUNXI_TIMER_BASE+0xB8)
#define SUNXI_RUN_EFEX_ADDR (SUNXI_RTC_BASE + 0x108)

VOID SunxiSetFelKey(VOID)
{

  const UINT32 SUNXI_RUN_EFEX_FLAG = 0x5AA5A55A;
  MmioWrite32(SUNXI_RUN_EFEX_ADDR,SUNXI_RUN_EFEX_FLAG);
}

VOID SunxiWatchDogReset(VOID)
{
  // Perform cold reset of the system.
  MmioWrite32 (SUNXI_WDOG_CFG, 0x1); //reset the whole system
  MmioWrite32 (SUNXI_WDOG_MODE, 0x1);//enable the watchdog and reset the system in 0.5s.
  MmioWrite32 (SUNXI_WDOG_CTL, 0x1);//restart the watchdog
  AsciiPrint("reset cpu\n");
  while(1);
}

UINT32 SunxiGetCoreClock(void)
{
  return 1000;
}

void BootDramUpdateFlagSet(UINT32 *dram_para)
{
  //dram_tpr13:bit31
  //0:uboot update boot0  1: not
  UINT32 flag = 0;
  __dram_para_t *pdram = (__dram_para_t *)dram_para;
  flag = pdram->dram_tpr13;
  flag |= (1<<31);
  pdram->dram_tpr13 = flag;
} 

void DumpDramPara(void* dram, UINT32 size)
{
  UINT32 i;
  UINT32 *addr = (UINT32 *)dram;

  for(i=0;i<size;i++)
  {
    AsciiPrint("dram para[%d] = 0x%x\n", i, addr[i]);
  }
}

