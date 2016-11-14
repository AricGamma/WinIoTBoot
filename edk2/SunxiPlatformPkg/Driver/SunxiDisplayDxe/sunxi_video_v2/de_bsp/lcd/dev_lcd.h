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

#ifndef __DEV_LCD_H__
#define __DEV_LCD_H__

#include "lcd_panel_cfg.h"
extern int sunxi_disp_get_source_ops(struct sunxi_disp_source_ops *src_ops);

extern struct sunxi_lcd_drv g_lcd_drv;

struct sunxi_lcd_drv
{
  struct sunxi_disp_source_ops      src_ops;
};

int lcd_init(void);

#endif
