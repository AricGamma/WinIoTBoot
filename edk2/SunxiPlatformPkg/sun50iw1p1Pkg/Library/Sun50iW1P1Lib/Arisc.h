/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  sunny <sunny@allwinnertech.com>
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


#ifndef __ARISC_I_H__
#define __ARISC_I_H__

#include <Library/IoLib.h>
#include <Library/DebugLib.h>


#define ARISC_DVFS_VF_TABLE_MAX         (16)
#define IR_NUM_KEY_SUP 16

/*
 * debug level define,
 * level 0 : dump debug information--none;
 * level 1 : dump debug information--error;
 * level 2 : dump debug information--error+warning;
 * level 3 : dump debug information--error+warning+information;
 * extern void printk(const char *, ...);
 */
//#define ARISC_DEBUG_ON
#ifdef ARISC_DEBUG_ON

/* debug levels */
#define DEBUG_LEVEL_INF    ((u32)1 << 0)
#define DEBUG_LEVEL_LOG    ((u32)1 << 1)
#define DEBUG_LEVEL_WRN    ((u32)1 << 2)
#define DEBUG_LEVEL_ERR    ((u32)1 << 3)

#define ARISC_INF(format, args...)                          \
  if(DEBUG_LEVEL_INF & (0xf0 >> (arisc_debug_level +1)))  \
    tick_printf("[ARISC] :"format, ##args);

#define ARISC_LOG(format, args...)                                      \
  if(DEBUG_LEVEL_LOG & (0xf0 >> (arisc_debug_level +1)))  \
    tick_printf("[ARISC] :"format, ##args);

#define ARISC_WRN(format, args...)                          \
  if(DEBUG_LEVEL_WRN & (0xf0 >> (arisc_debug_level +1)))  \
    printf("[ARISC WARING] :"format, ##args);

#define ARISC_ERR(format, args...)                          \
  if(DEBUG_LEVEL_ERR & (0xf0 >> (arisc_debug_level +1)))  \
    printf("[ARISC ERROR] :"format, ##args);

#else /* ARISC_DEBUG_ON */
#define ARISC_INF(...)
#define ARISC_WRN(...)
#define ARISC_ERR(...)
#define ARISC_LOG(...)

#endif /* ARISC_DEBUG_ON */

typedef EFI_PHYSICAL_ADDRESS phys_addr_t;
typedef UINT64 u64;
typedef UINT32 u32;
typedef UINT32 uint32_t;
typedef UINTN size_t;

typedef struct mem_cfg
{
  phys_addr_t base;
  size_t size;
} mem_cfg_t;

typedef struct mem_cfg_64
{
  u64 base;
  u64 size;
} mem_cfg_64_t;

typedef struct dev_cfg
{
  phys_addr_t base;
  size_t size;
  u32 irq;
  int status;
} dev_cfg_t;

typedef struct dev_cfg_64
{
  u64 base;
  u64 size;
  u32 irq;
  int status;
} dev_cfg_64_t;

typedef struct ir_code
{
  uint32_t key_code;
  uint32_t addr_code;
} ir_code_t;

typedef struct ir_key
{
  uint32_t num;
  ir_code_t ir_code[IR_NUM_KEY_SUP];
} ir_key_t;

typedef struct cir_cfg
{
  phys_addr_t base;
  size_t size;
  u32 irq;
  ir_key_t ir_key;
  int status;
} cir_cfg_t;

typedef struct cir_cfg_64
{
  u64 base;
  u64 size;
  u32 irq;
  ir_key_t ir_key;
  int status;
} cir_cfg_64_t;

typedef struct pmu_cfg
{
  u32 pmu_bat_shutdown_ltf;
  u32 pmu_bat_shutdown_htf;
  u32 pmu_pwroff_vol;
  u32 power_start;
} pmu_cfg_t;

typedef struct box_start_os_cfg
{
  u32 used;
  u32 start_type;
  u32 irkey_used;
  u32 pmukey_used;
  u32 pmukey_num;
  u32 led_power;
  u32 led_state;
} box_start_os_cfg_t;

typedef struct power_cfg
{
  u32 powchk_used;
  u32 power_reg;
  u32 system_power;
} power_cfg_t;

typedef struct image_cfg
{
  phys_addr_t base;
  size_t size;
} image_cfg_t;

typedef struct image_cfg_64
{
  u64 base;
  u64 size;
} image_cfg_64_t;

typedef struct space_cfg
{
  phys_addr_t sram_dst;
  phys_addr_t sram_offset;
  size_t sram_size;
  phys_addr_t dram_dst;
  phys_addr_t dram_offset;
  size_t dram_size;
  phys_addr_t para_dst;
  phys_addr_t para_offset;
  size_t para_size;
  phys_addr_t msgpool_dst;
  phys_addr_t msgpool_offset;
  size_t msgpool_size;
  phys_addr_t standby_dst;
  phys_addr_t standby_offset;
  size_t standby_size;
} space_cfg_t;

typedef struct space_cfg_64
{
  u64 sram_dst;
  u64 sram_offset;
  u64 sram_size;
  u64 dram_dst;
  u64 dram_offset;
  u64 dram_size;
  u64 para_dst;
  u64 para_offset;
  u64 para_size;
  u64 msgpool_dst;
  u64 msgpool_offset;
  u64 msgpool_size;
  u64 standby_dst;
  u64 standby_offset;
  u64 standby_size;
} space_cfg_64_t;

typedef struct dram_para
{
  //normal configuration
  u32 dram_clk;
  u32 dram_type; //dram_type DDR2: 2 DDR3: 3 LPDDR2: 6 DDR3L: 31
  u32 dram_zq;
  u32 dram_odt_en;

  //control configuration
  u32 dram_para1;
  u32 dram_para2;

  //timing configuration
  u32 dram_mr0;
  u32 dram_mr1;
  u32 dram_mr2;
  u32 dram_mr3;
  u32 dram_tpr0;
  u32 dram_tpr1;
  u32 dram_tpr2;
  u32 dram_tpr3;
  u32 dram_tpr4;
  u32 dram_tpr5;
  u32 dram_tpr6;

  //reserved for future use
  u32 dram_tpr7;
  u32 dram_tpr8;
  u32 dram_tpr9;
  u32 dram_tpr10;
  u32 dram_tpr11;
  u32 dram_tpr12;
  u32 dram_tpr13;

}dram_para_t;

typedef struct arisc_freq_voltage
{
  u32 freq;       //cpu frequency
  u32 voltage;    //voltage for the frequency
  u32 axi_div;    //the divide ratio of axi bus
} arisc_freq_voltage_t;

typedef struct dts_cfg
{
  struct dram_para dram_para;
  struct arisc_freq_voltage vf[ARISC_DVFS_VF_TABLE_MAX];
  struct space_cfg space;
  struct image_cfg image;
  struct mem_cfg prcm;
  struct mem_cfg cpuscfg;
  struct dev_cfg msgbox;
  struct dev_cfg hwspinlock;
  struct dev_cfg s_uart;
#if defined CONFIG_ARCH_SUN50IW2P1
  struct dev_cfg s_twi;
#else
  struct dev_cfg s_rsb;
#endif
  struct dev_cfg s_jtag;
  struct cir_cfg s_cir;
  struct pmu_cfg pmu;
  struct box_start_os_cfg start_os;
  struct power_cfg power;
} dts_cfg_t;

typedef struct dts_cfg_64
{
  struct dram_para dram_para;
  struct arisc_freq_voltage vf[ARISC_DVFS_VF_TABLE_MAX];
  struct space_cfg_64 space;
  struct image_cfg_64 image;
  struct mem_cfg_64 prcm;
  struct mem_cfg_64 cpuscfg;
  struct dev_cfg_64 msgbox;
  struct dev_cfg_64 hwspinlock;
  struct dev_cfg_64 s_uart;
#if defined CONFIG_ARCH_SUN50IW2P1
  struct dev_cfg_64 s_twi;
#else
  struct dev_cfg_64 s_rsb;
#endif
  struct dev_cfg_64 s_jtag;
  struct cir_cfg_64 s_cir;
  struct pmu_cfg pmu;
  struct box_start_os_cfg start_os;
  struct power_cfg power;
} dts_cfg_64_t;

int sunxi_arisc_probe(void);

#endif  //__ARISC_I_H__

