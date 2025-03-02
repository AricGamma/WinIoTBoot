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

#include "lp079x01.h"
#include "panels.h"
#include <Library/BaseLib.h> 
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h> 
#include <Library/MemoryAllocationLib.h>

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

static void LCD_cfg_panel_info(panel_extend_para * info)
{
  u32 i = 0, j=0;
  u32 items;
  u8 lcd_gamma_tbl[][2] =
  {
    //{input value, corrected value}
    {0, 0},
    {15, 15},
    {30, 30},
    {45, 45},
    {60, 60},
    {75, 75},
    {90, 90},
    {105, 105},
    {120, 120},
    {135, 135},
    {150, 150},
    {165, 165},
    {180, 180},
    {195, 195},
    {210, 210},
    {225, 225},
    {240, 240},
    {255, 255},
  };

  u32 lcd_cmap_tbl[2][3][4] = {
  {
    {LCD_CMAP_G0,LCD_CMAP_B1,LCD_CMAP_G2,LCD_CMAP_B3},
    {LCD_CMAP_B0,LCD_CMAP_R1,LCD_CMAP_B2,LCD_CMAP_R3},
    {LCD_CMAP_R0,LCD_CMAP_G1,LCD_CMAP_R2,LCD_CMAP_G3},
    },
    {
    {LCD_CMAP_B3,LCD_CMAP_G2,LCD_CMAP_B1,LCD_CMAP_G0},
    {LCD_CMAP_R3,LCD_CMAP_B2,LCD_CMAP_R1,LCD_CMAP_B0},
    {LCD_CMAP_G3,LCD_CMAP_R2,LCD_CMAP_G1,LCD_CMAP_R0},
    },
  };

  items = sizeof(lcd_gamma_tbl)/2;
  for(i=0; i<items-1; i++) {
    u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

    for(j=0; j<num; j++) {
      u32 value = 0;

      value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
      info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
    }
  }
  info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];

  CopyMem(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static s32 LCD_open_flow(u32 sel)
{
  LCD_OPEN_FUNC(sel, LCD_power_on, 100);   //open lcd power, and delay 50ms
  LCD_OPEN_FUNC(sel, LCD_panel_init, 200);   //open lcd power, than delay 200ms
  LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 200);     //open lcd controller, and delay 100ms
  LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

  return 0;
}

static s32 LCD_close_flow(u32 sel)
{
  LCD_CLOSE_FUNC(sel, LCD_bl_close, 0);       //close lcd backlight, and delay 0ms
  LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);         //close lcd controller, and delay 0ms
  LCD_CLOSE_FUNC(sel, LCD_panel_exit, 200);   //open lcd power, than delay 200ms
  LCD_CLOSE_FUNC(sel, LCD_power_off, 500);   //close lcd power, and delay 500ms

  return 0;
}

static void LCD_power_on(u32 sel)
{
  sunxi_lcd_power_enable(sel, 0);//config lcd_power pin to open lcd power0
  sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
  sunxi_lcd_pin_cfg(sel, 0);
  sunxi_lcd_power_disable(sel, 0);//config lcd_power pin to close lcd power0
}

static void LCD_bl_open(u32 sel)
{
  sunxi_lcd_pwm_enable(sel);//open pwm module
  sunxi_lcd_backlight_enable(sel);//config lcd_bl_en pin to open lcd backlight
}

static void LCD_bl_close(u32 sel)
{
  sunxi_lcd_backlight_disable(sel);//config lcd_bl_en pin to close lcd backlight
  sunxi_lcd_pwm_disable(sel);//close pwm module
}

static void LCD_panel_init(u32 sel)
{
  sunxi_lcd_pin_cfg(sel, 1);
  sunxi_lcd_delay_ms(10);

  panel_rst(0);
  sunxi_lcd_delay_ms(20);
  panel_rst(1);
  sunxi_lcd_delay_ms(10);

  sunxi_lcd_dsi_write(sel,DSI_DCS_EXIT_SLEEP_MODE, 0, 0);
  sunxi_lcd_delay_ms(200);

  sunxi_lcd_dsi_clk_enable(sel);

  sunxi_lcd_dsi_write(sel,DSI_DCS_SET_DISPLAY_ON, 0, 0);
  sunxi_lcd_delay_ms(200);

  return;
}

static void LCD_panel_exit(u32 sel)
{
  sunxi_lcd_dsi_clk_disable(sel);
  panel_rst(0);
  return ;
}

//sel: 0:lcd0; 1:lcd1
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
  return 0;
}

__lcd_panel_t lp079x01_panel = {
  /* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
  .name = "lp079x01",
  .func = {
    .cfg_panel_info = LCD_cfg_panel_info,
    .cfg_open_flow = LCD_open_flow,
    .cfg_close_flow = LCD_close_flow,
    .lcd_user_defined_func = LCD_user_defined_func,
  },
};
