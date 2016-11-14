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

#ifndef  _DRV_TV_AC200_LOWLEVEL_H_
#define  _DRV_TV_AC200_LOWLEVEL_H_

#include "tv_ac200.h"

s32 aw1683_tve_init(void);
s32 aw1683_tve_plug_status(void);
s32 aw1683_tve_set_mode(u32 mode);
s32 aw1683_tve_open(void);
s32 aw1683_tve_close(void);

#endif