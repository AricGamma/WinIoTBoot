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

#ifndef __DEV_DISP_H__
#define __DEV_DISP_H__

#include "drv_disp_i.h"
#include "de_bsp/lcd/dev_lcd.h"

#define FB_RESERVED_MEM

#define DISPLAY_NORMAL 0
#define DISPLAY_LIGHT_SLEEP 1
#define DISPLAY_DEEP_SLEEP 2

struct info_mm
{
  void *info_base;  /* Virtual address */
  unsigned long mem_start;  /* Start of frame buffer mem, (physical address) */
  u32 mem_len;      /* Length of frame buffer mem */
};

#if 0
struct proc_list
{
  void (*proc)(u32 screen_id);
  struct list_head list;
};
#endif

typedef struct
{
  u32                     reg_base[DISP_MOD_NUM];
  u32                     reg_size[DISP_MOD_NUM];
  u32                     irq_no[DISP_MOD_NUM];

  disp_init_para          disp_init;
}fb_info_t;

typedef struct
{
  u32               exit_mode;//0:clean all  1:disable interrupt
  bool              b_lcd_enabled[3];
}disp_drv_info;

extern s32 drv_disp_init(void);
extern s32 drv_disp_exit(void);

extern int sunxi_disp_get_source_ops(struct sunxi_disp_source_ops *src_ops);
extern long disp_ioctl(void *hd, unsigned int cmd, void *arg);
extern s32 drv_disp_standby(u32 cmd, void *pArg);

extern void disp_delay_us(__u32 us);
extern void disp_delay_ms(__u32 ms);


#endif
