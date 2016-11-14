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


#include  "usbc_i.h"

#define __msdelay(ms) MicroSecondDelay((ms)*1000)
#define udelay(us) MicroSecondDelay(us)
/*
*******************************************************************************
*                     usb_open_clock
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
int usb_open_clock(void)
{
  u32 reg_value = 0;

  //Enable module clock for USB phy0
  reg_value = readl(SUNXI_CCM_BASE + 0xcc);
  reg_value |= (1 << 0) | (1 << 8);
  writel(reg_value, (SUNXI_CCM_BASE + 0xcc));
  //delay some time
  __msdelay(10);

  //Gating AHB clock for USB_phy0
  reg_value = readl(SUNXI_CCM_BASE + 0x60);
  reg_value |= (1 << 23);
  writel(reg_value, (SUNXI_CCM_BASE + 0x60));

  //delay to wati SIE stable
  __msdelay(10);

  reg_value = readl(SUNXI_CCM_BASE + 0x2C0);
  reg_value |= (1 << 23);
  writel(reg_value, (SUNXI_CCM_BASE + 0x2C0));
  __msdelay(10);

  reg_value = readl(SUNXI_USBOTG_BASE + 0x420);
  reg_value |= (0x01 << 0);
  writel(reg_value, (SUNXI_USBOTG_BASE + 0x420));
  __msdelay(10);

  reg_value = readl(SUNXI_USBOTG_BASE + 0x410);
  reg_value &= ~(0x01 << 1);
  writel(reg_value, (SUNXI_USBOTG_BASE + 0x410));
  __msdelay(10);

  return 0;
}
/*
*******************************************************************************
*                     usb_op_clock
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
int usb_close_clock(void)
{
  u32 reg_value = 0;

  /* AHB reset */
  reg_value = readl(SUNXI_CCM_BASE + 0x2C0);
  reg_value &= ~(1 << 23);
  writel(reg_value, (SUNXI_CCM_BASE + 0x2C0));
  __msdelay(10);

  //关usb ahb时钟
  reg_value = readl(SUNXI_CCM_BASE + 0x60);
  reg_value &= ~(1 << 23);
  writel(reg_value, (SUNXI_CCM_BASE + 0x60));
  //等sie的时钟变稳
  __msdelay(10);

  //关USB phy时钟
  reg_value = readl(SUNXI_CCM_BASE + 0xcc);
  reg_value &= ~((1 << 0) | (1 << 8));
  writel(reg_value, (SUNXI_CCM_BASE + 0xcc));
  __msdelay(10);

  return 0;
}

