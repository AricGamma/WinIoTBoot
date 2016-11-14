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

#ifndef  __LT070ME05000_PANEL_H__
#define  __LT070ME05000_PANEL_H__

#include "panels.h"

extern __lcd_panel_t lt070me05000_panel;

extern s32 dsi_dcs_wr_0para(u32 sel,u8 cmd);
extern s32 dsi_dcs_wr_1para(u32 sel,u8 cmd,u8 para);
extern s32 dsi_dcs_wr_2para(u32 sel,u8 cmd,u8 para1,u8 para2);
extern s32 dsi_dcs_wr_3para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3);
extern s32 dsi_dcs_wr_4para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3,u8 para4);
extern s32 dsi_dcs_wr_5para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3,u8 para4,u8 para5);
extern s32 dsi_gen_wr_0para(u32 sel,u8 cmd);
extern s32 dsi_gen_wr_1para(u32 sel,u8 cmd,u8 para);
extern s32 dsi_gen_wr_2para(u32 sel,u8 cmd,u8 para1,u8 para2);
extern s32 dsi_gen_wr_3para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3);
extern s32 dsi_gen_wr_4para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3,u8 para4);
extern s32 dsi_gen_wr_5para(u32 sel,u8 cmd,u8 para1,u8 para2,u8 para3,u8 para4,u8 para5);

#endif
