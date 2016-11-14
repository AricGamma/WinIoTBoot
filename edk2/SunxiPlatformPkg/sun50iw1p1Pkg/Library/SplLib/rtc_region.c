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

#include <boot0_include/boot0_helper.h>
#include <Sun50iW1P1/platform.h>

#define  RTC_DATA_HOLD_REG_BASE        (SUNXI_RTC_BASE + 0x100)
#define  RTC_DATA_HOLD_REG_FEL         (RTC_DATA_HOLD_REG_BASE + 0x8)
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
unsigned int rtc_region_probe_fel_flag(void)
{
  unsigned int fel_flag, reg_value;
  int  i;

  fel_flag = readl(RTC_DATA_HOLD_REG_FEL);

  for(i=0;i<=5;i++)
  {
    reg_value = readl(RTC_DATA_HOLD_REG_BASE + i*4);
    printf("rtc[%d] value = 0x%x\n", i, reg_value);
  }

  return fel_flag;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void rtc_region_clear_fel_flag(void)
{
  volatile unsigned int flag = 0;
  do
  {
    writel(0, RTC_DATA_HOLD_REG_FEL);
    __usdelay(10);
    asm volatile("ISB SY");
    asm volatile("DMB SY");
    flag  = readl(RTC_DATA_HOLD_REG_FEL);
  }
  while(flag != 0);
}




