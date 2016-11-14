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


#ifndef __SMTA_H__
#define __SMTA_H__


#include "platform.h"


#define  SMTA_TWI0         (1<<1)
#define  SMTA_TWI1         (1<<2)
#define  SMTA_SPI0         (1<<3)
#define  SMTA_SPI1         (1<<4)
#define  SMTA_GPIO         (1<<5)
#define  SMTA_DE_SYS       (1<<6)
#define  SMTA_MMC          (1<<7)

#define  SMTA_NDFC         (1<<8)
#define  SMTA_DMA          (1<<9)
#define  SMTA_SS           (1<<10)
#define  SMTA_SRAMA1       (1<<11)
#define  SMTA_USBOTG       (1<<12)
#define  SMTA_USBHOST      (1<<13)
#define  SMTA_DRAMC        (1<<14)




#define SMTA_STATUS_REG(n)      (SMTA_BASE + (n) * 0x0C + 0x04)
#define SMTA_SET_REG(n)         (SMTA_BASE + (n) * 0x0C + 0x08)
#define SMTA_CLEAR_REG(n)       (SMTA_BASE + (n) * 0x0C + 0x0C)


void sunxi_smta_set_to_ns(uint type);
void sunxi_smta_set_to_s(uint type);
uint sunxi_smta_probe_status(uint type);

#endif /* __SMTA_H__ */
