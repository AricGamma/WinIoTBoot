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

#ifndef  __USB_DMA_CONTROL_H__
#define  __USB_DMA_CONTROL_H__

#include <Sunxi_type/Sunxi_type.h>

extern int usb_dma_init(uint husb);

extern uint usb_dma_request(void);

extern int usb_dma_release(uint dma_index);

extern int usb_dma_setting(uint dma_index, uint trans_dir, uint ep);

extern int usb_dma_set_pktlen(uint dma_index, uint pkt_len);

extern int usb_dma_start(uint dma_index, uint addr, uint bytes);

extern int usb_dma_stop(uint dma_index);

extern int usb_dma_int_query(void);

extern int usb_dma_int_clear(void);

#endif
