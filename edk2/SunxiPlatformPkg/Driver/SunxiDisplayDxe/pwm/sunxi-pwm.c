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

#include <pwm.h>
#include <Library/SysConfigLib.h>
#include <platform.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>

#define sys_get_wvalue(n)   (*((volatile unsigned int *)(n)))          /* word input */
#define sys_put_wvalue(n,c) (*((volatile unsigned int *)(n))  = (c))   /* word output */
#ifndef abs
#define abs(x) ((x&0x80000000)? (0-x):(x))
#endif

unsigned int pwm_active_sta[4] = {1, 0, 0, 0};
unsigned int pwm_pin_count[4] = {0};

user_gpio_set_t pwm_gpio_info[PWM_NUM][2];

#define sunxi_pwm_debug 0
#undef  sunxi_pwm_debug

#ifdef sunxi_pwm_debug
  #define pwm_debug(fmt,args...)  printf(fmt ,##args)
#else
  #define pwm_debug(fmt,args...)
#endif

unsigned int sunxi_pwm_read_reg(unsigned int offset)
{
  unsigned int value = 0;

  value = sys_get_wvalue(PWM03_BASE + offset);

  return value;
}

unsigned int sunxi_pwm_write_reg(unsigned int offset, unsigned int value)
{
  sys_put_wvalue(PWM03_BASE + offset, value);

  return 0;
}

void sunxi_pwm_get_sys_config(int pwm)
{
  int ret, val;
  char primary_key[25];
  user_gpio_set_t gpio_info[1];

  AsciiSPrint(primary_key,sizeof(primary_key), "pwm%d_para", pwm);
  ret = script_parser_fetch(primary_key, "pwm_used", &val, 1);
  if(ret < 0) {
    pwm_debug("fetch script data fail\n");
  } else {
    if(val == 1) {
      ret = script_parser_fetch(primary_key, "pwm_positive", (int *)&gpio_info, sizeof(user_gpio_set_t) / sizeof(int));
      if(ret < 0) {
        pwm_debug("fetch script data fail\n");
      } else {
        pwm_pin_count[pwm]++;
        CopyMem(&pwm_gpio_info[pwm][0], gpio_info, sizeof(user_gpio_set_t));
      }

      ret = script_parser_fetch(primary_key, "pwm_negative", (int *)&gpio_info, sizeof(user_gpio_set_t) / sizeof(int));
      if(ret < 0) {
        pwm_debug("fetch script data fail\n");
      } else {
        pwm_pin_count[pwm]++;
        CopyMem(&pwm_gpio_info[pwm][1], gpio_info, sizeof(user_gpio_set_t));
      }
    }
  }
}

int sunxi_pwm_set_polarity(int pwm, enum pwm_polarity polarity)
{
  unsigned int temp;

#if ((defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN9IW1P1))
  temp = sunxi_pwm_read_reg(pwm * 0x10);

  if(polarity == PWM_POLARITY_NORMAL) {
    pwm_active_sta[pwm] = 1;
    temp |= 1 << 5;
  } else {
    pwm_active_sta[pwm] = 0;
    temp &= ~(1 << 5);
  }

  sunxi_pwm_write_reg(pwm * 0x10, temp);

#elif (defined CONFIG_ARCH_SUN8IW3P1)

  temp = sunxi_pwm_read_reg(0);
  if(polarity == PWM_POLARITY_NORMAL) {
    pwm_active_sta[pwm] = 1;
    if(pwm == 0)
      temp |= 1 << 5;
    else
      temp |= 1 << 20;
  }else {
    pwm_active_sta[pwm] = 0;
    if(pwm == 0)
      temp &= ~(1 << 5);
    else
      temp &= ~(1 << 20);
  }
  #elif (defined CONFIG_ARCH_SUN7IW1P1)

  temp = sunxi_pwm_read_reg(0x200);
  if(polarity == PWM_POLARITY_NORMAL) {
    pwm_active_sta[pwm] = 1;
    if(pwm == 0)
      temp |= 1 << 5;
    else
      temp |= 1 << 20;
  }else {
    pwm_active_sta[pwm] = 0;
    if(pwm == 0)
      temp &= ~(1 << 5);
    else
      temp &= ~(1 << 20);
  }
  #endif
  return 0;
}

int sunxi_pwm_config(int pwm, int duty_ns, int period_ns)
{
#if ((defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN9IW1P1))

  unsigned int pre_scal[7] = {1, 2, 4, 8, 16, 32, 64};
  unsigned int freq;
  unsigned int pre_scal_id = 0;
  unsigned int entire_cycles = 256;
  unsigned int active_cycles = 192;
  unsigned int entire_cycles_max = 65536;
  unsigned int temp;
  unsigned int calc;

  if(period_ns < 10667)
    freq = 93747;
  else if(period_ns > 174762666) {
    freq = 6;
    calc = period_ns / duty_ns;
    duty_ns = 174762666 / calc;
    period_ns = 174762666;
  }
  else
    freq = 1000000000 / period_ns;

  entire_cycles = 24000000 / freq /pre_scal[pre_scal_id];

  while(entire_cycles > entire_cycles_max) {
    pre_scal_id++;

    if(pre_scal_id > 6)
      break;

    entire_cycles = 24000000 / freq / pre_scal[pre_scal_id];
  }

  if(period_ns < 5*100*1000)
    active_cycles = (duty_ns * entire_cycles + (period_ns/2)) /period_ns;
  else if(period_ns >= 5*100*1000 && period_ns < 6553500)
    active_cycles = ((duty_ns / 100) * entire_cycles + (period_ns /2 / 100)) / (period_ns/100);
  else
    active_cycles = ((duty_ns / 10000) * entire_cycles + (period_ns /2 / 10000)) / (period_ns/10000);

  temp = sunxi_pwm_read_reg(pwm * 0x10);

  temp = (temp & 0xfffffff0) | pre_scal_id;

  sunxi_pwm_write_reg(pwm * 0x10, temp);

  sunxi_pwm_write_reg(pwm * 0x10 + 0x04, ((entire_cycles - 1)<< 16) | active_cycles);

  pwm_debug("PWM _TEST: duty_ns=%d, period_ns=%d, freq=%d, per_scal=%d, period_reg=0x%x\n", duty_ns, period_ns, freq, pre_scal_id, temp);


#elif (defined CONFIG_ARCH_SUN8IW3P1) 

  unsigned int pre_scal[11][2] = {{15, 1}, {0, 120}, {1, 180}, {2, 240}, {3, 360}, {4, 480}, {8, 12000}, {9, 24000}, {10, 36000}, {11, 48000}, {12, 72000}};
  unsigned int freq;
  unsigned int pre_scal_id = 0;
  unsigned int entire_cycles = 256;
  unsigned int active_cycles = 192;
  unsigned int entire_cycles_max = 65536;
  unsigned int temp;

  if(period_ns < 10667)
    freq = 93747;
  else if(period_ns > 1000000000)
    freq = 1;
  else
    freq = 1000000000 / period_ns;

  entire_cycles = 24000000 / freq / pre_scal[pre_scal_id][1];

  while(entire_cycles > entire_cycles_max) {
    pre_scal_id++;

    if(pre_scal_id > 10)
      break;

    entire_cycles = 24000000 / freq / pre_scal[pre_scal_id][1];
  }

  if(period_ns < 5*100*1000)
    active_cycles = (duty_ns * entire_cycles + (period_ns/2)) /period_ns;
  else if(period_ns >= 5*100*1000 && period_ns < 6553500)
    active_cycles = ((duty_ns / 100) * entire_cycles + (period_ns /2 / 100)) / (period_ns/100);
  else
    active_cycles = ((duty_ns / 10000) * entire_cycles + (period_ns /2 / 10000)) / (period_ns/10000);

  temp = sunxi_pwm_read_reg(0);

  if(pwm == 0)
    temp = (temp & 0xfffffff0) |pre_scal[pre_scal_id][0];
  else
    temp = (temp & 0xfff87fff) |pre_scal[pre_scal_id][0];

  sunxi_pwm_write_reg(0, temp);

  sunxi_pwm_write_reg((pwm + 1)  * 0x04, ((entire_cycles - 1)<< 16) | active_cycles);

  pwm_debug("PWM _TEST: duty_ns=%d, period_ns=%d, freq=%d, per_scal=%d, period_reg=0x%x\n", duty_ns, period_ns, freq, pre_scal_id, temp);

#elif (defined CONFIG_ARCH_SUN7IW1P1) 

  __u32 pre_scal[10] = {120, 180, 240, 360, 480, 12000, 24000, 36000, 48000, 72000};
  __u32 pre_scal_id = 0, entire_cycle = 256, active_cycle = 192;
  __u32 i=0, tmp=0;
  __u32 freq;

  freq = 1000000 /period_ns;

  if(freq > 200000)
  {
    pwm_debug("pwm preq is large then 200khz, fix to 200khz\n");
    freq = 200000;
  }

  if(freq > 781)
  {
    pre_scal_id = 0;
    entire_cycle = (24000000 / pre_scal[pre_scal_id] + (freq/2)) / freq;
    pwm_debug("pre_scal:%d, entire_cycle:%d, pwm_freq:%d\n", pre_scal[i], entire_cycle, 24000000 / pre_scal[pre_scal_id] / entire_cycle );
  }
  else
  {
    for(i=0; i<10; i++)
    {
      __u32 pwm_freq = 0;

      pwm_freq = 24000000 / (pre_scal[i] * 256);
      if(abs(pwm_freq - freq) < abs(tmp - freq))
      {
        tmp = pwm_freq;
        pre_scal_id = i;
        entire_cycle = 256;
        pwm_debug("pre_scal:%d, entire_cycle:%d, pwm_freq:%d\n", pre_scal[i], 256, pwm_freq);
        pwm_debug("----%d\n", tmp);
      }
    }
  }
  active_cycle = (duty_ns * entire_cycle + (period_ns/2)) /period_ns;

  if(pre_scal_id >= 5)
  {
    pre_scal_id += 3;
  }

  if(pwm == 0)
  {
    sunxi_pwm_write_reg(0x204, ((entire_cycle - 1)<< 16) | active_cycle);

    tmp = sunxi_pwm_read_reg(0x200) & 0xfffffff0;
    tmp |=  pre_scal_id;//bit6:gatting the special clock for pwm0; bit5:pwm0  active state is high level
    sunxi_pwm_write_reg(0x200,tmp);
  }
  else
  {
    sunxi_pwm_write_reg(0x208, ((entire_cycle - 1)<< 16) | active_cycle);

    tmp = sunxi_pwm_read_reg(0x200) & 0xfff87fff;
    tmp |=  (pre_scal_id<<15);//bit21:gatting the special clock for pwm1; bit20:pwm1  active state is high level
    sunxi_pwm_write_reg(0x200,tmp);
  }

  pwm_debug("PWM _TEST: duty_ns=%d, period_ns=%d, freq=%d, per_scal=%d, period_reg=0x%x\n", duty_ns, period_ns, freq, pre_scal_id, temp);

#endif

  return 0;
}

int sunxi_pwm_enable(int pwm)
{
  unsigned int temp;

#ifndef CONFIG_FPGA_V4_PLATFORM

  int i;
  unsigned int ret = 0;

  for(i = 0; i < pwm_pin_count[pwm]; i++) {
    ret = gpio_request(&pwm_gpio_info[pwm][i], 1);
    if(ret == 0) {
      pwm_debug("pwm gpio request failed!\n");
    }

    gpio_release(ret, 2);
  }

#endif

#if ((defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN9IW1P1))

  temp = sunxi_pwm_read_reg(pwm * 0x10);

  temp |= 1 << 4;
  temp |= 1 << 6;

  sunxi_pwm_write_reg(pwm * 0x10, temp);

#elif (defined CONFIG_ARCH_SUN8IW3P1)

  temp = sunxi_pwm_read_reg(0);

  if(pwm == 0) {
    temp |= 1 << 4;
    temp |= 1 << 6;
  } else {
    temp |= 1 << 19;
    temp |= 1 << 21;
  }

  sunxi_pwm_write_reg(0, temp);
  
#elif (defined CONFIG_ARCH_SUN7IW1P1)

  temp = sunxi_pwm_read_reg(0x200);

  if(pwm == 0) {
    temp |= 1 << 4;
    temp |= 1 << 6;
  } else {
    temp |= 1 << 19;
    temp |= 1 << 21;
  }

  sunxi_pwm_write_reg(0x200, temp);


#endif

  return 0;
}

void sunxi_pwm_disable(int pwm)
{
  unsigned int temp;

#ifndef CONFIG_FPGA_V4_PLATFORM

  int i;
  unsigned int ret = 0;

  for(i = 0; i < pwm_pin_count[pwm]; i++) {
    ret = gpio_request(&pwm_gpio_info[pwm][i], 1);
    if(ret == 0) {
      pwm_debug("pwm gpio request failed!\n");
    }

    gpio_release(ret, 2);
  }

  #endif

#if ((defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN9IW1P1))

  temp = sunxi_pwm_read_reg(pwm * 0x10);

  temp &= ~(1 << 4);
  temp &= ~(1 << 6);

  sunxi_pwm_write_reg(pwm * 0x10, temp);

#elif (defined CONFIG_ARCH_SUN8IW3P1)

  temp = sunxi_pwm_read_reg(0);

  if(pwm == 0) {
    temp &= ~(1 << 4);
    temp &= ~(1 << 6);
  } else {
    temp &= ~(1 << 19);
    temp &= ~(1 << 21);
  }

#elif (defined CONFIG_ARCH_SUN7IW1P1)

  temp = sunxi_pwm_read_reg(0x200);

  if(pwm == 0) {
    temp &= ~(1 << 4);
    temp &= ~(1 << 6);
  } else {
    temp &= ~(1 << 19);
    temp &= ~(1 << 21);
  }

  sunxi_pwm_write_reg(0x200, temp);

#endif
}

void sunxi_pwm_init(void)
{
  int i;

  for(i = 0; i < PWM_NUM; i++)
    sunxi_pwm_get_sys_config(i);
}
