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

#ifndef __DISP_DEVICE_H__
#define __DISP_DEVICE_H__

#include "disp_private.h"

s32 disp_device_set_manager(struct disp_device* dispdev, struct disp_manager *mgr);
s32 disp_device_unset_manager(struct disp_device* dispdev);
s32 disp_device_get_resolution(struct disp_device* dispdev, u32 *xres, u32 *yres);
s32 disp_device_get_timings(struct disp_device* dispdev, struct disp_video_timings *timings);
s32 disp_device_is_interlace(struct disp_device *dispdev);
s32 disp_device_register(struct disp_device *dispdev);
s32 disp_device_unregister(struct disp_device *dispdev);
struct disp_device* disp_device_get(int disp, enum disp_output_type output_type);
struct disp_device* disp_device_find(int disp, enum disp_output_type output_type);
struct list_head* disp_device_get_list_head(void);


#endif
