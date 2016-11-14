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


#ifndef __SUNXI_CODEC_H__
#define __SUNXI_CODEC_H__

struct sunxi_codec_t {
  volatile u32 dac_dpc;
  volatile u32 dac_fifoc;
  volatile u32 dac_fifos;
  volatile u32 dac_txdata;
  volatile u32 dac_actl;
  volatile u32 dac_tune;
  volatile u32 dac_dg;
  volatile u32 adc_fifoc;
  volatile u32 adc_fifos;
  volatile u32 adc_rxdata;
  volatile u32 adc_actl;
  volatile u32 adc_dg;
  volatile u32 dac_cnt;
  volatile u32 adc_cnt;
  volatile u32 ac_dg;
};


extern  int sunxi_codec_init(void);
extern  int sunxi_codec_config(int samplerate, int samplebit, int channel);
extern  int sunxi_codec_start(void *buffer, uint length, uint loop_mode);
extern  int sunxi_codec_stop(void);
extern  int sunxi_codec_wink(void *buffer, uint length);
extern  int sunxi_codec_exit(void);


#endif


