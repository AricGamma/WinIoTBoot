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

//#include <malloc.h>
#include <Sun8iW1/drv_display.h>
#include <Library/TimerLib.h>
#include <Library/BaseMemoryLib.h>
#include<../video_sunxi/de_bsp/bsp_display.h>
#include <Library/MemoryAllocationLib.h>

extern unsigned int    g_layer_para,g_layer_para1;
extern unsigned int    g_layer_hd,g_layer_hd1,g_layer_hd2;
extern long disp_ioctl(void *hd, unsigned int cmd, void *arg);

int board_display_layer_request(__u32 screen_id)
{
  __u32 arg[4];

  arg[0] = screen_id;
  arg[1] = DISP_LAYER_WORK_MODE_NORMAL;
      if(screen_id == 0)
        {
  g_layer_hd = disp_ioctl(NULL, DISP_CMD_LAYER_REQUEST, (void*)arg);
  if(g_layer_hd == 0)
  {
        __inf("sunxi display error : display request layer failed\n");

        return -1;
  }
        }
  else if(screen_id == 1)
  {
  g_layer_hd1 = disp_ioctl(NULL, DISP_CMD_LAYER_REQUEST, (void*)arg);
  if(g_layer_hd1 == 0)
  {
        __inf("sunxi display error : display request layer failed\n");

        return -1;
  }
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
int board_display_layer_release(unsigned int screen_id)
{
  __u32 arg[4];

  if(g_layer_hd == 0)
  {
        __inf("sunxi display error : display layer is NULL\n");

        return -1;
  }

  arg[0] = screen_id;
  arg[1] = g_layer_hd;

  return disp_ioctl(NULL, DISP_CMD_LAYER_RELEASE, (void*)arg);
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
    disp_ioctl(NULL, DISP_CMD_LCD_OFF, (void *)arg);
    board_display_wait_lcd_close();
  }

  return 0;
}
/*
*******************************************************************************
*                     board_display_layer_open
*
* Description:
*    ��ͼ��
*
* Parameters:
*    Layer_hd    :  input. ͼ����
*
* Return value:
*    0  :  �ɹ�
*   !0  :  ʧ��
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
  disp_ioctl(NULL,DISP_CMD_LAYER_OPEN,(void*)arg);
}

else if (screen_id == 1)
  {
  arg[0] = screen_id;
  arg[1] = g_layer_hd1;
  arg[2] = 0;
  arg[3] = 0;
  disp_ioctl(NULL,DISP_CMD_LAYER_OPEN,(void*)arg);
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
*    �ر�ͼ��
*
* Parameters:
*    Layer_hd    :  input. ͼ����
*
* Return value:
*    0  :  �ɹ�
*   !0  :  ʧ��
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
  disp_ioctl(NULL,DISP_CMD_LAYER_CLOSE,(void*)arg);

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
  arg[1] = g_layer_hd;
  arg[2] = g_layer_para;
  arg[3] = 0;
  disp_ioctl(NULL,DISP_CMD_LAYER_SET_PARA,(void*)arg);
}
else if(screen_id ==1)
{
  arg[0] = screen_id;
  arg[1] = g_layer_hd1;
  arg[2] = g_layer_para1;
  arg[3] = 0;
  disp_ioctl(NULL,DISP_CMD_LAYER_SET_PARA,(void*)arg);
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
int board_display_framebuffer_set(int width, int height, int bitcount, void *buffer)
{
    __disp_layer_info_t *layer_para;
  __u32 screen_width, screen_height;
  __u32 arg[4];

  if(!g_layer_para)
  {
    layer_para = (__disp_layer_info_t *)AllocatePool(sizeof(__disp_layer_info_t));
    if(!layer_para)
    {
      __inf("sunxi display error: unable to malloc memory for layer\n");

      return -1;
    }
  }
  else
  {
    layer_para = (__disp_layer_info_t *)g_layer_para;
  }

  screen_width = width;
  screen_height = height;
  
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
  layer_para->alpha_en    = 1;
  layer_para->alpha_val   = 0xff;
  layer_para->pipe      = 0;
  layer_para->src_win.x   = 0;
  layer_para->src_win.y   = 0;
  layer_para->src_win.width = width;
  layer_para->src_win.height  = height;
  layer_para->b_trd_out   = 0;
  layer_para->out_trd_mode  = 0;
  layer_para->prio            = 1;

  g_layer_para = (__u32)layer_para;

  return 0;
}

int board_display_framebuffer_set_hdmi(int width, int height, int bitcount, void *buffer)
{
    __disp_layer_info_t *layer_para;
  __u32 screen_width, screen_height;

  if(!g_layer_para1)
  {
    layer_para = (__disp_layer_info_t *)AllocatePool(sizeof(__disp_layer_info_t));
    if(!layer_para)
    {
      __inf("sunxi display error: unable to malloc memory for layer\n");

      return -1;
    }
  }
  else
  {
    layer_para = (__disp_layer_info_t *)g_layer_para1;
  }
  screen_width = 1280;
  screen_height = 720;
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
  layer_para->mode            = DISP_LAYER_WORK_MODE_SCALER;
  layer_para->alpha_en    = 1;
  layer_para->alpha_val   = 0xff;
  layer_para->pipe      = 0;
  layer_para->scn_win.x   = 0;
  layer_para->scn_win.y   = 0;
  layer_para->scn_win.width = screen_width;
  layer_para->scn_win.height  = screen_height;
  layer_para->b_trd_out   = 0;
  layer_para->out_trd_mode  = 0;

  g_layer_para1 = (__u32)layer_para;

  return 0;
}

int board_display_framebuffer_change(void *buffer)
{
    __u32 arg[4];
  __disp_fb_t disp_fb;
  __disp_layer_info_t *layer_para = (__disp_layer_info_t *)g_layer_para;

  arg[0] = 0;
  arg[1] = g_layer_hd;
  arg[2] = (__u32)&disp_fb;
  arg[3] = 0;

  if(disp_ioctl(NULL, DISP_CMD_LAYER_GET_FB, (void*)arg))
  {
    __inf("sunxi display error :get framebuffer failed\n");

    return -1;
  }
  disp_fb.addr[0] = (__u32)buffer;
  arg[0] = 0;
    arg[1] = g_layer_hd;
    arg[2] = (__u32)&disp_fb;
    arg[3] = 0;
  //__inf("try to set framebuffer %x\n", (__u32)buffer);
    if(disp_ioctl(NULL, DISP_CMD_LAYER_SET_FB, (void*)arg))
    {
        __inf("sunxi display error :set framebuffer failed\n");

    return -1;
  }
  layer_para->fb.addr[0] = (__u32)buffer;

  return 0;
}
int setup_cursor(int width, int height,void *buffer)
{
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
    ret = disp_ioctl(NULL, DISP_CMD_LCD_ON, (void*)arg);

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
    ret = disp_ioctl(NULL, DISP_CMD_HDMI_ON, (void *)arg);
  return ret;
}


