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
***********************************************************************************
*                     USBC_Dma_Set_ChannalPara
*
* Description:
*    查询fifo是否为空
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg获得的句柄, 记录了USBC所需要的一些关键数据
*
*
* Returns:
*
* note:
*    无
*
***********************************************************************************
*/
int USBC_Dma_Set_ChannalPara(__hdle hUSB, __u32 dma_chan, __u32 trans_dir, __u32 ep_type)
{
  __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
  uint  reg_val;

  if(usbc_otg == NULL)
  {
    return -1;
  }

  reg_val = readl(usbc_otg->base_addr + USBC_REG_o_DMA_CONFIG + 0x10 * dma_chan);
  reg_val &= ~((1 << 4) | (0xf << 0));
  reg_val |=  ((trans_dir & 1) << 4);
  reg_val |=  ((ep_type & 0xf) << 0);
  writel(reg_val, usbc_otg->base_addr + USBC_REG_o_DMA_CONFIG + 0x10 * dma_chan);

  return 0;
}
/*
***********************************************************************************
*                     USBC_Dma_Set_PktLen
*
* Description:
*    查询fifo是否为空
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg获得的句柄, 记录了USBC所需要的一些关键数据
*
*
* Returns:
*
* note:
*    无
*
***********************************************************************************
*/
int USBC_Dma_Set_PktLen(__hdle hUSB, __u32 dma_chan, __u32 pkt_len)
{
  __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
  uint  reg_val;

  if(usbc_otg == NULL)
  {
    return -1;
  }

  reg_val = readl(usbc_otg->base_addr + USBC_REG_o_DMA_CONFIG + 0x10 * dma_chan);
  reg_val &= ~(0x7ff << 16);
  //1650 burst len: datawidth is 32bit = 4byte,so burst len = pkt_len/4 
  //reg_val |=  (((pkt_len/4) & 0x7ff) << 16);

  //1667 burst len :  datawidth is 8bit, so burst len = 1byte
  reg_val |=  (((pkt_len) & 0x7ff) << 16);
  writel(reg_val, usbc_otg->base_addr + USBC_REG_o_DMA_CONFIG + 0x10 * dma_chan);

  return 0;
}
/*
***********************************************************************************
*                     USBC_Dma_Start
*
* Description:
*    查询fifo是否为空
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg获得的句柄, 记录了USBC所需要的一些关键数据
*
*
* Returns:
*
* note:
*    无
*
***********************************************************************************
*/
int USBC_Dma_Start(__hdle hUSB, __u32 dma_chan, __u32 addr, __u32 pkt_len)
{
  __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
  uint  reg_val;

  if(usbc_otg == NULL)
  {
    return -1;
  }

  if(pkt_len & 0x03)
  {
    return -1;
  }

  reg_val  = readl(usbc_otg->base_addr + USBC_REG_o_DMA_ENABLE);
  reg_val |=  (1 << (dma_chan & 0xff));
  writel(reg_val, usbc_otg->base_addr + USBC_REG_o_DMA_ENABLE);

  writel(addr,    usbc_otg->base_addr + USBC_REG_o_DMA_ADDR + 0x10 * dma_chan);
  //1650 datawidth is 32 bit.so size=len/4
  //writel(pkt_len/4, usbc_otg->base_addr + USBC_REG_o_DMA_SIZE + 0x10 * dma_chan);

  //1667 datawidth is 8bit 
  writel(pkt_len, usbc_otg->base_addr + USBC_REG_o_DMA_SIZE + 0x10 * dma_chan);

  reg_val = readl(usbc_otg->base_addr + USBC_REG_o_DMA_CONFIG + 0x10 * dma_chan);
  reg_val |= (1U << 31);

  writel(reg_val, usbc_otg->base_addr + USBC_REG_o_DMA_CONFIG + 0x10 * dma_chan);

  return 0;
}
/*
***********************************************************************************
*                     USBC_Dma_Int_Stop
*
* Description:
*    查询fifo是否为空
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg获得的句柄, 记录了USBC所需要的一些关键数据
*
*
* Returns:
*
* note:
*    无
*
***********************************************************************************
*/
int USBC_Dma_Int_Stop(__hdle hUSB, __u32 dma_chan)
{
  __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
  uint  reg_val;

  if(usbc_otg == NULL)
  {
    return -1;
  }

  reg_val  = readl(usbc_otg->base_addr + USBC_REG_o_DMA_ENABLE);
  reg_val &= ~(1 << (dma_chan & 0xff));
  writel(reg_val, usbc_otg->base_addr + USBC_REG_o_DMA_ENABLE);

  return 0;
}
/*
***********************************************************************************
*                     USBC_Dma_Int_Query
*
* Description:
*    查询fifo是否为空
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg获得的句柄, 记录了USBC所需要的一些关键数据
*
*
* Returns:
*
* note:
*    无
*
***********************************************************************************
*/
int USBC_Dma_Int_Query(__hdle hUSB)
{
  __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

  if(usbc_otg == NULL)
  {
    return -1;
  }

  return readl(usbc_otg->base_addr + USBC_REG_o_DMA_STATUS);
}
/*
***********************************************************************************
*                     USBC_Dma_Int_Clear
*
* Description:
*    查询fifo是否为空
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg获得的句柄, 记录了USBC所需要的一些关键数据
*
*
* Returns:
*
* note:
*    无
*
***********************************************************************************
*/
int USBC_Dma_Int_Clear(__hdle hUSB)
{
  __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

  if(usbc_otg == NULL)
  {
    return -1;
  }

  writel(0xff, usbc_otg->base_addr + USBC_REG_o_DMA_STATUS);

  return 0;
}
