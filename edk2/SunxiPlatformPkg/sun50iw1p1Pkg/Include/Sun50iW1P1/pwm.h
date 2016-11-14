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

#ifndef __PWM_H__
#define __PWM_H__

#define PWM_NUM 2
#define PWM_REG_NUM 3

#if !defined(bool)
#define bool unsigned char
#endif


enum pwm_polarity {
  PWM_POLARITY_NORMAL,
  PWM_POLARITY_INVERSED,
};

struct pwm_device {
  const char    *label;
  unsigned long   flags;
  unsigned int    hwpwm;
  unsigned int    pwm;
  struct pwm_chip   *chip;
  void      *chip_data;

  unsigned int    period; /* in nanoseconds */
};

struct pwm_chip {
  struct device   *dev;
  struct list_head  list;
  const struct pwm_ops  *ops;
  int     base;
  unsigned int    npwm;

  struct pwm_device *pwms;

  //struct pwm_device * (*of_xlate)(struct pwm_chip *pc,
             // const struct of_phandle_args *args);
  unsigned int    of_pwm_n_cells;
  bool      can_sleep;
};

#if !defined(CONFIG_ARCH_SUN50IW1P1)
/* old version use this function. */
int sunxi_pwm_set_polarity(int pwm, enum pwm_polarity polarity);
int sunxi_pwm_config(int pwm, int duty_ns, int period_ns);
int sunxi_pwm_enable (int pwm);
void sunxi_pwm_disable(int pwm);
void sunxi_pwm_init(void);

#else
/* it mapping uboot2014 verison.now just only SUN50IW1P1 use this function,*/
int pwm_set_polarity(int pwm, enum pwm_polarity polarity);
int pwm_config    (int pwm, int duty_ns, int period_ns);
int pwm_enable    (int pwm);
int pwm_disable   (int pwm);
int pwm_init    (void);
int pwm_request(int pwm, const char *label);
#endif


#endif
