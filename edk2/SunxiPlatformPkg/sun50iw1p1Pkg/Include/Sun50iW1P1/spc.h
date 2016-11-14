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


#define SPC_DECPORT_STA_REG(n)       (SUNXI_SPC_BASE + (n) * 0x0C + 0x04)
#define SPC_DECPORT_SET_REG(n)       (SUNXI_SPC_BASE + (n) * 0x0C + 0x08)
#define SPC_DECPORT_CLR_REG(n)       (SUNXI_SPC_BASE + (n) * 0x0C + 0x0C)

#define SPC_STATUS_REG(n)      (SUNXI_SPC_BASE + (n) * 0x0C + 0x04)
#define SPC_SET_REG(n)         (SUNXI_SPC_BASE + (n) * 0x0C + 0x08)
#define SPC_CLEAR_REG(n)       (SUNXI_SPC_BASE + (n) * 0x0C + 0x0C)

void sunxi_spc_set_to_ns(uint type);
void sunxi_spc_set_to_s(uint type);
uint sunxi_spc_probe_status(uint type);

#endif /* __SMTA_H__ */
