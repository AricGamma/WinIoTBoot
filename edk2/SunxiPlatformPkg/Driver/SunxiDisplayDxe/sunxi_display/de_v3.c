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

#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>
#include <Library/SysConfigLib.h>
#include <Sunxi_type/sunxi_display_v3.h>
#include <de_v3.h>
#include <Sunxi_type/PrivateDef.h>



#define DISP_DEBUG_ENABLE
#define DISP_DEBUG_MSG(ARG...)  DEBUG (( EFI_D_ERROR,"[Disp Msg]:"ARG))

#ifdef DISP_DEBUG_ENABLE
#define DISP_DEBUG_INFO(ARG...)  DEBUG (( EFI_D_INFO,"[Disp Info]:"ARG))
#define DISP_DEBUG_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[Disp Error]:"ARG))
#else
#define DISP_DEBUG_INFO(ARG...)  
#define DISP_DEBUG_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[Disp Error]:"ARG))
#endif


static __u32 screen_id = 0;
//static __u32 disp_para = 0;
extern long disp_ioctl(void *hd, unsigned int cmd, void *arg);

extern unsigned int    g_layer_para;
extern unsigned int    g_layer_para1;

extern unsigned int    g_layer_hd;

int board_display_layer_request(void)
{
  g_layer_hd = 0;
  return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_layer_release(void)
{
  return 0;

}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_wait_lcd_open(void)
{
  int ret;
  int timedly = 5000;
  int check_time = timedly/50;
  u32 arg[4] = { 0 };
  u32 cmd = 0;

  cmd = DISP_LCD_CHECK_OPEN_FINISH;

  do
  {
      ret = disp_ioctl(NULL, cmd, (void*)arg);
    if(ret == 1)    //open already
    {
      break;
    }
    else if(ret == -1)  //open falied
    {
      return -1;
    }
    MicroSecondDelay(50*1000);
    check_time --;
    if(check_time <= 0)
    {
      return -1;
    }
  }
  while(1);

  return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_wait_lcd_close(void)
{
  int ret;
  int timedly = 5000;
  int check_time = timedly/50;
  u32 arg[4] = { 0 };
  u32 cmd = 0;

  cmd = DISP_LCD_CHECK_CLOSE_FINISH;

  do
  {
      ret = disp_ioctl(NULL, cmd, (void*)arg);
    if(ret == 1)    //open already
    {
      break;
    }
    else if(ret == -1)  //open falied
    {
      return -1;
    }
    MicroSecondDelay(50*1000);
    check_time --;
    if(check_time <= 0)
    {
      return -1;
    }
  }
  while(1);

  return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_set_exit_mode(int lcd_off_only)
{
  u32 arg[4] = { 0 };
  u32 cmd = 0;
  cmd = DISP_SET_EXIT_MODE;

  if(lcd_off_only)
  {
    arg[0] = DISP_EXIT_MODE_CLEAN_PARTLY;
    disp_ioctl(NULL, cmd, (void *)arg);
  }
  else
  {
    cmd = DISP_LCD_DISABLE;
    disp_ioctl(NULL, cmd, (void *)arg);
    board_display_wait_lcd_close();
  }

  return 0;
}
/*
*******************************************************************************
*                     board_display_layer_open
*
* Description:
*    open layer
*
* Parameters:
*    Layer_hd    :  input. handle of layer
*
* Return value:
*    0  :  success
*   !0  :  fail
*
* note:
*    void
*
*******************************************************************************
*/
int board_display_layer_open(unsigned int ScreenId)
{
  u32 arg[4];
  struct disp_layer_config *config;

    if(ScreenId == 0)
    {
      config = (struct disp_layer_config *)g_layer_para;
      arg[0] = ScreenId;
      arg[1] = (unsigned long)config;
      arg[2] = 1;
      arg[3] = 0;
    }
  else if(ScreenId == 1)
  {
      config = (struct disp_layer_config *)g_layer_para1;
      arg[0] = ScreenId;
      arg[1] = (unsigned long)config;
      arg[2] = 1;
      arg[3] = 0;   
  }
  else
  {
    DISP_DEBUG_ERROR("srceen_id invalid\n");
    return 0;
  }
  
  disp_ioctl(NULL,DISP_LAYER_GET_CONFIG,(void*)arg);
  config->enable = 1;
  disp_ioctl(NULL,DISP_LAYER_SET_CONFIG,(void*)arg);

  return 0;
}


/*
*******************************************************************************
*                     board_display_layer_close
*
* Description:
*    open layer
*
* Parameters:
*    
*
* Return value:
*    0  :  success
*   !0  :  fail
*
* note:
*    void
*
*******************************************************************************
*/

int board_display_layer_close(unsigned int ScreenId)
{
  u32 arg[4];
  struct disp_layer_config *config;

    if(ScreenId == 0)
    {
        config = (struct disp_layer_config *)g_layer_para;
      arg[0] = ScreenId;
      arg[1] = (unsigned long)config;
      arg[2] = 1;
      arg[3] = 0;
    }
  else if(ScreenId == 1)
  {
        config = (struct disp_layer_config *)g_layer_para1;
      arg[0] = ScreenId;
      arg[1] = (unsigned long)config;
      arg[2] = 1;
      arg[3] = 0;
  }
  else
  {
    DISP_DEBUG_ERROR("srceen_id invalid\n");
    return 0;
  }
  
  disp_ioctl(NULL,DISP_LAYER_GET_CONFIG,(void*)arg);
  config->enable = 0;
  disp_ioctl(NULL,DISP_LAYER_SET_CONFIG,(void*)arg);

  return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_layer_para_set(unsigned int ScreenId)
{
  u32 arg[4];

    if(ScreenId == 0)
    {
      arg[0] = ScreenId;
      arg[1] = g_layer_para;
      arg[2] = 1;
      arg[3] = 0;
      disp_ioctl(NULL,DISP_LAYER_SET_CONFIG,(void*)arg);
    }
  else if(ScreenId == 1)
  {
      arg[0] = ScreenId;
      arg[1] = g_layer_para1;
      arg[2] = 1;
      arg[3] = 0;
      disp_ioctl(NULL,DISP_LAYER_SET_CONFIG,(void*)arg);    
  }
  else
  {
    DISP_DEBUG_ERROR("srceen_id invalid\n");
  }
  
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_show_until_lcd_open(int display_source)
{
  DISP_DEBUG_MSG("%s\n", __func__);
  if(!display_source)
  {
    board_display_wait_lcd_open();
  }
  board_display_layer_para_set(0);
  board_display_layer_open(0);

  return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_show(int display_source)
{
  board_display_layer_para_set(display_source);
  board_display_layer_open(display_source);

  return 0;

}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_get_width(unsigned int ScreenId)
{
    u32 arg[4];
    arg[0] = ScreenId;
    return disp_ioctl(NULL, DISP_GET_SCN_WIDTH, (void*)arg);
  
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_get_heigth(unsigned int ScreenId)
{
    u32 arg[4];
    arg[0] = ScreenId;
    return disp_ioctl(NULL, DISP_GET_SCN_HEIGHT, (void*)arg);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_framebuffer_set(int width, int height, int bitcount, void *buffer)
{

  struct disp_layer_config *layer_para;
  u32 screen_width, screen_height;
  u32 arg[4];
  u32 full = 0;

  if(!g_layer_para)
  {
    layer_para = (struct disp_layer_config *)AllocatePool(sizeof(struct disp_layer_config));
    if(!layer_para)
    {
      DISP_DEBUG_ERROR("sunxi display error: unable to malloc memory for layer\n");

      return -1;
    }
  }
  else
  {
    layer_para = (struct disp_layer_config *)g_layer_para;
  }

  if(script_parser_fetch("boot_disp", "output_full", (INT32*)&full, 1))
  {
    DISP_DEBUG_ERROR("fetch script data boot_disp.output_full fail\n");
  }

  arg[0] = DISP_LCD_SCREEN_ID;
  screen_width = disp_ioctl(NULL, DISP_GET_SCN_WIDTH, (void*)arg);
  screen_height = disp_ioctl(NULL, DISP_GET_SCN_HEIGHT, (void*)arg);
  DISP_DEBUG_MSG("screen_id =%d, screen_width =%d, screen_height =%d\n", 0, screen_width, screen_height);
  memset((void *)layer_para, 0, sizeof(struct disp_layer_config));
  layer_para->info.fb.addr[0]   = (u32)buffer;
  DISP_DEBUG_MSG("frame buffer address %x\n", (u32)buffer);
  layer_para->channel = 1;
  layer_para->layer_id = 0;
  layer_para->info.fb.format    = (bitcount == 24)? DISP_FORMAT_RGB_888:DISP_FORMAT_ARGB_8888;
  layer_para->info.fb.size[0].width = width;
  layer_para->info.fb.size[0].height  = height;
  layer_para->info.fb.crop.x  = 0;
  layer_para->info.fb.crop.y  = 0;
  layer_para->info.fb.crop.width  = ((unsigned long long)width) << 32;
  layer_para->info.fb.crop.height = ((unsigned long long)height) << 32;
  layer_para->info.fb.flags = DISP_BF_NORMAL;
  layer_para->info.fb.scan = DISP_SCAN_PROGRESSIVE;
  DISP_DEBUG_MSG("bitcount = %d\n", bitcount);
  layer_para->info.mode     = LAYER_MODE_BUFFER;
  layer_para->info.alpha_mode    = 1;
  layer_para->info.alpha_value   = 0xff;
  if(full) {
    layer_para->info.screen_win.x = 0;
    layer_para->info.screen_win.y = 0;
    layer_para->info.screen_win.width = screen_width;
    layer_para->info.screen_win.height  = screen_height;
  } else {
    layer_para->info.screen_win.x = (screen_width - width) / 2;
    layer_para->info.screen_win.y = (screen_height - height) / 2;
    layer_para->info.screen_win.width = width;
    layer_para->info.screen_win.height  = height;
  }
  layer_para->info.b_trd_out    = 0;
  layer_para->info.out_trd_mode   = 0;
  g_layer_para = (u32)layer_para;

  return 0;
}

int board_display_framebuffer_set_hdmi(int width, int height, int bitcount, void *buffer)
{

  struct disp_layer_config *layer_para;
  u32 screen_width, screen_height;
  u32 arg[4];
  u32 full = 0;

  if(!g_layer_para1)
  {
    layer_para = (struct disp_layer_config *)AllocatePool(sizeof(struct disp_layer_config));
    if(!layer_para)
    {
      DISP_DEBUG_ERROR("sunxi display error: unable to malloc memory for layer\n");

      return -1;
    }
  }
  else
  {
    layer_para = (struct disp_layer_config *)g_layer_para1;
  }

  if(script_parser_fetch("boot_disp", "output_full", (INT32*)&full, 1))
  {
    DISP_DEBUG_ERROR("fetch script data boot_disp.output_full fail\n");
  }

  arg[0] = DISP_HDMI_SCREEN_ID;
  screen_width = disp_ioctl(NULL, DISP_GET_SCN_WIDTH, (void*)arg);
  screen_height = disp_ioctl(NULL, DISP_GET_SCN_HEIGHT, (void*)arg);
  DISP_DEBUG_MSG("screen_id =%d, screen_width =%d, screen_height =%d\n", 1, screen_width, screen_height);
  memset((void *)layer_para, 0, sizeof(struct disp_layer_config));
  layer_para->info.fb.addr[0]   = (u32)buffer;
  DISP_DEBUG_MSG("frame buffer address %x\n", (u32)buffer);
  layer_para->channel = 1;
  layer_para->layer_id = 0;
  layer_para->info.fb.format    = (bitcount == 24)? DISP_FORMAT_RGB_888:DISP_FORMAT_ARGB_8888;
  layer_para->info.fb.size[0].width = width;
  layer_para->info.fb.size[0].height  = height;
  layer_para->info.fb.crop.x  = 0;
  layer_para->info.fb.crop.y  = 0;
  layer_para->info.fb.crop.width  = ((unsigned long long)width) << 32;
  layer_para->info.fb.crop.height = ((unsigned long long)height) << 32;
  layer_para->info.fb.flags = DISP_BF_NORMAL;
  layer_para->info.fb.scan = DISP_SCAN_PROGRESSIVE;
  DISP_DEBUG_MSG("bitcount = %d\n", bitcount);
  layer_para->info.mode     = LAYER_MODE_BUFFER;
  layer_para->info.alpha_mode    = 1;
  layer_para->info.alpha_value   = 0xff;
  if(full) {
    layer_para->info.screen_win.x = 0;
    layer_para->info.screen_win.y = 0;
    layer_para->info.screen_win.width = screen_width;
    layer_para->info.screen_win.height  = screen_height;
  } else {
    layer_para->info.screen_win.x = (screen_width - width) / 2;
    layer_para->info.screen_win.y = (screen_height - height) / 2;
    layer_para->info.screen_win.width = width;
    layer_para->info.screen_win.height  = height;
  }
  layer_para->info.b_trd_out    = 0;
  layer_para->info.out_trd_mode   = 0;
  g_layer_para1 = (u32)layer_para;

  return 0;
}


void board_display_set_alpha_mode(int mode)
{
  if(!g_layer_para)
  {
    return;
  }

  struct disp_layer_config *layer_para;
  layer_para = (struct disp_layer_config *)g_layer_para;
  layer_para->info.alpha_mode = mode;

}

int board_display_framebuffer_change(void *buffer)
{
  return 0;
}

int setup_cursor(int width, int height,void *buffer)
{
  return 0;
}

int board_display_device_open(void)
{

  int  value = 1;
  int  ret = 0;
  u32 output_type = 0;
  u32 output_mode = 0;
  u32 auto_hpd = 0;
  u32 err_count = 0;
  unsigned long arg[4] = {0};


  DISP_DEBUG_MSG("De_OpenDevice\n");
  /* getproc output_disp, indicate which disp channel will be using */
  if(script_parser_fetch("boot_disp", "output_full", (INT32*)&screen_id, 1))
  {
    DISP_DEBUG_MSG("fetch script data boot_disp.output_disp fail\n");
    err_count ++;
  } else
    printf("boot_disp.output_disp=%d\n", screen_id);

  /* getproc output_type, indicate which kind of device will be using */
  if(script_parser_fetch("boot_disp", "output_type", (INT32*)&value, 1))
  {
    DISP_DEBUG_MSG("fetch script data boot_disp.output_type fail\n");
    err_count ++;
  } else
    DISP_DEBUG_MSG("boot_disp.output_type=%d\n", value);

  if(value == 0)
  {
    output_type = DISP_OUTPUT_TYPE_NONE;
  }
  else if(value == 1)
  {
    output_type = DISP_OUTPUT_TYPE_LCD;
  }
  else if(value == 2)
  {
    output_type = DISP_OUTPUT_TYPE_TV;
  }
  else if(value == 3)
  {
    output_type = DISP_OUTPUT_TYPE_HDMI;
  }
  else if(value == 4)
  {
    output_type = DISP_OUTPUT_TYPE_VGA;
  }
  else
  {
    DISP_DEBUG_MSG("invalid output_type %d\n", value);
    return -1;
  }
  
  /* getproc output_mode, indicate which kind of mode will be output */
  if(script_parser_fetch("boot_disp", "output_mode", (INT32*)&output_mode, 1))
  {
    DISP_DEBUG_MSG("fetch script data boot_disp.output_mode fail\n");
    err_count ++;
  } else
    DISP_DEBUG_MSG("boot_disp.output_mode=%d\n", output_mode);

  /* getproc auto_hpd, indicate output device decided by the hot plug status of device */
  if(script_parser_fetch("boot_disp", "auto_hpd", (INT32*)&output_mode, 1))
  {
    DISP_DEBUG_MSG("fetch script data boot_disp.auto_hpd fail\n");
    err_count ++;
  } else
    DISP_DEBUG_MSG("boot_disp.auto_hpd=%d\n", auto_hpd);


  if(err_count >= 4)//no boot_disp config
  {
    output_type = DISP_OUTPUT_TYPE_LCD;
  }
  else//has boot_disp config
  {

  }
  DISP_DEBUG_MSG("disp%d device type(%d) enable\n", screen_id, output_type);

  arg[0] = screen_id;
  arg[1] = output_type;
  arg[2] = output_mode;
  disp_ioctl(NULL, DISP_DEVICE_SWITCH, (void *)arg);
  return ret;
}

int board_display_device_open_lcd(unsigned int ScreenId)
{
  int  ret = 0;
  u32 output_type = 0;
  u32 output_mode = 0;
  u32 err_count = 0;
  unsigned long arg[4] = {0};


  DISP_DEBUG_MSG("%a\n", __FUNCTION__);

  /* getproc output_mode, indicate which kind of mode will be output */
  if(script_parser_fetch("boot_disp", "output_mode", (INT32*)&output_mode, 1))
  {
    DISP_DEBUG_MSG("fetch script data boot_disp.output_mode fail\n");
    err_count ++;
  } else
    DISP_DEBUG_MSG("boot_disp.output_mode=%d\n", output_mode);

  DISP_DEBUG_MSG("disp%d device type(%d) enable\n", ScreenId, output_type);

  arg[0] = ScreenId;
  arg[1] = DISP_OUTPUT_TYPE_LCD;
  arg[2] = output_mode;
  disp_ioctl(NULL, DISP_DEVICE_SWITCH, (void *)arg);
  return ret;
}

int board_display_device_open_hdmi(unsigned int ScreenId)
{
  int  ret = 0;
  u32 output_type = 0;
  u32 output_mode = 0;
  u32 err_count = 0;
  unsigned long arg[4] = {0};


  DISP_DEBUG_MSG("%a\n", __FUNCTION__);

  /* getproc output_mode, indicate which kind of mode will be output */
  if(script_parser_fetch("boot_disp", "output_mode", (INT32*)&output_mode, 1))
  {
    DISP_DEBUG_MSG("fetch script data boot_disp.output_mode fail\n");
    err_count ++;
  } else
    DISP_DEBUG_MSG("boot_disp.output_mode=%d\n", output_mode);

  DISP_DEBUG_MSG("disp%d device type(%d) enable\n", ScreenId, output_type);

  arg[0] = ScreenId;
  arg[1] = DISP_OUTPUT_TYPE_HDMI;
  arg[2] = output_mode;
  disp_ioctl(NULL, DISP_DEVICE_SWITCH, (void *)arg);
  return ret;
}


