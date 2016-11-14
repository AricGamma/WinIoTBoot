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

#include"clk_sun50iw1.h"
#include<Clk/clk_plat.h>
#include"clk_factor.h"
#include"clk_periph.h"
#include"clk_sun50iw1_tbl.c"

#define LOCKBIT(x) x

/*ns  nw  ks  kw  ms  mw  ps  pw  d1s  d1w  d2s  d2w  {frac  out  mode}  en-s   sdmss  sdmsw  sdmpat    sdmval*/
SUNXI_CLK_FACTORS(      pll_video0, 8,  7,  0,  0,  0,  4,  0,  0,  0,   0,   0,   0,    1,    25,  24,    31,   20,     0,       PLL_VIDEO0PAT,0xd1303333);
SUNXI_CLK_FACTORS(      pll_video1, 8,  7,  0,  0,  0,  4,  0,  0,  0,   0,   0,   0,    1,    25,  24,    31,   20,     0,       PLL_VEDEO1PAT,0xd1303333);
SUNXI_CLK_FACTORS(      pll_mipi,   8,  4,  4,  2,  0,  4,  0,  0,  0,   0,   0,   0,    0,    0,   0,     31,   20,     0,       PLL_MIPIPAT, 0xd1303333);
SUNXI_CLK_FACTORS(      pll_de,     8,  7,  0,  0,  0,  4,  0,  0,  0,   0,   0,   0,    1,    25,  24,    31,   20,     0,       PLL_DEPAT   ,0xd1303333);

static int get_factors_pll_video0(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
  u64 tmp_rate;
  int index;

  if(!factor)
    return -1;

  tmp_rate = rate>pllvideo0_max ? pllvideo0_max : rate;
  do_div(tmp_rate, 1000000);
  index = tmp_rate;

  if (sunxi_clk_com_ftr_sr(&sunxi_clk_factor_pll_video0, factor,
        factor_pllvideo0_tbl, index,
        sizeof(factor_pllvideo0_tbl)
        / sizeof(struct sunxi_clk_factor_freq)))
    return -1;

  if (rate == 297000000) {
    factor->frac_mode = 0;
    factor->frac_freq = 1;
    factor->factorm = 0;
  } else if (rate == 270000000) {
    factor->frac_mode = 0;
    factor->frac_freq = 0;
    factor->factorm = 0;
  } else {
    factor->frac_mode = 1;
    factor->frac_freq = 0;
  }

  return 0;
}

static int get_factors_pll_video1(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
  u64 tmp_rate;
  int index;

  if(!factor)
    return -1;

  tmp_rate = rate>pllvideo1_max ? pllvideo1_max : rate;
  do_div(tmp_rate, 1000000);
  index = tmp_rate;

  if (sunxi_clk_com_ftr_sr(&sunxi_clk_factor_pll_video1, factor,
        factor_pllvideo1_tbl, index,
        sizeof(factor_pllvideo1_tbl)
        / sizeof(struct sunxi_clk_factor_freq)))
    return -1;

  if (rate == 297000000) {
    factor->frac_mode = 0;
    factor->frac_freq = 1;
    factor->factorm = 0;
  } else if (rate == 270000000) {
    factor->frac_mode = 0;
    factor->frac_freq = 0;
    factor->factorm = 0;
  } else {
    factor->frac_mode = 1;
    factor->frac_freq = 0;
  }

  return 0;
}

static int get_factors_pll_mipi(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{

  u64 tmp_rate;
  u32 delta1,delta2,want_rate,new_rate,save_rate=0;
  int n,k,m;

  if(!factor)
    return -1;
  tmp_rate = rate>1440000000 ? 1440000000 : rate;
  do_div(tmp_rate, 1000000);
  want_rate = tmp_rate;
  for(m=1; m <=16; m++) {
    for(k=2; k <=4; k++) {
      for(n=1; n <=16; n++) {
        new_rate = (parent_rate/1000000)*k*n/m;
        delta1 = (new_rate > want_rate)?(new_rate - want_rate):(want_rate - new_rate);
        delta2 =  (save_rate > want_rate)?(save_rate - want_rate):(want_rate - save_rate);
        if(delta1 < delta2) {
          factor->factorn = n-1;
          factor->factork = k-1;
          factor->factorm = m-1;
          save_rate = new_rate;
        }
      }
    }
  }

  return 0;
}

static int get_factors_pll_de(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
  u64 tmp_rate;
  int index;

  if (!factor)
    return -1;

  tmp_rate = rate>pllde_max ? pllde_max : rate;
  do_div(tmp_rate, 1000000);
  index = tmp_rate;

  if (sunxi_clk_com_ftr_sr(&sunxi_clk_factor_pll_de, factor,
        factor_pllde_tbl, index,
        sizeof(factor_pllde_tbl)
        / sizeof(struct sunxi_clk_factor_freq)))
    return -1;

  if (rate == 297000000) {
    factor->frac_mode = 0;
    factor->frac_freq = 1;
    factor->factorm = 0;
  } else if (rate == 270000000) {
    factor->frac_mode = 0;
    factor->frac_freq = 0;
    factor->factorm = 0;
  } else {
    factor->frac_mode = 1;
    factor->frac_freq = 0;
  }

  return 0;
}


/*  pll_video0:24*N/M */
static unsigned long calc_rate_media(u32 parent_rate, struct clk_factors_value *factor)
{
    unsigned long long tmp_rate = (parent_rate?parent_rate:24000000);
    if(factor->frac_mode == 0)
    {
        if(factor->frac_freq == 1)
          return 297000000;
        else
          return 270000000;
    }
    else
    {
        tmp_rate = tmp_rate * (factor->factorn+1);
        do_div(tmp_rate, factor->factorm+1);
        return (unsigned long)tmp_rate;
    }
}

/*  pll_mipi: pll_video0*N*K/M  */
static unsigned long calc_rate_pll_mipi(u32 parent_rate, struct clk_factors_value *factor)
{
    unsigned long long tmp_rate = (parent_rate?parent_rate:24000000);
    tmp_rate = tmp_rate * (factor->factorn+1) * (factor->factork+1);
    do_div(tmp_rate, factor->factorm+1);
    return (unsigned long)tmp_rate;
}

u8 get_parent_pll_mipi(struct clk_hw *hw)
{
    u8 parent;
    unsigned long reg;
  struct sunxi_clk_factors *factor = to_clk_factor(hw);

    if(!factor->reg)
        return 0;
    reg = readl(factor->reg);
    parent = GET_BITS(21, 1, reg);

    return parent;
}
int set_parent_pll_mipi(struct clk_hw *hw, u8 index)
{
    unsigned long reg;
  struct sunxi_clk_factors *factor = to_clk_factor(hw);

    if(!factor->reg)
        return 0;
    reg = readl(factor->reg);
    reg = SET_BITS(21, 1, reg, index);
    writel(reg, factor->reg);
    return 0;
}
static int clk_enable_pll_mipi(struct clk_hw *hw)
{
  struct sunxi_clk_factors *factor = to_clk_factor(hw);
    struct sunxi_clk_factors_config *config = factor->config;
    unsigned long reg = readl(factor->reg);

    if(config->sdmwidth)
    {
        writel(config->sdmval, (void __iomem *)config->sdmpat);
        reg = SET_BITS(config->sdmshift, config->sdmwidth, reg, 1);
    }

    reg |= 0x3 << 22;
    writel(reg, factor->reg);
    udelay(100);

    reg = SET_BITS(config->enshift, 1, reg, 1);
    writel(reg, factor->reg);
    udelay(100);

    return 0;
}

static void clk_disable_pll_mipi(struct clk_hw *hw)
{
  struct sunxi_clk_factors *factor = to_clk_factor(hw);
    struct sunxi_clk_factors_config *config = factor->config;
    unsigned long reg = readl(factor->reg);

    if(config->sdmwidth)
        reg = SET_BITS(config->sdmshift, config->sdmwidth, reg, 0);
    reg = SET_BITS(config->enshift, 1, reg, 0);
    reg &= ~(0x3 << 22);
    writel(reg, factor->reg);
}

static const char *mipi_parents[] = {"pll_video0",""};
static const char *hosc_parents[] = {"hosc"};
struct clk_ops pll_mipi_ops;

struct factor_init_data sunxi_factos[] = {
  /* name         parent        parent_num, flags                                      reg          lock_reg     lock_bit     pll_lock_ctrl_reg lock_en_bit lock_mode           config                         get_factors               calc_rate              priv_ops*/
  {"pll_video0",  hosc_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_VIDEO0,  PLL_VIDEO0,  LOCKBIT(28), PLL_CLK_CTRL,     2,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_video0,  &get_factors_pll_video0,  &calc_rate_media,      (struct clk_ops*)NULL},
  {"pll_video1",  hosc_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_VIDEO1,  PLL_VIDEO1,  LOCKBIT(28), PLL_CLK_CTRL,     6,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_video1,  &get_factors_pll_video1,  &calc_rate_media,      (struct clk_ops*)NULL},
  {"pll_mipi",    mipi_parents, 2,          CLK_IGNORE_DISABLE,                        MIPI_PLL,    MIPI_PLL,    LOCKBIT(28), PLL_CLK_CTRL,     8,          PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_mipi,    &get_factors_pll_mipi,    &calc_rate_pll_mipi,   &pll_mipi_ops        },
  {"pll_de",      hosc_parents, 1,          CLK_IGNORE_DISABLE,                        PLL_DE,      PLL_DE,      LOCKBIT(28), PLL_CLK_CTRL,     10,         PLL_LOCK_NONE_MODE, &sunxi_clk_factor_pll_de,      &get_factors_pll_de,      &calc_rate_media,      (struct clk_ops*)NULL},
};

static const char *de_parents[] = {"pll_periph0x2", "pll_de", "", "", "", "","",""};
static const char *tcon0_parents[] = {"pll_mipi", "", "pll_video0x2", "", "", "", "", ""};
static const char *tcon1_parents[] = {"pll_video0", "", "pll_video1", ""};
static const char *mipidsi_parents[] = {"pll_video0", "", "pll_periph0",""};
static const char *lvds_parents[] = {"tcon0"};
static const char *hdmi_parents[]= {"pll_video0","pll_video1","",""};

struct sunxi_clk_comgate com_gates[]={
{"csi",      0,  0x3,    BUS_GATE_SHARE|RST_GATE_SHARE|MBUS_GATE_SHARE, 0},
{"adda",     0,  0x1,    BUS_GATE_SHARE|RST_GATE_SHARE,                 0},
{"usbhci1",   0,  0x3,    RST_GATE_SHARE|MBUS_GATE_SHARE, 0},
{"usbhci0",   0,  0x3,    RST_GATE_SHARE|MBUS_GATE_SHARE, 0},
};

static int clk_lock = 0;

SUNXI_CLK_PERIPH(de,    DE_CFG,   24,    3,     DE_CFG,   0,      4,      0,      0,      0,      DE_CFG,   BUS_RST1,  BUS_GATE1,  0,       31,     12,      12,        0,         &clk_lock,NULL,         0);
SUNXI_CLK_PERIPH(tcon0,   TCON0_CFG,  24,    3,     0,      0,      0,      0,      0,      0,      TCON0_CFG,  BUS_RST1,  BUS_GATE1,  0,       31,      3,       3,        0,         &clk_lock,NULL,         0);
SUNXI_CLK_PERIPH(tcon1,   TCON1_CFG,  24,    2,     TCON1_CFG,  0,      4,      0,      0,      0,      TCON1_CFG,  BUS_RST1,  BUS_GATE1,  0,       31,      4,       4,        0,         &clk_lock,NULL,         0);
SUNXI_CLK_PERIPH(lvds,    0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST2,  0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(mipidsi, MIPI_DSI,    8,        2,         MIPI_DSI,   0,          4,          0,          0,          0,          MIPI_DSI,   BUS_RST0,  BUS_GATE0,    0,           15,            1,           1,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(hdmi,     HDMI_CFG,  24,        2,         HDMI_CFG,   0,          4,          0,          0,          0,          HDMI_CFG,   BUS_RST1,  BUS_GATE1,    BUS_RST1,    31,           11,          11,             10,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(hdmi_slow,0,        0,        0,         0,          0,          0,          0,          0,          0,          HDMI_SLOW,  0,          0,           0,           31,            0,           0,              0,               &clk_lock,NULL,             0);


struct periph_init_data sunxi_periphs_init[] = {
  {"de",       0,               de_parents,     ARRAY_SIZE(de_parents),     &sunxi_clk_periph_de},
    {"tcon0",    0,               tcon0_parents,    ARRAY_SIZE(tcon0_parents),    &sunxi_clk_periph_tcon0},
    {"tcon1",    0,               tcon1_parents,    ARRAY_SIZE(tcon1_parents),    &sunxi_clk_periph_tcon1},
    {"hdmi",     0,                 hdmi_parents,     ARRAY_SIZE(hdmi_parents),     &sunxi_clk_periph_hdmi},
    {"hdmi_slow",0,                 hosc_parents,     ARRAY_SIZE(hosc_parents),     &sunxi_clk_periph_hdmi_slow},
  {"lvds",     0,                 lvds_parents,     ARRAY_SIZE(lvds_parents),     &sunxi_clk_periph_lvds},
  {"mipidsi",  0,               mipidsi_parents,  ARRAY_SIZE(mipidsi_parents),  &sunxi_clk_periph_mipidsi},
};

static void  *sunxi_clk_base = NULL;

void  init_clocks(void)
{
  int i;
  //struct clk *clk;
  struct factor_init_data *factor;
  struct periph_init_data *periph;
  /* get clk register base address */
  sunxi_clk_base = (void*)0x01c20000; //fixed base address.
  sunxi_clk_factor_initlimits();

  sunxi_clk_get_factors_ops(&pll_mipi_ops);
  pll_mipi_ops.get_parent = get_parent_pll_mipi;
  pll_mipi_ops.set_parent = set_parent_pll_mipi;
  pll_mipi_ops.enable = clk_enable_pll_mipi;
  pll_mipi_ops.disable = clk_disable_pll_mipi;

  clk_register_fixed_rate(NULL, "hosc", NULL, CLK_IS_ROOT, 24000000);

  /* register normal factors, based on sunxi factor framework */
  for(i=0; i<ARRAY_SIZE(sunxi_factos); i++) {
    factor = &sunxi_factos[i];
    factor->priv_regops = NULL;
    sunxi_clk_register_factors(NULL, (void*)sunxi_clk_base, (struct factor_init_data*)factor);
  }

  /* register periph clock */
  for(i=0; i<ARRAY_SIZE(sunxi_periphs_init); i++) {
    periph = &sunxi_periphs_init[i];
    periph->periph->priv_regops = NULL;
    sunxi_clk_register_periph(periph, sunxi_clk_base);
  }
  DEBUG (( EFI_D_INFO,"finish init_clocks.\n"));
}

