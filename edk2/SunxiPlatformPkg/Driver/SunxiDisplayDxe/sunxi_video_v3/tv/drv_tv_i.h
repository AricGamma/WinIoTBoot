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

#ifndef  _DRV_TV_I_H_
#define  _DRV_TV_I_H_

#include <asm/arch/sunxi_display2.h>
#include "de_tvec_i.h"
#include "../disp_sys_intf.h"
#include "../dev_disp.h"

typedef unsigned char       u8;
typedef signed char         s8;
typedef unsigned short      u16;
typedef signed short        s16;
typedef unsigned int        u32;
typedef signed int          s32;
typedef unsigned long long  u64;

typedef struct
{
  disp_tv_dac_source    dac_source[4];
  disp_tv_mode          tv_mode;
  u32                     base_address;
  u32           sid;
  u32           cali_offset;
}tv_screen_t;


typedef struct
{
    u32                 enable;
  u32         dac_count;
  tv_screen_t     screen[2];
}tv_info_t;

extern tv_info_t g_tv_info;

s32 tv_init(void);

s32 tv_exit(void);

s32 tv_get_mode(u32 sel);

s32 tv_set_mode(u32 sel, disp_tv_mode tv_mod);

s32 tv_get_input_csc(void);

s32 tv_get_video_timing_info(u32 sel, disp_video_timings **video_info);

s32 tv_enable(u32 sel);

s32 tv_disable(u32 sel);

s32 tv_suspend(void);

s32 tv_resume(void);

s32 tv_mode_support(disp_tv_mode mode);

s32 tv_get_dac_hpd(u32 sel);

#endif
