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

#ifndef __HDMI_HW_INTERFACE_H_
#define __HDMI_HW_INTERFACE_H_

struct sunxi_audio_para
{
    __s32   audio_en;
    __s32   sample_rate;
    __s32   channel_num;

    __s32   CTS;
    __s32   ACR_N;
    __s32   CH_STATUS0;
    __s32   CH_STATUS1;
    __u8    data_raw;   /*0:pcm;1:raw*/
    __u8    sample_bit;/* 8/16/20/24/32 bit */
    __u8    ca; /* channel allocation */
};

struct sunxi_video_para
{
    __u32 vic;
    __u8  is_hdmi;
    __u8  is_yuv;
    __u8  is_hdcp;
};

typedef void (*hdmi_udelay) (unsigned long us);

int api_set_func(hdmi_udelay udelay);
void sunxi_video_config(struct sunxi_video_para *info);
void sunxi_audio_config(struct sunxi_audio_para *info);
int sunxi_get_hpd(void);
int sunxi_ddc_read(char cmd,char pointer,char offset,int nbyte,char * pbuf);
void sunxi_set_reg_base(unsigned int address);
int sunxi_hdmi_enter_lp(void);
void sunxi_hdcp_rst(void);
void sunxi_hdcp_hdl(void);

#endif /* API_H_ */

