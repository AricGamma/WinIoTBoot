/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Jerry Wang <wangflord@allwinnertech.com>
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

#ifndef  __SUNXI_DE_H__
#define  __SUNXI_DE_H__

#define DISP_LCD_SCREEN_ID     0
#define DISP_HDMI_SCREEN_ID     1

extern int board_display_layer_request(void);

extern int board_display_layer_release(void);

extern int board_display_layer_open(unsigned int ScreenId);

extern int board_display_layer_close(unsigned int ScreenId);

extern int board_display_layer_para_set(unsigned int ScreenId);

extern int board_display_show(int display_source);

extern int board_display_framebuffer_set(int width, int height, int bitcount, void *buffer);

extern int board_display_framebuffer_change(void *buffer);

extern int board_display_device_open(void);

extern int board_display_get_width(unsigned int ScreenId);

extern int board_display_get_heigth(unsigned int ScreenId);

int board_display_device_open_lcd(unsigned int ScreenId);

int board_display_device_open_hdmi(unsigned int ScreenId);

extern int board_display_framebuffer_set_hdmi(int width, int height, int bitcount, void *buffer);


#endif   //__SUNXI_DE_H__
