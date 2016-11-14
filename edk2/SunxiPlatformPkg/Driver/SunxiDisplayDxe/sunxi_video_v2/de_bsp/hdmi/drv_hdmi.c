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

#include "hdmi_hal.h"

static bool hdmi_used;
static bool bopen;
static char hdmi_power[25];

extern void disp_delay_ms(__u32 ms);
extern void disp_delay_us(__u32 us);


extern disp_video_timing video_timing[];
void hdmi_delay_ms(__u32 ms)
{
#if defined(__LINUX_PLAT__)
  u32 timeout = ms*HZ/1000;

  set_current_state(TASK_INTERRUPTIBLE);
  schedule_timeout(timeout);
#elif defined(__UBOOT_OSAL__)
    disp_delay_ms(ms);
#endif
  return ;
}

void hdmi_delay_us(unsigned long us)
{
#if defined(__LINUX_PLAT__)
  udelay(us);
#elif defined(__UBOOT_OSAL__)
    disp_delay_us(us);
#endif
  return ;
}

__s32 Hdmi_open(void)
{
  __inf("[Hdmi_open]\n");
#if !defined(__UBOOT_OSAL__)
  Hdmi_hal_video_enable(1);
#else
  __inf("%s\n", __func__);
  Hdmi_hal_video_enable_sync(1);
#endif
  //if(ghdmi.bopen == 0)
  //{
  //  up(run_sem);
  //}
  bopen = 1;
  return 0;
}

__s32 Hdmi_close(void)
{
  __inf("[Hdmi_close]\n");

  Hdmi_hal_video_enable(0);
  bopen = 0;
  return 0;
}

struct disp_hdmi_mode hdmi_mode_tbl[] = {
  {DISP_TV_MOD_480I,                HDMI1440_480I,     },
  {DISP_TV_MOD_576I,                HDMI1440_576I,     },
  //{DISP_TV_MOD_480P,                HDMI480P,          },
  {DISP_TV_MOD_576P,                HDMI576P,          },
  {DISP_TV_MOD_720P_50HZ,           HDMI720P_50,       },
  {DISP_TV_MOD_720P_60HZ,           HDMI720P_60,       },
  {DISP_TV_MOD_1080I_50HZ,          HDMI1080I_50,      },
  {DISP_TV_MOD_1080I_60HZ,          HDMI1080I_60,      },
  {DISP_TV_MOD_1080P_24HZ,          HDMI1080P_24,      },
  {DISP_TV_MOD_1080P_50HZ,          HDMI1080P_50,      },
  {DISP_TV_MOD_1080P_60HZ,          HDMI1080P_60,      },
  {DISP_TV_MOD_1080P_25HZ,          HDMI1080P_25,      },
  {DISP_TV_MOD_1080P_30HZ,          HDMI1080P_30,      },
  {DISP_TV_MOD_1080P_24HZ_3D_FP,    HDMI1080P_24_3D_FP,},
  {DISP_TV_MOD_720P_50HZ_3D_FP,     HDMI720P_50_3D_FP, },
  {DISP_TV_MOD_720P_60HZ_3D_FP,     HDMI720P_60_3D_FP, },
  {DISP_TV_MOD_3840_2160P_30HZ,     HDMI3840_2160P_30, },
  //{DISP_TV_MOD_3840_2160P_24HZ,     HDMI3840_2160P_24, },
  {DISP_TV_MOD_3840_2160P_25HZ,     HDMI3840_2160P_25, },
};

__s32 Hdmi_set_display_mode(disp_tv_mode mode)
{
  __u32 hdmi_mode;
  __u32 i;
  bool find = false;

  __inf("[Hdmi_set_display_mode],mode:%d\n",mode);

  for(i=0; i<sizeof(hdmi_mode_tbl)/sizeof(struct disp_hdmi_mode); i++)
  {
    if(hdmi_mode_tbl[i].mode == mode) {
      hdmi_mode = hdmi_mode_tbl[i].hdmi_mode;
      find = true;
      break;
    }
  }

  if(find) {
    return Hdmi_hal_set_display_mode(hdmi_mode);
  } else {
    __wrn("unsupported video mode %d when set display mode\n", mode);
    return -1;
  }

}

__s32 Hdmi_mode_support(disp_tv_mode mode)
{
  __u32 hdmi_mode;
  __u32 i;
  bool find = false;

  for(i=0; i<sizeof(hdmi_mode_tbl)/sizeof(struct disp_hdmi_mode); i++)
  {
    if(hdmi_mode_tbl[i].mode == mode) {
      hdmi_mode = hdmi_mode_tbl[i].hdmi_mode;
      find = true;
      break;
    }
  }

  if(find) {
    return Hdmi_hal_mode_support(hdmi_mode);
  } else {
    return 0;
  }
}

__s32 Hdmi_get_HPD_status(void)
{
  return Hdmi_hal_get_HPD();
}

__s32 Hdmi_get_video_timming_info(disp_video_timing **video_info)
{
  *video_info = video_timing;
  return 0;
}

int Hdmi_get_video_info_index(u32 mode_id)
{
  u32 i;
  u32 vic = 0;
  u32 ret = 0;
  disp_video_timing *video_info;

  for(i = 0; i < sizeof(hdmi_mode_tbl) / sizeof(struct disp_hdmi_mode); i++) {
    if(hdmi_mode_tbl[i].mode == mode_id) {
      vic = hdmi_mode_tbl[i].hdmi_mode;
      ret = 1;
      break;
    }
  }

  if(ret == 0)
    return -1;

  video_info = video_timing;
  i = 0;

  while(video_info) {
    if(video_info->vic == vic)
      return i;

    i++;
    video_info = video_info + 1;
  }

  return -1;
}


int Hdmi_run_thread(void *parg)
{
  while (1) {

    Hdmi_hal_main_task();

    hdmi_delay_ms(200);
  }

  return 0;
}

extern s32 disp_set_hdmi_func(u32 screen_id, disp_hdmi_func * func);
__s32 Hdmi_init(void)
{
  s32 ret = 0;
  disp_hdmi_func disp_func;
  s32 val;

  hdmi_used = 0;

  ret = script_parser_fetch("hdmi_para", "hdmi_used", &val, 1);
  if(ret == 0) {
    hdmi_used = val;

    if(hdmi_used)
    {
      ret = script_parser_fetch("hdmi_para", "hdmi_power", (int *)hdmi_power, 25);
      if(ret == 0) {
        OSAL_Power_Enable(hdmi_power);
      }
      ret = script_parser_fetch("hdmi_para", "hdmi_cts_compatibility", &val, 1);
      if(ret == 0) {
        Hdmi_hal_cts_enable(val);
        __inf("cts_enable, %d\n", val);
      }
      ret = script_parser_fetch("hdmi_para", "hdmi_hdcp_enable", &val, 1);
      if(ret == 0) {
        Hdmi_hal_hdcp_enable(val);
        __inf("hdcp_enable, %d\n", val);
      }

      Hdmi_hal_init();

      disp_func.hdmi_open = Hdmi_open;
      disp_func.hdmi_close = Hdmi_close;
      disp_func.hdmi_set_mode = Hdmi_set_display_mode;
      disp_func.hdmi_mode_support = Hdmi_mode_support;
      disp_func.hdmi_get_HPD_status = Hdmi_get_HPD_status;
      disp_func.hdmi_get_video_timing_info = Hdmi_get_video_timming_info;
      disp_func.hdmi_get_video_info_index = Hdmi_get_video_info_index;
      disp_set_hdmi_func(0, &disp_func);
      disp_set_hdmi_func(1, &disp_func);
    }
  }
  return 0;
}

__s32 Hdmi_exit(void)
{
  if(hdmi_used) {
    OSAL_Power_Disable(hdmi_power);
    Hdmi_hal_exit();
  }

  return 0;
}
