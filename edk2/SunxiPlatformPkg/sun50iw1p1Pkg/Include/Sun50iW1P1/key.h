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


#ifndef _SUNXI_KEY_H
#define _SUNXI_KEY_H


struct sunxi_lradc {
  volatile UINT32 ctrl;         /* lradc control */
  volatile UINT32 intc;         /* interrupt control */
  volatile UINT32 ints;         /* interrupt status */
  volatile UINT32 data0;        /* lradc 0 data */
  volatile UINT32 data1;        /* lradc 1 data */
};

#define SUNXI_KEY_ADC_CRTL        (SUNXI_KEYADC_BASE + 0x00)
#define SUNXI_KEY_ADC_INTC        (SUNXI_KEYADC_BASE + 0x04)
#define SUNXI_KEY_ADC_INTS        (SUNXI_KEYADC_BASE + 0x08)
#define SUNXI_KEY_ADC_DATA0       (SUNXI_KEYADC_BASE + 0x0C)

#define LRADC_EN                  (0x1)   /* LRADC enable */
#define LRADC_SAMPLE_RATE         0x2    /* 32.25 Hz */
#define LEVELB_VOL                0x2    /* 0x33(~1.6v) */
#define LRADC_HOLD_EN             (0x1 << 6)    /* sample hold enable */
#define KEY_MODE_SELECT           0x0    /* normal mode */

#define ADC0_DATA_PENDING         (1 << 0)    /* adc0 has data */
#define ADC0_KEYDOWN_PENDING      (1 << 1)    /* key down */
#define ADC0_HOLDKEY_PENDING      (1 << 2)    /* key hold */
#define ADC0_ALRDY_HOLD_PENDING   (1 << 3)    /* key already hold */
#define ADC0_KEYUP_PENDING        (1 << 4)    /* key up */


extern int sunxi_key_init(void);

extern int sunxi_key_exit(void);

extern int sunxi_key_read(void);

extern int sunxi_key_probe(void);

#endif
