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

#ifndef __EBIOS_LCDC_TVE_H_
#define __EBIOS_LCDC_TVE_H_

#include "../bsp_display.h"

typedef enum
{
    LCD_IRQ_TCON0_VBLK  = 15,
    LCD_IRQ_TCON1_VBLK = 14,
    LCD_IRQ_TCON0_LINE = 13,
    LCD_IRQ_TCON1_LINE = 12,
    LCD_IRQ_TCON0_TRIF = 11,
    LCD_IRQ_TCON0_CNTR = 10,
}__lcd_irq_id_t;

typedef enum
{
    DSI_IRQ_VIDEO_LINE  = 3,
    DSI_IRQ_VIDEO_VBLK  = 2,
    DSI_IRQ_INSTR_STEP  = 1,
    DSI_IRQ_INSTR_END   = 0,
}__dsi_irq_id_t;

typedef enum
{
  EDP_IRQ_VBLK  = 0,
  EDP_IRQ_LINE1 = 1,
  EDP_IRQ_EMPTY = 2,
}__edp_irq_id_t;

typedef enum
{
    LCD_SRC_BE0   = 0,
    LCD_SRC_BE1   = 1,
    LCD_SRC_DMA888  = 2,
    LCD_SRC_DMA565  = 3,
    LCD_SRC_BLACK = 4,
    LCD_SRC_WHITE = 5,
    LCD_SRC_BLUE  = 6,
}__lcd_src_t;

#define TVE_D0ActFlags  (0x01)
#define TVE_D1ActFlags  (0x01<<1)
#define TVE_D2ActFlags  (0x01<<2)
#define TVE_D3ActFlags  (0x01<<3)

typedef enum
{
    TVE_MODE_NTSC = 0,
    TVE_MODE_PAL,
    TVE_MODE_480I,
    TVE_MODE_576I,
    TVE_MODE_480P,
    TVE_MODE_576P,
    TVE_MODE_720P_50HZ,
    TVE_MODE_720P_60HZ,
    TVE_MODE_1080I_50HZ,
    TVE_MODE_1080I_60HZ,
    TVE_MODE_1080P_50HZ,
    TVE_MODE_1080P_60HZ,
    TVE_MODE_VGA
}__tve_mode_t;

typedef enum tag_TVE_DAC
{
    DAC1 = 1, //bit0
    DAC2 = 2, //bit1
    DAC3 = 4  //bit2
}__tve_dac_t;

typedef enum tag_TVE_SRC
{
    CVBS = 0,
    SVIDEO_Y = 1,
    SVIDEO_C = 2,
    COMPONENT_Y = 4,
    COMPONENT_PB = 5,
    COMPONENT_PR = 6,
    VGA_R = 4,
    VGA_G = 5,
    VGA_B = 6
}__tve_src_t;

s32   hmid_src_sel(u32 sel);
s32   dsi_src_sel(u32 sel);
s32   lvds_open(u32 sel, disp_panel_para * panel);
s32   lvds_close(u32 sel);


s32   tcon_set_reg_base(u32 sel, u32 address);
u32   tcon_get_reg_base(u32 sel);
s32   tcon_init(u32 sel);
s32   tcon_exit(u32 sel);
s32   tcon_get_timing(u32 sel,u32 index,disp_video_timing* tt);
u32   tcon_irq_query(u32 sel,__lcd_irq_id_t id);
u32   tcon_get_start_delay(u32 sel,u32 tcon_index);
u32   tcon_get_cur_line(u32 sel, u32 tcon_index);
s32   tcon_gamma(u32 sel, u32 en,u32 *gamma_tbl);


s32   tcon0_cfg(u32 sel, disp_panel_para * panel);
s32   tcon0_src_select(u32 sel, __lcd_src_t src);
s32 tcon0_src_get(u32 sel);
s32   tcon0_open(u32 sel, disp_panel_para * panel);
s32   tcon0_close(u32 sel);
s32   tcon0_set_dclk_div(u32 sel, u8 div);
u32   tcon0_get_dclk_div(u32 sel);
s32 tcon0_tri_busy(u32 sel);
s32   tcon0_tri_start(u32 sel);
s32 tcon0_cpu_wr_24b(u32 sel, u32 index, u32 data);
s32 tcon0_cpu_wr_24b_index(u32 sel, u32 index);
s32 tcon0_cpu_wr_24b_data(u32 sel, u32 data);
s32 tcon0_cpu_rd_24b(u32 sel, u32 index, u32 *data);
s32 tcon0_cpu_wr_16b(u32 sel, u32 index, u32 data);
s32 tcon0_cpu_wr_16b_index(u32 sel, u32 index);
s32 tcon0_cpu_wr_16b_data(u32 sel, u32 data);
s32 tcon0_cpu_rd_16b(u32 sel, u32 index, u32 *data);

s32   tcon1_open(u32 sel);
s32   tcon1_close(u32 sel);
s32 tcon1_set_hdmi_mode(u32 sel, disp_video_timing *timming);
s32   tcon1_src_select(u32 sel, __lcd_src_t src);
s32   tcon1_src_get(u32 sel);

#include "de_dsi.h"
#endif

