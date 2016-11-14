/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Tyle <tyle@allwinnertech.com>
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


#include "dev_disp.h"
#include <timer.h>
#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Protocol/HardwareInterrupt.h>
#include "OSAL/OSAL_Int.h"
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

static disp_drv_info g_disp_drv;
static u32 init_flag;

#define MY_BYTE_ALIGN(x) ( ( (x + (4*1024-1)) >> 12) << 12)             /* alloc based on 4K byte */

static u32 suspend_output_type[3] = {0,0,0};
static u32 suspend_status = 0;//0:normal; suspend_status&1 != 0:in early_suspend; suspend_status&2 != 0:in suspend;

//boot plat
static u32    lcd_flow_cnt[2] = {0};
static s8   lcd_op_finished[2] = {0};
static s8   lcd_op_start[2] = {0};

#if defined (CONFIG_ARCH_SUN9IW1P1)
static unsigned int gbuffer[4096];
#endif

static u32 disp_cmd_print = 0xffff;   //print cmd which eq disp_cmd_print

static u32 g_output_type = DISP_OUTPUT_TYPE_LCD;


EFI_EVENT gCheckLcdOpenEvent,gCheckLcdcloseEvent;

VOID
EFIAPI
drv_lcd_open_callback (
  IN  EFI_EVENT   Event,
  IN  VOID        *Context
  );

VOID
EFIAPI
drv_lcd_close_callback (
  IN  EFI_EVENT   Event,
  IN  VOID        *Context
  );
  
static s32 copy_from_user(void *dest, void* src, u32 size)
{
    CopyMem(dest, src, size);
  return 0;
}

static s32 copy_to_user(void *src, void* dest, u32 size)
{
    CopyMem(dest, src, size);
  return 0;
}


void disp_delay_ms(__u32 ms)
{
    /* todo */
    MicroSecondDelay (ms*1000);
}

void disp_delay_us(__u32 us)
{
    MicroSecondDelay (us);
}

// [before][step_0][delay_0][step_1][delay_1]......[step_n-2][delay_n-2][step_n-1][delay_n-1][after]
VOID
EFIAPI
drv_lcd_open_callback (
  IN  EFI_EVENT   Event,
  IN  VOID        *Context
  )
{
    EFI_STATUS Status;
    disp_lcd_flow *flow;
    u32 sel = (u32)Context;
    s32 i = lcd_flow_cnt[sel]++;

    flow = bsp_disp_lcd_get_open_flow(sel);

  if(i < flow->func_num)
    {
      flow->func[i].func(sel);
        if(flow->func[i].delay == 0)
        {
            drv_lcd_open_callback(Event,(void*)sel);
        }
        else
        {
           Status = gBS->SetTimer(
                gCheckLcdOpenEvent,
                TimerRelative,
                (UINT64)(10*1000*(flow->func[i].delay))); // 200 ms
      }
    }
    else if(i == flow->func_num)
    {
        bsp_disp_lcd_post_enable(sel);
        lcd_op_finished[sel] = 1;
    Status = gBS->SetTimer(
        gCheckLcdOpenEvent,
        TimerCancel,
        (UINT64)(0)); // 0 ms 

       Status = gBS->CloseEvent (gCheckLcdOpenEvent);
  ASSERT_EFI_ERROR (Status); 
    }
}


static s32 drv_lcd_enable(u32 sel)
{
    EFI_STATUS  Status;
  if(bsp_disp_lcd_is_used(sel)) {
    lcd_flow_cnt[sel] = 0;
    lcd_op_finished[sel] = 0;
    lcd_op_start[sel] = 1;

        Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL | EVT_TIMER,
                    TPL_CALLBACK,
                    drv_lcd_open_callback,
                    (VOID *)sel,
                    &gCheckLcdOpenEvent);
        ASSERT_EFI_ERROR (Status);  
    bsp_disp_lcd_pre_enable(sel);
    drv_lcd_open_callback(gCheckLcdOpenEvent,(void*)sel);
  }
    return 0;
}

static s8 drv_lcd_check_open_finished(u32 sel)
{
  if(bsp_disp_lcd_is_used(sel) && (lcd_op_start[sel] == 1))
  {
      if(lcd_op_finished[sel])
      {
          //del_timer(&lcd_timer[sel]);
            lcd_op_start[sel] = 0;
      }
    return lcd_op_finished[sel];
  }

  return 1;
}

 void drv_lcd_close_callback( IN EFI_EVENT Event, void *parg)
{
    EFI_STATUS Status=EFI_SUCCESS;
    disp_lcd_flow *flow;
    u32 sel = (__u32)parg;
    s32 i = lcd_flow_cnt[sel]++;

    flow = bsp_disp_lcd_get_close_flow(sel);

    if(i < flow->func_num)
    {
      flow->func[i].func(sel);
        if(flow->func[i].delay == 0)
        {
            drv_lcd_close_callback(Event,(void*)sel);
        }
        else
        {
            Status = gBS->SetTimer(
                  gCheckLcdcloseEvent,
                  TimerRelative,
                  (UINT64)(10*1000*(flow->func[i].delay))); // 200 ms
        }
    }
    else if(i == flow->func_num)
    {
        bsp_disp_lcd_post_disable(sel);
        lcd_op_finished[sel] = 1;
        Status = gBS->SetTimer(
        gCheckLcdcloseEvent,
        TimerCancel,
        (UINT64)(0)); // 0 ms 
       Status = gBS->CloseEvent (gCheckLcdcloseEvent);
       
    }
    ASSERT_EFI_ERROR (Status);  
}

static s32 drv_lcd_disable(u32 sel)
{
    if(bsp_disp_lcd_is_used(sel))
    {
        lcd_flow_cnt[sel] = 0;
        lcd_op_finished[sel] = 0;
        lcd_op_start[sel] = 1;

        //init_timer(&lcd_timer[sel]);

        bsp_disp_lcd_pre_disable(sel);
        drv_lcd_close_callback(gCheckLcdOpenEvent, (void*)sel);
    }

    return 0;
}

static s8 drv_lcd_check_close_finished(u32 sel)
{
    if(bsp_disp_lcd_is_used(sel) && (lcd_op_start[sel] == 1))
    {
        if(lcd_op_finished[sel])
        {
            //del_timer(&lcd_timer[sel]);
            lcd_op_start[sel] = 0;
        }
        return lcd_op_finished[sel];
    }
    return 1;
}

#if defined(CONFIG_ARCH_SUN9IW1P1)
s32 disp_set_hdmi_func(u32 screen_id, disp_hdmi_func * func)
{
  return bsp_disp_set_hdmi_func(screen_id, func);
}
#endif

extern s32 bsp_disp_delay_ms(u32 ms);

extern s32 bsp_disp_delay_us(u32 us);

extern __s32 Hdmi_init(void);



s32 drv_disp_init(void)
{
#ifdef CONFIG_FPGA
    return 0;
#else
    __disp_bsp_init_para para;

    OSAL_InterruptInit();
  sunxi_pwm_init();

  ZeroMem(&para, sizeof(__disp_bsp_init_para));

#if defined(CONFIG_ARCH_SUN9IW1P1)
  para.reg_base[DISP_MOD_BE0]    = BE0_BASE;
  para.reg_size[DISP_MOD_BE0]    = 0x9fc;
  para.reg_base[DISP_MOD_BE1]    = BE1_BASE;
  para.reg_size[DISP_MOD_BE1]    = 0x9fc;
  para.reg_base[DISP_MOD_BE2]    = BE2_BASE;
  para.reg_size[DISP_MOD_BE2]    = 0x9fc;
  para.reg_base[DISP_MOD_FE0]    = FE0_BASE;
  para.reg_size[DISP_MOD_FE0]    = 0x22c;
  para.reg_base[DISP_MOD_FE1]    = FE1_BASE;
  para.reg_size[DISP_MOD_FE1]    = 0x22c;
  para.reg_base[DISP_MOD_FE2]    = FE2_BASE;
  para.reg_size[DISP_MOD_FE2]    = 0x22c;
  para.reg_base[DISP_MOD_LCD0]   = LCD0_BASE;
  para.reg_size[DISP_MOD_LCD0]   = 0x3fc;
  para.reg_base[DISP_MOD_LCD1]   = LCD1_BASE;
  para.reg_size[DISP_MOD_LCD1]   = 0x3fc;
  para.reg_base[DISP_MOD_CCMU]   = CCMPLL_BASE;
  para.reg_size[DISP_MOD_CCMU]   = 0x2dc;
  para.reg_base[DISP_MOD_PIOC]   = PIO_BASE;
  para.reg_size[DISP_MOD_PIOC]   = 0x27c;
  para.reg_base[DISP_MOD_PWM]    = PWM03_BASE;
  para.reg_size[DISP_MOD_PWM]    = 0x3c;
  para.reg_base[DISP_MOD_DEU0]   = DEU0_BASE;
  para.reg_size[DISP_MOD_DEU0]   = 0x60;
  para.reg_base[DISP_MOD_DEU1]   = DEU1_BASE;
  para.reg_size[DISP_MOD_DEU1]   = 0x60;
  para.reg_base[DISP_MOD_CMU0]   = BE0_BASE;
  para.reg_size[DISP_MOD_CMU0]   = 0xfc;
  para.reg_base[DISP_MOD_CMU1]   = BE1_BASE;
  para.reg_size[DISP_MOD_CMU1]   = 0xfc;
  para.reg_base[DISP_MOD_DRC0]   = DRC0_BASE;
  para.reg_size[DISP_MOD_DRC0]   = 0xfc;
  para.reg_base[DISP_MOD_DRC1]   = DRC1_BASE;
  para.reg_size[DISP_MOD_DRC1]   = 0xfc;
  para.reg_base[DISP_MOD_DSI0]   = MIPI_DSI0_BASE;
  para.reg_size[DISP_MOD_DSI0]   = 0x2fc;
  para.reg_base[DISP_MOD_DSI0_DPHY]   = MIPI_DSI0_DPHY_BASE;
  para.reg_size[DISP_MOD_DSI0_DPHY]   = 0xfc;
  para.reg_base[DISP_MOD_HDMI]   = HDMI_BASE;
  para.reg_size[DISP_MOD_HDMI]   = 0xfc;
  para.reg_base[DISP_MOD_TOP]   = REGS_AHB2_BASE;
  para.reg_size[DISP_MOD_TOP]   = 0xfc;

  para.irq_no[DISP_MOD_BE0]         = AW_IRQ_DEBE0;
  para.irq_no[DISP_MOD_BE1]         = AW_IRQ_DEBE1;
  para.irq_no[DISP_MOD_BE2]         = AW_IRQ_DEBE2;
  para.irq_no[DISP_MOD_FE0]         = AW_IRQ_DEFE0;
  para.irq_no[DISP_MOD_FE1]         = AW_IRQ_DEFE1;
  para.irq_no[DISP_MOD_DRC0]        = AW_IRQ_DRC01;
  para.irq_no[DISP_MOD_DRC1]        = AW_IRQ_DEU01;
  para.irq_no[DISP_MOD_LCD0]        = AW_IRQ_LCD0;
  para.irq_no[DISP_MOD_LCD1]        = AW_IRQ_LCD1;
  para.irq_no[DISP_MOD_DSI0]        = AW_IRQ_MIPIDSI;
  para.irq_no[DISP_MOD_EDP]         = AW_IRQ_EDP;
#elif defined(CONFIG_ARCH_SUN8IW5P1)
  para.reg_base[DISP_MOD_BE0]    = DEBE0_BASE;
  para.reg_size[DISP_MOD_BE0]    = 0xfc;
  para.reg_base[DISP_MOD_FE0]    = DEFE0_BASE;
  para.reg_size[DISP_MOD_FE0]    = 0x22c;
  para.reg_base[DISP_MOD_LCD0]   = LCD0_BASE;
  para.reg_size[DISP_MOD_LCD0]   = 0x3fc;
  para.reg_base[DISP_MOD_DRC0]   = DRC0_BASE;
  para.reg_size[DISP_MOD_DRC0]   = 0xfc;
  para.reg_base[DISP_MOD_DSI0]   = MIPI_DSI0_BASE;
  para.reg_size[DISP_MOD_DSI0]   = 0x2fc;
  para.reg_base[DISP_MOD_DSI0_DPHY]   = MIPI_DSI0PHY_BASE;
  para.reg_size[DISP_MOD_DSI0_DPHY]   = 0xfc;
  para.reg_base[DISP_MOD_CCMU]   = CCM_BASE;
  para.reg_size[DISP_MOD_CCMU]   = 0x2dc;
  para.reg_base[DISP_MOD_PIOC]   = PIO_BASE;
  para.reg_size[DISP_MOD_PIOC]   = 0x27c;
  para.reg_base[DISP_MOD_PWM]    = PWM03_BASE;
  para.reg_size[DISP_MOD_PWM]    = 0x3c;
  para.reg_base[DISP_MOD_WB0]   = DRC0_BASE+ 0x200;
  para.reg_size[DISP_MOD_WB0]   = 0x2fc;
  para.reg_base[DISP_MOD_SAT0]   = SAT0_BASE;
  para.reg_size[DISP_MOD_SAT0]   = 0x2fc;

  para.irq_no[DISP_MOD_BE0]         = AW_IRQ_DEBE0;
  para.irq_no[DISP_MOD_LCD0]        = AW_IRQ_LCD0;
  para.irq_no[DISP_MOD_DSI0]        = AW_IRQ_MIPIDSI;
#endif

  ZeroMem(&g_disp_drv,  sizeof(disp_drv_info));

  bsp_disp_init(&para);
#if ((defined CONFIG_SUN6I) || (defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN9IW1P1))
  Hdmi_init();
#endif
  bsp_disp_open();

  lcd_init();

  init_flag = 1;

  __inf("DRV_DISP_Init end\n");
  return 0;
#endif
}

s32 drv_disp_exit(void)
{
  if(init_flag == 1) {
    init_flag = 0;
    bsp_disp_close();
    bsp_disp_exit(g_disp_drv.exit_mode);
  }
  return 0;
}

extern s32 dsi_clk_enable(u32 sel, u32 en);
extern s32 dsi_dcs_wr(u32 sel,u8 cmd,u8* para_p,u32 para_num);
int sunxi_disp_get_source_ops(struct sunxi_disp_source_ops *src_ops)
{
  src_ops->sunxi_lcd_delay_ms = bsp_disp_lcd_delay_ms;
  src_ops->sunxi_lcd_delay_us = bsp_disp_lcd_delay_us;
  src_ops->sunxi_lcd_tcon_enable = bsp_disp_lcd_tcon_enable;
  src_ops->sunxi_lcd_tcon_disable = bsp_disp_lcd_tcon_disable;
  src_ops->sunxi_lcd_pwm_enable = bsp_disp_lcd_pwm_enable;
  src_ops->sunxi_lcd_pwm_disable = bsp_disp_lcd_pwm_disable;
  src_ops->sunxi_lcd_backlight_enable = bsp_disp_lcd_backlight_enable;
  src_ops->sunxi_lcd_backlight_disable = bsp_disp_lcd_backlight_disable;
  src_ops->sunxi_lcd_power_enable = bsp_disp_lcd_power_enable;
  src_ops->sunxi_lcd_power_disable = bsp_disp_lcd_power_disable;
  src_ops->sunxi_lcd_set_panel_funs = bsp_disp_lcd_set_panel_funs;
  src_ops->sunxi_lcd_dsi_write = dsi_dcs_wr;
  src_ops->sunxi_lcd_dsi_clk_enable = dsi_clk_enable;
  src_ops->sunxi_lcd_pin_cfg = bsp_disp_lcd_pin_cfg;
  src_ops->sunxi_lcd_gpio_set_value = bsp_disp_lcd_gpio_set_value;
  src_ops->sunxi_lcd_gpio_set_direction = bsp_disp_lcd_gpio_set_direction;
  return 0;
}

long disp_ioctl(void *hd, unsigned int cmd, void *arg)
{
  unsigned long karg[4];
  unsigned long ubuffer[4] = {0};
  s32 ret = 0;
  int num_screens = 2;

  num_screens = bsp_disp_feat_get_num_screens();

  if (copy_from_user((void*)karg,(void*)arg, 4*sizeof(unsigned long))) {
    __wrn("copy_from_user fail\n");
    return -1;
  }

  ubuffer[0] = *(unsigned long*)karg;
  ubuffer[1] = (*(unsigned long*)(karg+1));
  ubuffer[2] = (*(unsigned long*)(karg+2));
  ubuffer[3] = (*(unsigned long*)(karg+3));

  if(cmd < DISP_CMD_FB_REQUEST) {
    if(ubuffer[0] >= num_screens) {
      __wrn("para err in disp_ioctl, cmd = 0x%x,screen id = %d\n", cmd, (int)ubuffer[0]);
      return -1;
    }
  }
  if(DISPLAY_DEEP_SLEEP == suspend_status) {
    __wrn("ioctl:%x fail when in suspend!\n", cmd);
    return -1;
  }

  if(cmd == disp_cmd_print) {
//    OSAL_PRINTF("cmd:0x%x,%ld,%ld\n",cmd, ubuffer[0], ubuffer[1]);
  }

  switch(cmd) {
  //----disp global----
  case DISP_CMD_SET_BKCOLOR:
  {
    disp_color_info para;

    if(copy_from_user(&para, (void*)ubuffer[1],sizeof(disp_color_info)))  {
      __wrn("copy_from_user fail\n");
      return  -1;
    }
    ret = bsp_disp_set_back_color(ubuffer[0], &para);
    break;
  }

  case DISP_CMD_GET_OUTPUT_TYPE:
    if(DISPLAY_NORMAL == suspend_status)  {
      ret =  bsp_disp_get_output_type(ubuffer[0]);
    } else {
      ret = suspend_output_type[ubuffer[0]];
    }

    break;

  case DISP_CMD_GET_SCN_WIDTH:
    ret = bsp_disp_get_screen_width(ubuffer[0]);
    break;

  case DISP_CMD_GET_SCN_HEIGHT:
    ret = bsp_disp_get_screen_height(ubuffer[0]);
    break;

  case DISP_CMD_SHADOW_PROTECT:
    ret = bsp_disp_shadow_protect(ubuffer[0], ubuffer[1]);
    break;

  case DISP_CMD_VSYNC_EVENT_EN:
    ret = bsp_disp_vsync_event_enable(ubuffer[0], ubuffer[1]);
    break;

  //----layer----
  case DISP_CMD_LAYER_ENABLE:
    ret = bsp_disp_layer_enable(ubuffer[0], ubuffer[1]);
    break;

  case DISP_CMD_LAYER_DISABLE:
    ret = bsp_disp_layer_disable(ubuffer[0], ubuffer[1]);
    break;

  case DISP_CMD_LAYER_SET_INFO:
  {
    disp_layer_info para;

    if(copy_from_user(&para, (void*)ubuffer[2],sizeof(disp_layer_info))) {
      return  -1;
    }
    ret = bsp_disp_layer_set_info(ubuffer[0], ubuffer[1], &para);

    break;
  }

  case DISP_CMD_LAYER_GET_INFO:
  {
    disp_layer_info para;

    ret = bsp_disp_layer_get_info(ubuffer[0], ubuffer[1], &para);
    if(copy_to_user((void*)ubuffer[2],&para, sizeof(disp_layer_info))) {
      __wrn("copy_to_user fail\n");
      return  -1;
    }
    break;
  }

  case DISP_CMD_LAYER_GET_FRAME_ID:
    ret = bsp_disp_layer_get_frame_id(ubuffer[0], ubuffer[1]);
    break;

  //----lcd----
  case DISP_CMD_LCD_ENABLE:
    ret = drv_lcd_enable(ubuffer[0]);
    suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_LCD;

    break;

  case DISP_CMD_LCD_DISABLE:
    ret = drv_lcd_disable(ubuffer[0]);
    suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_NONE;
    break;

  case DISP_CMD_LCD_SET_BRIGHTNESS:
    ret = bsp_disp_lcd_set_bright(ubuffer[0], ubuffer[1]);
    break;

  case DISP_CMD_LCD_GET_BRIGHTNESS:
    ret = bsp_disp_lcd_get_bright(ubuffer[0]);
    break;

  case DISP_CMD_LCD_BACKLIGHT_ENABLE:
    if(DISPLAY_NORMAL == suspend_status) {
      ret = bsp_disp_lcd_backlight_enable(ubuffer[0]);
    }
    break;

  case DISP_CMD_LCD_BACKLIGHT_DISABLE:
    if(DISPLAY_NORMAL == suspend_status) {
      ret = bsp_disp_lcd_backlight_disable(ubuffer[0]);
    }
    break;

#if (defined CONFIG_ARCH_SUN9IW1P1)
  //----hdmi----
  case DISP_CMD_HDMI_ENABLE:
    ret = bsp_disp_hdmi_enable(ubuffer[0]);
    suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_HDMI;
    break;

  case DISP_CMD_HDMI_DISABLE:
    ret = bsp_disp_hdmi_disable(ubuffer[0]);
    suspend_output_type[ubuffer[0]] = DISP_OUTPUT_TYPE_NONE;
    break;

  case DISP_CMD_HDMI_SET_MODE:
    ret = bsp_disp_hdmi_set_mode(ubuffer[0], ubuffer[1]);
    break;

  case DISP_CMD_HDMI_GET_MODE:
    ret = bsp_disp_hdmi_get_mode(ubuffer[0]);
    break;

        case DISP_CMD_HDMI_SUPPORT_MODE:
    ret = bsp_disp_hdmi_check_support_mode(ubuffer[0], ubuffer[1]);
    break;
#endif
    //----enhance----
    case DISP_CMD_SET_BRIGHT:
    ret = bsp_disp_smcl_set_bright(ubuffer[0], ubuffer[1]);
    break;

  case DISP_CMD_GET_BRIGHT:
    ret = bsp_disp_smcl_get_bright(ubuffer[0]);
    break;

  case DISP_CMD_SET_CONTRAST:
    ret = bsp_disp_smcl_set_contrast(ubuffer[0], ubuffer[1]);
    break;

  case DISP_CMD_GET_CONTRAST:
    ret = bsp_disp_smcl_get_contrast(ubuffer[0]);
    break;

  case DISP_CMD_SET_SATURATION:
    ret = bsp_disp_smcl_set_saturation(ubuffer[0], ubuffer[1]);
    break;

  case DISP_CMD_GET_SATURATION:
    ret = bsp_disp_smcl_get_saturation(ubuffer[0]);
    break;

  case DISP_CMD_SET_HUE:
    ret = bsp_disp_smcl_set_hue(ubuffer[0], ubuffer[1]);
    break;

  case DISP_CMD_GET_HUE:
    ret = bsp_disp_smcl_get_hue(ubuffer[0]);
    break;

  case DISP_CMD_ENHANCE_ENABLE:
    ret = bsp_disp_smcl_enable(ubuffer[0]);
    break;

  case DISP_CMD_ENHANCE_DISABLE:
    ret = bsp_disp_smcl_disable(ubuffer[0]);
    break;

  case DISP_CMD_GET_ENHANCE_EN:
    ret = bsp_disp_smcl_is_enabled(ubuffer[0]);
    break;

  case DISP_CMD_SET_ENHANCE_MODE:
    ret = bsp_disp_smcl_set_mode(ubuffer[0], ubuffer[1]);
    break;

  case DISP_CMD_GET_ENHANCE_MODE:
    ret = bsp_disp_smcl_get_mode(ubuffer[0]);
    break;

  case DISP_CMD_SET_ENHANCE_WINDOW:
  {
    disp_window para;

    if(copy_from_user(&para, (void*)ubuffer[1],sizeof(disp_window))) {
      __wrn("copy_from_user fail\n");
      return  -1;
    }
    ret = bsp_disp_smcl_set_window(ubuffer[0], &para);
    break;
  }

  case DISP_CMD_GET_ENHANCE_WINDOW:
  {
    disp_window para;

    ret = bsp_disp_smcl_get_window(ubuffer[0], &para);
    if(copy_to_user((void*)ubuffer[1],&para, sizeof(disp_window))) {
      __wrn("copy_to_user fail\n");
      return  -1;
    }
    break;
  }

  case DISP_CMD_DRC_ENABLE:
    ret = bsp_disp_smbl_enable(ubuffer[0]);
    break;

  case DISP_CMD_DRC_DISABLE:
    ret = bsp_disp_smbl_disable(ubuffer[0]);
    break;

  case DISP_CMD_GET_DRC_EN:
    ret = bsp_disp_smbl_is_enabled(ubuffer[0]);
    break;

  case DISP_CMD_DRC_SET_WINDOW:
  {
    disp_window para;

    if(copy_from_user(&para, (void*)ubuffer[1],sizeof(disp_window))) {
      __wrn("copy_from_user fail\n");
      return  -1;
    }
    ret = bsp_disp_smbl_set_window(ubuffer[0], &para);
    break;
  }

  case DISP_CMD_DRC_GET_WINDOW:
  {
    disp_window para;

    ret = bsp_disp_smbl_get_window(ubuffer[0], &para);
    if(copy_to_user((void*)ubuffer[1],&para, sizeof(disp_window))) {
      __wrn("copy_to_user fail\n");
      return  -1;
    }
    break;
  }

#if defined(CONFIG_ARCH_SUN9IW1P1)
  //---- cursor ----
  case DISP_CMD_CURSOR_ENABLE:
    ret =  bsp_disp_cursor_enable(ubuffer[0]);
    break;

  case DISP_CMD_CURSOR_DISABLE:
    ret =  bsp_disp_cursor_disable(ubuffer[0]);
    break;

  case DISP_CMD_CURSOR_SET_POS:
  {
    disp_position para;

    if(copy_from_user(&para, (void*)ubuffer[1],sizeof(disp_position)))  {
      __wrn("copy_from_user fail\n");
      return  -1;
    }
    ret = bsp_disp_cursor_set_pos(ubuffer[0], &para);
    break;
  }

  case DISP_CMD_CURSOR_GET_POS:
  {
    disp_position para;

    ret = bsp_disp_cursor_get_pos(ubuffer[0], &para);
    if(copy_to_user((void*)ubuffer[1],&para, sizeof(disp_position)))  {
      __wrn("copy_to_user fail\n");
      return  -1;
    }
    break;
  }

  case DISP_CMD_CURSOR_SET_FB:
  {
    disp_cursor_fb para;

    if(copy_from_user(&para, (void*)ubuffer[1],sizeof(disp_cursor_fb))) {
      __wrn("copy_from_user fail\n");
      return  -1;
    }
    ret = bsp_disp_cursor_set_fb(ubuffer[0], &para);
    break;
  }

  case DISP_CMD_CURSOR_SET_PALETTE:
    if((ubuffer[1] == 0) || ((int)ubuffer[3] <= 0)) {
      __wrn("para invalid in display ioctrl DISP_CMD_HWC_SET_PALETTE_TABLE,buffer:0x%x, size:0x%x\n", (unsigned int)ubuffer[1], (unsigned int)ubuffer[3]);
      return -1;
    }
    if(copy_from_user(gbuffer, (void*)ubuffer[1],ubuffer[3])) {
      __wrn("copy_from_user fail\n");
      return  -1;
    }
    ret = bsp_disp_cursor_set_palette(ubuffer[0], (void*)gbuffer, ubuffer[2], ubuffer[3]);
    break;
#endif

  case DISP_CMD_SET_EXIT_MODE:
        ret = g_disp_drv.exit_mode = ubuffer[0];
    break;

  case DISP_CMD_LCD_CHECK_OPEN_FINISH:
    ret = drv_lcd_check_open_finished(ubuffer[0]);
    break;

  case DISP_CMD_LCD_CHECK_CLOSE_FINISH:
    ret = drv_lcd_check_close_finished(ubuffer[0]);
    break;

  default:
    break;
  }

  return ret;
}

#define  DELAY_ONCE_TIME   (50)
#define BOOT_MOD_ENTER_STANDBY (0)
#define BOOT_MOD_EXIT_STANDBY   (1)
s32 drv_disp_standby(u32 cmd, void *pArg)
{
  s32 ret;
  s32 timedly = 5000;
  s32 check_time = timedly/DELAY_ONCE_TIME;

  if(cmd == BOOT_MOD_ENTER_STANDBY)
  {
      if(g_output_type == DISP_OUTPUT_TYPE_HDMI)
      {
    }
    else
        {
            drv_lcd_disable(0);
    }
    do
    {
      ret = drv_lcd_check_close_finished(0);
      if(ret == 1)
      {
        break;
      }
      else if(ret == -1)
      {
        return -1;
      }
      disp_delay_ms(DELAY_ONCE_TIME);
      check_time --;
      if(check_time <= 0)
      {
        return -1;
      }
    }
    while(1);

    return 0;
  }
  else if(cmd == BOOT_MOD_EXIT_STANDBY)
  {
    if(g_output_type == DISP_OUTPUT_TYPE_HDMI)
    {
    }
    else
    {
      drv_lcd_enable(0);
        }

    do
    {
      ret = drv_lcd_check_open_finished(0);
      if(ret == 1)
      {
        break;
      }
      else if(ret == -1)
      {
        return -1;
      }
      disp_delay_ms(DELAY_ONCE_TIME);
      check_time --;
      if(check_time <= 0)
      {
        return -1;
      }
    }
    while(1);

    return 0;
  }

  return -1;
}
