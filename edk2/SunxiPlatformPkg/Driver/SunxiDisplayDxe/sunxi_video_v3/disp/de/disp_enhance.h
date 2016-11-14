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

#ifndef _DISP_ENHANCE_H_
#define _DISP_ENHANCE_H_

#include "disp_private.h"

static s32 disp_enhance_shadow_protect(struct disp_enhance *enhance, bool protect);
s32 disp_init_enhance(disp_bsp_init_para * para);
s32 disp_enhance_set_para(struct disp_enhance* enhance, disp_enhance_para *para);


#endif

