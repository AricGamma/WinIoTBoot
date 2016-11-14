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

 
#include <Library/BaseLib.h> 
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h> 
#include <Library/SysConfigLib.h>
#include <Library/IoLib.h>



#include <Sunxi_type/Sunxi_type.h>
#include <Sunxi_type/list.h>
#include <Sunxi_type/PrivateDef.h>

#include <platform.h>
#include <pwm.h>





#define sys_get_wvalue(n)   (*((volatile uint *)(n)))          /* word input */
#define sys_put_wvalue(n,c) (*((volatile uint *)(n))  = (c))   /* word output */
#ifndef abs
#define abs(x) (((x)&0x80000000)? (0-(x)):(x))
#endif

uint pwm_active_sta[4] = {1, 0, 0, 0};
uint pwm_pin_count[4] = {0};

user_gpio_set_t pwm_gpio_info[PWM_NUM][2];

#define sunxi_pwm_debug 0
#undef  sunxi_pwm_debug

#ifdef sunxi_pwm_debug
  #define pwm_debug(ARG...) DEBUG (( EFI_D_INFO,"[PWM Info]:"ARG))
#else
  #define pwm_debug(ARG...)
#endif

#define pwm_msg(ARG...)         DEBUG (( EFI_D_WARN,"[PWM Msg]:"ARG))

#define PWM_PIN_STATE_ACTIVE "active"
#define PWM_PIN_STATE_SLEEP "sleep"

#define SETMASK(width, shift)   ((width?((-1U) >> (32-width)):0)  << (shift))
#define CLRMASK(width, shift)   (~(SETMASK(width, shift)))
#define GET_BITS(shift, width, reg)     \
            (((reg) & SETMASK(width, shift)) >> (shift))
#define SET_BITS(shift, width, reg, val) \
            (((reg) & CLRMASK(width, shift)) | (val << (shift)))

struct sunxi_pwm_cfg {
  unsigned int reg_busy_offset;
  unsigned int reg_busy_shift;
  unsigned int reg_enable_offset;
  unsigned int reg_enable_shift;
  unsigned int reg_clk_gating_offset;
  unsigned int reg_clk_gating_shift;
  unsigned int reg_bypass_offset;
  unsigned int reg_bypass_shift;
  unsigned int reg_pulse_start_offset;
  unsigned int reg_pulse_start_shift;
  unsigned int reg_mode_offset;
  unsigned int reg_mode_shift;
  unsigned int reg_polarity_offset;
  unsigned int reg_polarity_shift;
  unsigned int reg_period_offset;
  unsigned int reg_period_shift;
  unsigned int reg_period_width;
  unsigned int reg_active_offset;
  unsigned int reg_active_shift;
  unsigned int reg_active_width;
  unsigned int reg_prescal_offset;
  unsigned int reg_prescal_shift;
  unsigned int reg_prescal_width;

};

struct sunxi_pwm_chip {
  //struct pwm_chip chip;
  struct list_head     list;
  const struct pwm_ops  *ops;
  unsigned int            base;
  int                     pwm;
  int       pwm_base;
  struct sunxi_pwm_cfg *config;
};

static LIST_HEAD(pwm_list);


#if 0
static inline struct sunxi_pwm_chip *to_sunxi_pwm_chip(struct pwm_chip *chip)
{
  //return container_of(chip, struct sunxi_pwm_chip, chip);
  return 0;
}
#endif

static inline u32 sunxi_pwm_readl(struct sunxi_pwm_chip *chip, u32 offset)
{
  unsigned int value;

  value = readl((unsigned long)chip->base + offset);

  return value;
}

static inline u32 sunxi_pwm_writel(struct sunxi_pwm_chip *chip, u32 offset, u32 value)
{

  writel(value, chip->base + offset);

  return 0;
}


uint sunxi_pwm_read_reg(uint offset)
{
    uint value = 0;

   value = sys_get_wvalue(SUNXI_PWM03_BASE + offset);

    return value;
}

uint sunxi_pwm_write_reg(uint offset, uint value)
{
    sys_put_wvalue(SUNXI_PWM03_BASE + offset, value);

    return 0;
}

static int sunxi_pwm_pin_set_state(char *dev_name, char *name)
{
  char compat[32];
  u32 len = 0;
  int ret = -1;

  if (!strcmp(name, PWM_PIN_STATE_ACTIVE))
    strcpy(compat,"pwm0");
  else
    strcpy(compat,"pwm0_suspend");

  pwm_msg("%a: state = %d\n",__func__);
  len = sprintf(compat, "%a", dev_name);
  if (len > 32)
    pwm_msg("disp_sys_set_state, size of mian_name is out of range\n");

  ret = gpio_request_simple(compat, NULL);
  if (0 != ret)
    pwm_msg("%a, gpio_request_simple, ret=%d\n", __func__, ret);
  return ret;
}

int sunxi_pwm_set_polarity(struct sunxi_pwm_chip* pchip, enum pwm_polarity polarity)
{
    uint temp;
  unsigned int reg_offset, reg_shift;

  reg_offset = pchip->config->reg_polarity_offset;
  reg_shift = pchip->config->reg_polarity_shift;
  temp = sunxi_pwm_readl(pchip, reg_offset);

  if (polarity == PWM_POLARITY_NORMAL)
    temp = SET_BITS(reg_shift, 1, temp, 1);
  else
    temp = SET_BITS(reg_shift, 1, temp, 0);

  sunxi_pwm_writel(pchip, reg_offset, temp);
/*
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

  sunxi_pwm_write_reg(0, temp);
*/
    return 0;
}

int sunxi_pwm_config(struct sunxi_pwm_chip* pchip, int duty_ns, int period_ns)
{


    uint pre_scal[11][2] = {{15, 1}, {0, 120}, {1, 180}, {2, 240}, {3, 360}, {4, 480}, {8, 12000}, {9, 24000}, {10, 36000}, {11, 48000}, {12, 72000}};
    uint freq;
    uint pre_scal_id = 0;
    uint entire_cycles = 256;
    uint active_cycles = 192;
    uint entire_cycles_max = 65536;
    uint temp;
  unsigned int reg_offset, reg_shift, reg_width;

  reg_offset = pchip->config->reg_bypass_offset;
  reg_shift = pchip->config->reg_bypass_shift;

  if (period_ns < 42) {
    /* if freq lt 24M, then direct output 24M clock */
    temp = sunxi_pwm_readl(pchip, reg_offset);
    //temp |= (0x1 << reg_shift);//pwm bypass

    temp = SET_BITS(reg_shift, 1, temp, 1);
    sunxi_pwm_writel(pchip, reg_offset, temp);

    return 0;
  }

  /* disable bypass function */
  temp = sunxi_pwm_readl(pchip, reg_offset);
  temp = SET_BITS(reg_shift, 1, temp, 0);
  sunxi_pwm_writel(pchip, reg_offset, temp);

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

  reg_offset = pchip->config->reg_prescal_offset;
  reg_shift = pchip->config->reg_prescal_shift;
  reg_width = pchip->config->reg_prescal_width;
  temp = sunxi_pwm_readl(pchip, reg_offset);

  temp = SET_BITS(reg_shift, reg_width, temp, (pre_scal[pre_scal_id][0]));

  pwm_debug("%a: reg_shift = %d, reg_width = %d, prescale temp = %x, pres=%d\n",
         __func__, reg_shift, reg_width, temp, pre_scal[pre_scal_id][0]);
  sunxi_pwm_writel(pchip, reg_offset, temp);

  /*config active cycles*/
  reg_offset = pchip->config->reg_active_offset;
  reg_shift = pchip->config->reg_active_shift;
  reg_width = pchip->config->reg_active_width;
  temp = sunxi_pwm_readl(pchip, reg_offset);
  temp = SET_BITS(reg_shift, reg_width, temp, active_cycles);
  sunxi_pwm_writel(pchip, reg_offset, temp);

  /*config period cycles*/
  reg_offset = pchip->config->reg_period_offset;
  reg_shift = pchip->config->reg_period_shift;
  reg_width = pchip->config->reg_period_width;
  temp = sunxi_pwm_readl(pchip, reg_offset);
  temp = SET_BITS(reg_shift, reg_width, temp, (entire_cycles - 1));
  sunxi_pwm_writel(pchip, reg_offset, temp);

  pwm_debug("PWM %a: duty_ns=%d, period_ns=%d, freq=%d, per_scal=%d, period_reg=0x%x\n",
          __func__, duty_ns, period_ns, freq, pre_scal_id, temp);
/*
    if(pwm == 0)
        temp = (temp & 0xfffffff0) |pre_scal[pre_scal_id][0];
    else
        temp = (temp & 0xfff87fff) |pre_scal[pre_scal_id][0];

    sunxi_pwm_write_reg(0, temp);

    sunxi_pwm_write_reg((pwm + 1)  * 0x04, ((entire_cycles - 1)<< 16) | active_cycles);

    pwm_debug("PWM _TEST: duty_ns=%d, period_ns=%d, freq=%d, per_scal=%d, period_reg=0x%x\n", duty_ns, period_ns, freq, pre_scal_id, temp);
*/
    return 0;
}

int sunxi_pwm_enable(struct sunxi_pwm_chip* pchip)
{
  int value;
  char pin_name[5];
  unsigned int reg_offset, reg_shift;

#ifndef FPGA_PLATFORM

  int i;
  uint ret = 0;
  int pwm = pchip->pwm;
  int base = pchip->pwm_base;

    for(i = 0; i < pwm_pin_count[pwm]; i++) {
        ret = gpio_request(&pwm_gpio_info[pwm][i], 1);
        if(ret == 0) {
            pwm_debug("pwm gpio request failed!\n");
        }

        gpio_release(ret, 2);
    }

#endif
/*
    temp = sunxi_pwm_read_reg(0);

    if(pwm == 0) {
        temp |= 1 << 4;
        temp |= 1 << 6;
        } else {
            temp |= 1 << 19;
            temp |= 1 << 21;
            }

    sunxi_pwm_pin_set_state("pwm", PWM_PIN_STATE_ACTIVE);
    sunxi_pwm_write_reg(0, temp);
*/
  /*active pin config.*/
  if (base > 0)
    sprintf(pin_name, "spwm%d", pwm - base);
  else
    sprintf(pin_name, "pwm%d", pwm);
  sunxi_pwm_pin_set_state(pin_name, PWM_PIN_STATE_ACTIVE);

  /* enable clk for pwm controller. */
  reg_offset = pchip->config->reg_clk_gating_offset;
  reg_shift = pchip->config->reg_clk_gating_shift;
  value = sunxi_pwm_readl(pchip, reg_offset);
  value = SET_BITS(reg_shift, 1, value, 1);
  sunxi_pwm_writel(pchip, reg_offset, value);

  /* enable pwm controller. */
  reg_offset = pchip->config->reg_enable_offset;
  reg_shift = pchip->config->reg_enable_shift;
  value = sunxi_pwm_readl(pchip, reg_offset);
  value = SET_BITS(reg_shift, 1, value, 1);
  sunxi_pwm_writel(pchip, reg_offset, value);

    return 0;
}

void sunxi_pwm_disable(struct sunxi_pwm_chip* pchip)
{
    uint temp;
  int pwm, base;
  unsigned int reg_offset, reg_shift;
  char pin_name[5];

  base = pchip->pwm_base;
  pwm = pchip->pwm;
#ifndef FPGA_PLATFORM

    int i;
    uint ret = 0;

    for(i = 0; i < pwm_pin_count[pwm]; i++) {
        ret = gpio_request(&pwm_gpio_info[pwm][i], 1);
        if(ret == 0) {
            pwm_debug("pwm gpio request failed!\n");
        }

        gpio_release(ret, 2);
    }
#endif

/*
    temp = sunxi_pwm_read_reg(0);

    if(pwm == 0) {
        temp &= ~(1 << 4);
        temp &= ~(1 << 6);
        } else {
            temp &= ~(1 << 19);
            temp &= ~(1 << 21);
            }
  sunxi_pwm_write_reg(0, temp);
  sunxi_pwm_pin_set_state("pwm", PWM_PIN_STATE_SLEEP);
*/
  /* disable pwm controller. */
  reg_offset = pchip->config->reg_enable_offset;
  reg_shift = pchip->config->reg_enable_shift;
  temp = sunxi_pwm_readl(pchip, reg_offset);
  temp = SET_BITS(reg_shift, 1, temp, 0);
  sunxi_pwm_writel(pchip, reg_offset, temp);

  /* disable clk for pwm controller. */
  reg_offset = pchip->config->reg_clk_gating_offset;
  reg_shift = pchip->config->reg_clk_gating_shift;
  temp = sunxi_pwm_readl(pchip, reg_offset);
  temp = SET_BITS(reg_shift, 1, temp, 0);
  sunxi_pwm_writel(pchip, reg_offset, temp);

  /* disable pin config. */
  if (base > 0)
    sprintf(pin_name, "spwm%d", pwm - base);
  else
    sprintf(pin_name, "pwm%d", pwm);
  sunxi_pwm_pin_set_state(pin_name, PWM_PIN_STATE_SLEEP);

}

void sunxi_pwm_init(void)
{

}
static int sunxi_pwm_get_config(int node, struct sunxi_pwm_cfg *config)
{
  config->reg_enable_offset = 0x0;
  config->reg_enable_shift = 0x4;
  config->reg_clk_gating_offset = 0x0;
  config->reg_clk_gating_shift = 0x6;
  config->reg_bypass_offset= 0x0;
  config->reg_bypass_shift = 0x9;
  config->reg_polarity_offset = 0x0;
  config->reg_polarity_shift = 0x5;
  config->reg_period_offset = 0x4;
  config->reg_period_shift = 16;
  config->reg_period_width = 16;
  config->reg_active_offset = 0x4;
  config->reg_active_shift = 0x0;
  config->reg_active_width = 16;
  config->reg_prescal_offset = 0x0;
  config->reg_prescal_shift = 0x0;
  config->reg_prescal_width = 0x4;
  return 0;
}

struct pwm_ops {
  void      (*free)(struct sunxi_pwm_chip* pchip);
  int       (*config)(struct sunxi_pwm_chip* pchip, int duty_ns, int period_ns);
  int       (*set_polarity)(struct sunxi_pwm_chip* pchip, enum pwm_polarity polarity);
  int       (*enable)(struct sunxi_pwm_chip* pchip);
  void      (*disable)(struct sunxi_pwm_chip* pchip);
};

static struct pwm_ops sunxi_pwm_ops = {
  .config = sunxi_pwm_config,
  .enable = sunxi_pwm_enable,
  .disable = sunxi_pwm_disable,
  .set_polarity = sunxi_pwm_set_polarity,
};


int pwm_config(int pwm, int duty_ns, int period_ns)
{
  struct sunxi_pwm_chip* pchip = NULL;

  list_for_each_entry(pchip, &pwm_list, list) {
    if(pchip->pwm == pwm) {
      if(pchip->ops->config)
        return pchip->ops->config(pchip, duty_ns, period_ns);
    }

    return 0;
  }

  return -1;
}

int pwm_enable(int pwm)
{
  struct sunxi_pwm_chip* pchip = NULL;

  list_for_each_entry(pchip, &pwm_list, list) {
    if(pchip->pwm == pwm) {
      if(pchip->ops->enable)
        return pchip->ops->enable(pchip);
    }
    return 0;
  }

  return -1;
}

int pwm_disable(int pwm)
{
  struct sunxi_pwm_chip* pchip = NULL;

  list_for_each_entry(pchip, &pwm_list, list) {
    if(pchip->pwm == pwm) {
      if(pchip->ops->disable)
        pchip->ops->disable(pchip);
    }
    return 0;
  }

  return -1;
}

int pwm_set_polarity(int pwm, enum pwm_polarity polarity)
{
  struct sunxi_pwm_chip* pchip = NULL;

    list_for_each_entry(pchip, &pwm_list, list) {
      if(pchip->pwm == pwm) {
        if(pchip->ops->set_polarity)
          pchip->ops->set_polarity(pchip, polarity);
      }
      return 0;
    }

  return -1;
}

int pwm_init(void)
{
  return 0;
}

int pwm_request(int pwm, const char *label)
{
  char main_name[20], sub_name[25];
  INT32 pwm_base = 0;
  INT32 pwm_number = 0;
  struct sunxi_pwm_chip* pchip;

  list_for_each_entry(pchip, &pwm_list, list) {
    if(pchip->pwm == pwm) {
      pwm_msg("%a: err:this pwm has been requested!\n", __func__);
      return -1;
    }
  }

  
    
  sprintf(main_name, "pwm");
  sprintf(sub_name, "pwm-base");
  script_parser_fetch(main_name, sub_name, &pwm_base, 1);
  
  sprintf(sub_name, "pwm-number");
  script_parser_fetch(main_name, sub_name, &pwm_number, 1);

  pwm_base = 0x0;
  pwm_number = 0x1;

  /* get pwm config.*/
  pchip = malloc(sizeof(*pchip));
  if (!pchip) {
    pwm_msg("%a: error:pwm chip malloc failed!\n",__func__);
    return -1;
  }else {
    memset(pchip, 0, sizeof(*pchip));
  }

  pchip->pwm_base = pwm_base;
  pchip->config = (struct sunxi_pwm_cfg*) malloc(sizeof(struct sunxi_pwm_cfg));
  if (!pchip->config) {
    pwm_msg("%a: error:pwm chip malloc failed!\n",__func__);
    return -1;
  }else {
    memset(pchip->config, 0, sizeof(struct sunxi_pwm_cfg));
  }
  pchip->pwm = pwm;
  pchip->ops = &sunxi_pwm_ops;
  pchip->base = 0x01c21400;  //reg_base
  

  sunxi_pwm_get_config(0, pchip->config);

  list_add_tail(&pchip->list, &pwm_list);

  pwm_msg("request pwm success,pwm-base=0x%x, pwm_number=%d,pwm = %d.\n",pwm_base,pwm_number, pwm);

  return pwm;
}


