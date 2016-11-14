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


#ifndef _SUNXI_RTC_H_
#define _SUNXI_RTC_H_

/* Rtc */
struct sunxi_rtc_regs {
  volatile UINT32 losc_ctrl;
  volatile UINT32 losc_auto_swt_status;
  volatile UINT32 clock_prescalar;
  volatile UINT32 res0[1];
  volatile UINT32 yymmdd;
  volatile UINT32 hhmmss;
  volatile UINT32 res1[2];
  volatile UINT32 alarm0_counter;
  volatile UINT32 alarm0_current_value;
  volatile UINT32 alarm0_enable;
  volatile UINT32 alarm0_irq_enable;
  volatile UINT32 alarm0_irq_status;
  volatile UINT32 res2[3];
  volatile UINT32 alarm1_wk_hms;
  volatile UINT32 alarm1_enable;
  volatile UINT32 alarm1_irq_enable;
  volatile UINT32 alarm1_irq_status;
  volatile UINT32 alarm_config;
};

#define SUNXI_LOSC_CTRL_REG                 (0x0)

#define SUNXI_RTC_DATE_REG                  (0x0010)
#define SUNXI_RTC_TIME_REG                  (0x0014)

#define SUNXI_RTC_ALARM_COUNTER_REG       (0x0020)
#define SUNXI_RTC_ALARM_CURRENT_REG       (0x0024)
#define SUNXI_ALARM_EN_REG                  (0x0028)
#define SUNXI_ALARM_INT_CTRL_REG            (0x002c)
#define SUNXI_ALARM_INT_STATUS_REG          (0x0030)
#define SUNXI_ALARM_CONFIG          (0x0050)

/*rtc count interrupt control*/
#define RTC_ALARM_COUNT_INT_EN        0x00000100

#define RTC_ENABLE_CNT_IRQ              0x00000001

/*Crystal Control*/
#define REG_LOSCCTRL_MAGIC            0x16aa0000
#define REG_CLK32K_AUTO_SWT_EN        (0x00004000)
#define RTC_SOURCE_EXTERNAL             0x00000001
#define RTC_HHMMSS_ACCESS               0x00000100
#define RTC_YYMMDD_ACCESS               0x00000080
#define EXT_LOSC_GSM                      (0x00000008)

/*Date Value*/
#define DATE_GET_DAY_VALUE(x)           ((x) &0x0000001f)
#define DATE_GET_MON_VALUE(x)           (((x)&0x00000f00) >> 8 )
#define DATE_GET_YEAR_VALUE(x)          (((x)&0x003f0000) >> 16)

#define DATE_SET_DAY_VALUE(x)           DATE_GET_DAY_VALUE(x)
#define DATE_SET_MON_VALUE(x)           (((x)&0x0000000f) << 8 )
#define DATE_SET_YEAR_VALUE(x)          (((x)&0x0000003f) << 16)
#define LEAP_SET_VALUE(x)               (((x)&0x00000001) << 22)

/*Time Value*/
#define TIME_GET_SEC_VALUE(x)           ((x) &0x0000003f)
#define TIME_GET_MIN_VALUE(x)           (((x)&0x00003f00) >> 8 )
#define TIME_GET_HOUR_VALUE(x)          (((x)&0x001f0000) >> 16)

#define TIME_SET_SEC_VALUE(x)           TIME_GET_SEC_VALUE(x)
#define TIME_SET_MIN_VALUE(x)           (((x)&0x0000003f) << 8 )
#define TIME_SET_HOUR_VALUE(x)          (((x)&0x0000001f) << 16)


#endif

