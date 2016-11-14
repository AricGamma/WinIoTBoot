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

#ifndef __DISP_CAPTURE_H__
#define __DISP_CAPTURE_H__

#include "disp_private.h"

s32 disp_capture_start(struct disp_capture *cptr);
s32 disp_capture_stop(struct disp_capture *cptr);
s32 disp_capture_commit(struct disp_capture *cptr, struct disp_capture_info *info);
s32 disp_capture_set_manager(struct disp_capture *cptr, struct disp_manager *mgr);
s32 disp_capture_unset_manager(struct disp_capture *cptr);
s32 disp_capture_init(struct disp_capture *cptr);
s32 disp_capture_exit(struct disp_capture *cptr);
s32 disp_capture_sync(struct disp_capture *cptr);
s32 disp_init_capture(disp_bsp_init_para *para);
s32 disp_capture_shadow_protect(struct disp_capture *capture, bool protect);
s32 disp_capture_apply(struct disp_capture *cptr);
s32 disp_capture_force_apply(struct disp_capture *cptr);
s32 disp_capture_suspend(struct disp_capture *cptr);
s32 disp_capture_resume(struct disp_capture *cptr);
s32 disp_capture_query(struct disp_capture *cptr);

#endif
