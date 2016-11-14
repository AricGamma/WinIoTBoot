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

#ifndef _TIMER_H_
#define _TIMER_H_


#include "hd_scrpt.h"


#define TMRC_AVS_CTRL     (TMRC_REGS_BASE + 0x80)
#define TMRC_AVS_COUNT0     (TMRC_REGS_BASE + 0x84)
#define TMRC_AVS_COUNT1     (TMRC_REGS_BASE + 0x88)
#define TMRC_AVS_DIVISOR    (TMRC_REGS_BASE + 0x8C)


#define WATCHDOG1_CTRL        (TMRC_REGS_BASE + 0xB0)
#define WATCHDOG1_CFG         (TMRC_REGS_BASE + 0xB4)
#define WATCHDOG1_MODE        (TMRC_REGS_BASE + 0xB8)



#endif  /* _TMRC_H_ */

