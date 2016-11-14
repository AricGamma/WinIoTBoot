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


/*
 ***************************************************************************
 *
 * 定义USB PHY控制寄存器位
 *
 ***************************************************************************
 */

//Common Control Bits for Both PHYs
#define  USBC_PHY_PLL_BW        0x03
#define  USBC_PHY_RES45_CAL_EN      0x0c

//Private Control Bits for Each PHY
#define  USBC_PHY_TX_AMPLITUDE_TUNE   0x20
#define  USBC_PHY_TX_SLEWRATE_TUNE    0x22
#define  USBC_PHY_VBUSVALID_TH_SEL    0x25
#define  USBC_PHY_PULLUP_RES_SEL    0x27
#define  USBC_PHY_OTG_FUNC_EN     0x28
#define  USBC_PHY_VBUS_DET_EN     0x29
#define  USBC_PHY_DISCON_TH_SEL     0x2a



/*
***********************************************************************************
*                     USBC_PHY_SetCommonConfig
*
* Description:
*    Phy的公共设置，用于USB PHY的公共初始化
*
* Arguments:
*    NULL
*
* Returns:
*    NULL
*
* note:
*    无
*
***********************************************************************************
*/
void USBC_PHY_SetCommonConfig(void)
{
    //__USBC_PHY_RES45_CALIBRATION_ENABLE(1);
}

/*
***********************************************************************************
*                     USBC_PHY_SetPrivateConfig
*
* Description:
*    USB PHY的各自设置
*
* Arguments:
*    hUSB       :  input.  USBC_open_otg获得的句柄, 记录了USBC所需要的一些关键数据
*
* Returns:
*    NULL
*
* note:
*    无
*
***********************************************************************************
*/
void USBC_PHY_SetPrivateConfig(__hdle hUSB)
{

}

/*
***********************************************************************************
*                     USBC_PHY_GetCommonConfig
*
* Description:
*    读取Phy的公共设置，主要用于Debug，看Phy的设置是否正确
*
* Arguments:
*    NULL
*
* Returns:
*    32bits的USB PHY公共设置值
*
* note:
*    无
*
***********************************************************************************
*/
__u32 USBC_PHY_GetCommonConfig(void)
{
  return 0;
}

/*
***********************************************************************************
*                                usb_phy0_write
*Description:
*    写usb phy0的phy寄存器，主要用于phy0 standby时的写入
*
*Arguments:
*    address,  data,   dmask
*
*returns:
*    return the data wrote
*
*note:
*    no
************************************************************************************
*/

static __u32 usb_phy0_write(__u32 addr, __u32 data, __u32 dmask, ulong usbc_base_addr)
{
  __u32 i=0;

  data = data & 0x0f;
  addr = addr & 0x0f;
  dmask = dmask & 0x0f;

  USBC_Writeb((dmask<<4)|data, usbc_base_addr + 0x404 + 2);
  USBC_Writeb(addr|0x10, usbc_base_addr + 0x404);
  for(i=0;i<5;i++);
  USBC_Writeb(addr|0x30, usbc_base_addr + 0x404);
  for(i=0;i<5;i++);
  USBC_Writeb(addr|0x10, usbc_base_addr + 0x404);
  for(i=0;i<5;i++);
  return (USBC_Readb(usbc_base_addr + 0x404 + 3) & 0x0f);
}

/*
*******************************************************************************
*                     USBC_phy_Standby
*
* Description:
*    Standby the usb phy with the input usb phy index number
*
* Parameters:
*    usb phy index number, which used to select the phy to standby
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void USBC_phy_Standby(__hdle hUSB, __u32 phy_index)
{
  __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

  if(phy_index == 0){
    usb_phy0_write(0xB, 0x8, 0xf, usbc_otg->base_addr);
    usb_phy0_write(0x7, 0xf, 0xf, usbc_otg->base_addr);
    usb_phy0_write(0x1, 0xf, 0xf, usbc_otg->base_addr);
    usb_phy0_write(0x2, 0xf, 0xf, usbc_otg->base_addr);
  }

  return;
}

/*
*******************************************************************************
*                     USBC_Phy_Standby_Recover
*
* Description:
*    Recover the standby phy with the input index number
*
* Parameters:
*    usb phy index number
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void USBC_Phy_Standby_Recover(__hdle hUSB, __u32 phy_index)
{
  __u32 i;

  if(phy_index == 0){
    for(i=0; i<0x10; i++);
  }

  return;
}



