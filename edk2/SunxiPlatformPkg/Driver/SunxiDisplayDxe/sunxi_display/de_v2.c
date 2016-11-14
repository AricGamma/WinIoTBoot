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

#include <drv_display.h>
#include <Library/TimerLib.h>
#include <Library/BaseMemoryLib.h>
#include<../sunxi_video_v2/de_bsp/de/bsp_display.h>
#include <Library/MemoryAllocationLib.h>

extern unsigned int    g_layer_para,g_layer_para1;
extern unsigned int    g_layer_hd,g_layer_hd1,g_layer_hd2;
extern long disp_ioctl(void *hd, unsigned int cmd, void *arg);

int board_display_layer_request(__u32 screen_id)
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
int board_display_layer_release(unsigned int screen_id)
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
  __u32 arg[4] = { 0 };

  do
  {
      ret = disp_ioctl(NULL, DISP_CMD_LCD_CHECK_OPEN_FINISH, (void*)arg);
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
  __u32 arg[4] = { 0 };

  do
  {
      ret = disp_ioctl(NULL, DISP_CMD_LCD_CHECK_CLOSE_FINISH, (void*)arg);
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
  __u32 arg[4] = { 0 };

  if(lcd_off_only)
  {
    arg[0] = DISP_EXIT_MODE_CLEAN_PARTLY;
    disp_ioctl(NULL, DISP_CMD_SET_EXIT_MODE, (void *)arg);
  }
  else
  {
    disp_ioctl(NULL, DISP_CMD_LCD_DISABLE, (void *)arg);
    board_display_wait_lcd_close();
  }

  return 0;
}
/*
*******************************************************************************
*                     board_display_layer_open
*
* Description:
*    ´ò¿ªÍ¼²ã
*
* Parameters:
*    Layer_hd    :  input. Í¼²ã¾ä±ú
*
* Return value:
*    0  :  ³É¹¦
*   !0  :  Ê§°Ü
*
* note:
*    void
*
*******************************************************************************
*/
int board_display_layer_open(unsigned int screen_id)
{
    __u32 arg[4];

if(screen_id == 0)
{
  arg[0] = screen_id;
  arg[1] = g_layer_hd;
  arg[2] = 0;
  arg[3] = 0;
  disp_ioctl(NULL,DISP_CMD_LAYER_ENABLE,(void*)arg);
}

else if (screen_id == 1)
  {
  arg[0] = screen_id;
  arg[1] = g_layer_hd1;
  arg[2] = 0;
  arg[3] = 0;
  disp_ioctl(NULL,DISP_CMD_LAYER_ENABLE,(void*)arg);
}
else
  __inf("screen_id invalid\n");
    return 0;
}


/*
*******************************************************************************
*                     board_display_layer_close
*
* Description:
*    ¹Ø±ÕÍ¼²ã
*
* Parameters:
*    Layer_hd    :  input. Í¼²ã¾ä±ú
*
* Return value:
*    0  :  ³É¹¦
*   !0  :  Ê§°Ü
*
* note:
*    void
*
*******************************************************************************
*/
int board_display_layer_close(void)
{
    __u32 arg[4];

  arg[0] = 0;
  arg[1] = g_layer_hd;
  arg[2] = 0;
  arg[3] = 0;
  disp_ioctl(NULL,DISP_CMD_LCD_DISABLE,(void*)arg);

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
int board_display_layer_para_set(unsigned int screen_id)
{
    __u32 arg[4];

if(screen_id ==0)
{
  arg[0] = screen_id;
  arg[1] = 0;
  arg[2] = g_layer_para;
  arg[3] = 0;
  disp_ioctl(NULL,DISP_CMD_LAYER_SET_INFO,(void*)arg);
}
else if(screen_id ==1)
{
  arg[0] = screen_id;
  arg[1] = 0;
  arg[2] = g_layer_para1;
  arg[3] = 0;
  disp_ioctl(NULL,DISP_CMD_LAYER_SET_INFO,(void*)arg);
}
else
  __inf("srceen_id invalid\n");
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
    if(!display_source)
    {
      board_display_wait_lcd_open();
    }
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
int board_display_get_width(int display_source)
{
    u32 arg[4];
    arg[0] = display_source;
    return disp_ioctl(NULL, DISP_CMD_GET_SCN_WIDTH, (void*)arg);
  
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
int board_display_get_heigth(int display_source)
{
    u32 arg[4];
    arg[0] = display_source;
    return disp_ioctl(NULL, DISP_CMD_GET_SCN_HEIGHT, (void*)arg);
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
    disp_layer_info *layer_para;
  u32 screen_width, screen_height;
  u32 arg[4];

  if(!g_layer_para)
  {
    layer_para = (disp_layer_info *)AllocatePool(sizeof(disp_layer_info));
    if(!layer_para)
    {
      __inf("sunxi display error: unable to malloc memory for layer\n");

      return -1;
    }
  }
  else
  {
    layer_para = (disp_layer_info *)g_layer_para;
  }
  arg[0] = 0;
  screen_width = disp_ioctl(NULL, DISP_CMD_GET_SCN_WIDTH, (void*)arg);
  screen_height = disp_ioctl(NULL, DISP_CMD_GET_SCN_HEIGHT, (void*)arg);
    __inf("screen width %d, height %d\n", screen_width,screen_height);
  ZeroMem((void *)layer_para, sizeof(disp_layer_info));
  layer_para->fb.addr[0]    = (u32)buffer;
  __inf("frame buffer address %x\n", (u32)buffer);
  layer_para->fb.format   = (bitcount == 24)? DISP_FORMAT_RGB_888:DISP_FORMAT_ARGB_8888;
  layer_para->fb.size.width = screen_width;
  layer_para->fb.size.height  = screen_height;
  __inf("bitcount = %d\n", bitcount);
  layer_para->fb.b_trd_src  = 0;
  layer_para->fb.trd_mode   = 0;
  layer_para->ck_enable   = 0;
  layer_para->mode            = DISP_LAYER_WORK_MODE_NORMAL;
  layer_para->alpha_mode    = 1;
  layer_para->alpha_value   = 0xff;
  layer_para->pipe      = 0;
  //layer_para->screen_win.x    = (screen_width - width) / 2;
  //layer_para->screen_win.y    = (screen_height - height) / 2;
  //layer_para->screen_win.width  = width;
  //layer_para->screen_win.height = height;
  layer_para->screen_win.x    = 0;
  layer_para->screen_win.y    = 0;
  layer_para->screen_win.width  = screen_width;
  layer_para->screen_win.height = screen_height;
  layer_para->b_trd_out   = 0;
  layer_para->out_trd_mode  = 0;

  g_layer_para = (__u32)layer_para;

  return 0;
}

int board_display_framebuffer_set_hdmi(int width, int height, int bitcount, void *buffer)
{
    #if 1
    disp_layer_info *layer_para;
  __u32 screen_width, screen_height;


  if(!g_layer_para1)
  {
    layer_para = (disp_layer_info *)AllocatePool(sizeof(disp_layer_info));
    if(!layer_para)
    {
      __inf("sunxi display error: unable to malloc memory for layer\n");

      return -1;
    }
  }
  else
  {
    layer_para = (disp_layer_info *)g_layer_para1;
  }
  //screen_width = 1280;
  //screen_height = 720;
  

    screen_width = board_display_get_width(0);
    screen_height = board_display_get_heigth(0);
  
    ZeroMem((void *)layer_para, sizeof(disp_layer_info));
    layer_para->fb.addr[0]      = (u32)buffer;
    __inf("frame buffer address %x\n", (u32)buffer);
    layer_para->fb.format       = (bitcount == 24)? DISP_FORMAT_RGB_888:DISP_FORMAT_ARGB_8888;
    layer_para->fb.size.width   = screen_width;
    layer_para->fb.size.height  = screen_height;
    __inf("bitcount = %d\n", bitcount);
    layer_para->fb.b_trd_src    = 0;
    layer_para->fb.trd_mode     = 0;
    layer_para->ck_enable       = 0;
    layer_para->mode            = DISP_LAYER_WORK_MODE_NORMAL;
    layer_para->alpha_mode      = 1;
    layer_para->alpha_value     = 0xff;
    layer_para->pipe            = 0;
    layer_para->screen_win.x        = 0;
    layer_para->screen_win.y        = 0;
    layer_para->screen_win.width    = width;
    layer_para->screen_win.height   = height;
    layer_para->b_trd_out       = 0;
    layer_para->out_trd_mode    = 0;


  g_layer_para1 = (__u32)layer_para;
#endif
  return 0;
}

int board_display_framebuffer_change(void *buffer)
{
    
  return 0;
}
int setup_cursor(int width, int height,void *buffer)
{
    #if 0
  __u32 arg[4];
  __disp_layer_info_t *layer_para;
  __u32 bitcount;

//request layer
  arg[0] = 0;
  arg[1] = DISP_LAYER_WORK_MODE_NORMAL;
  g_layer_hd2 = disp_ioctl(NULL, DISP_CMD_LAYER_REQUEST, (void*)arg);
  if(g_layer_hd2 == 0)
  {
        __inf("sunxi display error : display request layer failed\n");

        return -1;
  }

// set fb for cusor

    layer_para = (__disp_layer_info_t *)AllocatePool(sizeof(__disp_layer_info_t));
    if(!layer_para)
    {
      __inf("sunxi display error: unable to malloc memory for layer\n");

      return -1;
    }
  bitcount =32;
  ZeroMem((void *)layer_para,sizeof(__disp_layer_info_t));
  layer_para->fb.addr[0]    = (__u32)buffer;
  __inf("frame buffer address %x\n", (__u32)buffer);
  layer_para->fb.size.width = width;
  layer_para->fb.size.height  = height;
  layer_para->fb.mode     = DISP_MOD_INTERLEAVED;
  layer_para->fb.format   = (bitcount == 24)? DISP_FORMAT_RGB888:DISP_FORMAT_ARGB8888;
  __inf("bitcount = %d\n", bitcount);
  layer_para->fb.br_swap    = 0;
  layer_para->fb.seq      = DISP_SEQ_ARGB;
  layer_para->fb.b_trd_src  = 0;
  layer_para->fb.trd_mode   = 0;
  layer_para->ck_enable   = 0;
  layer_para->mode            = DISP_LAYER_WORK_MODE_NORMAL;
  layer_para->alpha_en    = 0;
  layer_para->alpha_val   = 0xff;
  layer_para->pipe      = 1;
  layer_para->src_win.x   = 0;
  layer_para->src_win.y   = 0;
  layer_para->src_win.width = width;
  layer_para->src_win.height  = height;
  layer_para->scn_win.x   = 0;
  layer_para->scn_win.y   = 0;
  layer_para->scn_win.width = width;
  layer_para->scn_win.height  = height;
  layer_para->b_trd_out   = 0;
  layer_para->out_trd_mode  = 0;
  layer_para->prio            = 0;
// set layer para
  arg[0] = 0;
  arg[1] = g_layer_hd2;
  arg[2] = (__u32)layer_para;
  arg[3] = 0;
  disp_ioctl(NULL,DISP_CMD_LAYER_SET_PARA,(void*)arg);

  arg[0] = 0;
  arg[1] = g_layer_hd2;
  arg[2] = 0;
  arg[3] = 0;
  disp_ioctl(NULL,DISP_CMD_LAYER_OPEN,(void*)arg);
  
  FreePool(layer_para);
  #endif
  return 0;
  
}
int board_display_device_open_lcd(unsigned int screen_id)
{
  int  ret = 0;
  unsigned long arg[4] = {0};
      

  __inf("lcd open\n");
  arg[0] = screen_id;
  arg[1] = 0;
  arg[2] = 0;
  ret = disp_ioctl(NULL, DISP_CMD_LCD_ENABLE, (void*)arg);

  return ret;
}

int board_display_device_open_hdmi(unsigned int screen_id)
{
  int  ret = 0;
  unsigned long arg[4] = {0};
      

    __inf("hdmi open\n");
    arg[0] = screen_id;
    arg[1] = DISP_TV_MOD_720P_50HZ;
    arg[2] = 0;
    disp_ioctl(NULL, DISP_CMD_HDMI_SET_MODE, (void *)arg);
    ret = disp_ioctl(NULL, DISP_CMD_HDMI_ENABLE, (void *)arg);
  return ret;
}


