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

#ifndef  __DEFAULT_PANEL_H__
#define  __DEFAULT_PANEL_H__

#include "panels.h"
#include "../../de/disp_display.h"
#include "../lcd_source_interface.h"

#define panel_rst(v)    (sunxi_lcd_gpio_set_value(0, 0, v))
extern __lcd_panel_t sl698ph_720p_panel;

#endif
