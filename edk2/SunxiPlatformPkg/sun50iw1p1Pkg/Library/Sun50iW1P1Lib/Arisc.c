/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Martin.Zheng <martinzheng@allwinnertech.com>
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

#include <Library/IoLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/SunxiSmcLib.h>
#include "Arisc.h"
//if all value are zero , compile will not allocate space to this global variable.
//use them before uboot Relocation will cause error.

//#define PARSE_FROM_CONFIG 

struct dts_cfg dts_cfg =
{
  .dram_para = {0x1,0x1}
};
struct dts_cfg_64 dts_cfg_64 =
{
  .dram_para = {0x1,0x1}
};
unsigned int arisc_debug_level = 2;

/* Copies bytes between buffers */
static void * memcpy (void *dest, const void *src, unsigned int count)
{
  return CopyMem (dest, src, (UINTN)count);
}

#ifdef PARSE_FROM_CONFIG
int arisc_dvfs_cfg_vf_table(void)
{
  u32    value = 0;
  int    index = 0;
  int    nodeoffset;
  u32    vf_table_size = 0;
  char   vf_table_main_key[64];
  char   vf_table_sub_key[64];
  u32    vf_table_count = 0;
  u32    vf_table_type = 0;

  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,dvfs_table");
  nodeoffset = fdt_path_offset(working_fdt, "/dvfs_table");

  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,dvfs_table] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "vf_table_count", &vf_table_count))) {
    ARISC_LOG("%s: support only one vf_table\n", __func__);
    sprintf(vf_table_main_key, "%s", "/dvfs_table");
  } else {
    //vf_table_type = sunxi_get_soc_bin();
    sprintf(vf_table_main_key, "%s%d", "/vf_table", vf_table_type);
  }
  ARISC_INF("%s: vf table type [%d=%s]\n", __func__, vf_table_type, vf_table_main_key);

  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, vf_table_main_key);
  nodeoffset = fdt_path_offset(working_fdt, vf_table_main_key);
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [%s] device node error\n", vf_table_main_key);
    return -EINVAL;
  }

  /* parse system config v-f table information */
  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "lv_count", &vf_table_size))) {
    ARISC_ERR("parse system config dvfs_table size fail\n");
    return -EINVAL;
  }
  for (index = 0; index < vf_table_size; index++) {
    sprintf(vf_table_sub_key, "lv%d_freq", index + 1);
    if (!IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, vf_table_sub_key, &value))) {
      dts_cfg.vf[index].freq = value;
    }
    ARISC_INF("%s: freq [%s-%d=%d]\n", __func__, vf_table_sub_key, index, value);
    sprintf(vf_table_sub_key, "lv%d_volt", index + 1);
    if (!IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, vf_table_sub_key, &value))) {
      dts_cfg.vf[index].voltage = value;
    }
    ARISC_INF("%s: volt [%s-%d=%d]\n", __func__, vf_table_sub_key, index, value);
  }

  return 0;
}

static int sunxi_arisc_parse_cfg(void)
{
  u32 value[4] = {0, 0, 0};
  int nodeoffset;
  uint32_t i;
  char subkey[64];

  /* parse arisc node */
  //fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,sunxi-arisc");
  nodeoffset = fdt_path_offset(working_fdt,"/soc/arisc");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,sunxi-arisc] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "powchk_used", &dts_cfg.power.powchk_used))) {
    ARISC_ERR("parse power powchk_used fail\n");
    return -EINVAL;
  }
  ARISC_INF("powchk_used:0x%x\n", dts_cfg.power.powchk_used);

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "power_reg", &dts_cfg.power.power_reg))) {
    ARISC_ERR("parse power power_reg fail\n");
    return -EINVAL;
  }
  ARISC_INF("power_reg:0x%x\n", dts_cfg.power.power_reg);

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "system_power", &dts_cfg.power.system_power))) {
    ARISC_ERR("parse power system_power fail\n");
    return -EINVAL;
  }
  ARISC_INF("system_power:0x%x\n", dts_cfg.power.system_power);

  /* parse arisc_space node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "/soc/arisc_space");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/arisc_space");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,arisc_space] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "space1", value))) {
    ARISC_ERR("get arisc_space space1 error\n");
    return -EINVAL;
  }

  dts_cfg.space.sram_dst = (phys_addr_t)value[0];
  dts_cfg.space.sram_offset = (phys_addr_t)value[1];
  dts_cfg.space.sram_size = (size_t)value[2];

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "space2", value))) {
    ARISC_ERR("get arisc_space space2 error\n");
    return -EINVAL;
  }

  dts_cfg.space.dram_dst = (phys_addr_t)value[0];
  dts_cfg.space.dram_offset = (phys_addr_t)value[1];
  dts_cfg.space.dram_size = (size_t)value[2];

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "space3", value))) {
    ARISC_ERR("get arisc_space space3 error\n");
    return -EINVAL;
  }

  dts_cfg.space.para_dst = (phys_addr_t)value[0];
  dts_cfg.space.para_offset = (phys_addr_t)value[1];
  dts_cfg.space.para_size = (size_t)value[2];

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "space4", value))) {
    ARISC_ERR("get arisc_space space4 error\n");
    return -EINVAL;
  }

  dts_cfg.space.msgpool_dst = (phys_addr_t)value[0];
  dts_cfg.space.msgpool_offset = (phys_addr_t)value[1];
  dts_cfg.space.msgpool_size = (size_t)value[2];

  ARISC_INF("arisc_space space1 sram_dst:0x%p, sram_offset:0x%p, sram_size:0x%zx, \n",
    (void *)dts_cfg.space.sram_dst, (void *)dts_cfg.space.sram_offset, dts_cfg.space.sram_size);
  ARISC_INF("arisc_space space2 dram_dst:0x%p, dram_offset:0x%p, dram_size:0x%zx, \n",
    (void *)dts_cfg.space.dram_dst, (void *)dts_cfg.space.dram_offset, dts_cfg.space.dram_size);
  ARISC_INF("arisc_space space3 para_dst:0x%p, para_offset:0x%p, para_size:0x%zx, \n",
    (void *)dts_cfg.space.para_dst, (void *)dts_cfg.space.para_offset, dts_cfg.space.para_size);
  ARISC_INF("arisc_space space4 msgpool_dst:0x%p, msgpool_offset:0x%p, msgpool_size:0x%zx, \n",
    (void *)dts_cfg.space.msgpool_dst, (void *)dts_cfg.space.msgpool_offset, dts_cfg.space.msgpool_size);

  /* parse standby_space node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,standby_space");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/standby_space");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,standby_space] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "space1", value))) {
    ARISC_ERR("get standby_space space1 error\n");
    return -EINVAL;
  }

  dts_cfg.space.standby_dst = (phys_addr_t)value[0];
  dts_cfg.space.standby_offset = (phys_addr_t)value[1];
  dts_cfg.space.standby_size = (size_t)value[2];

  ARISC_INF("standby_space space1 standby_dst:0x%p, standby_offset:0x%p, standby_size:0x%zx, \n",
    (void *)dts_cfg.space.standby_dst, (void *)dts_cfg.space.standby_offset, dts_cfg.space.standby_size);

  /* parse dram node */
  memcpy((void *)&dts_cfg.dram_para, (void *)(uboot_spare_head.boot_data.dram_para), sizeof(dts_cfg.dram_para));

  /* parse prcm node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,prcm");
  nodeoffset = fdt_path_offset(working_fdt,  "/prcm");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,prcm] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "reg", value))) {
    ARISC_ERR("get prcm reg error\n");
    return -EINVAL;
  }

  dts_cfg.prcm.base = (phys_addr_t)value[1];
  dts_cfg.prcm.size = (size_t)value[3];

  ARISC_INF("prcm base:0x%p, size:0x%zx\n",
    (void *)dts_cfg.prcm.base, dts_cfg.prcm.size);

  /* parse cpuscfg node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,cpuscfg");
  nodeoffset = fdt_path_offset(working_fdt,  "/cpuscfg");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,cpuscfg] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "reg", value))) {
    ARISC_ERR("get cpuscfg reg error\n");
    return -EINVAL;
  }

  dts_cfg.cpuscfg.base = (phys_addr_t)value[1];
  dts_cfg.cpuscfg.size = (size_t)value[3];

  ARISC_INF("cpuscfg base:0x%p, size:0x%zx\n",
    (void *)dts_cfg.cpuscfg.base, dts_cfg.cpuscfg.size);

  /* parse msgbox node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,msgbox");
  nodeoffset = fdt_path_offset(working_fdt,  "/soc/msgbox");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,msgbox] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "reg", value))) {
    ARISC_ERR("get msgbox reg error\n");
    return -EINVAL;
  }

  dts_cfg.msgbox.base = (phys_addr_t)value[1];
  dts_cfg.msgbox.size = (size_t)value[3];

  dts_cfg.msgbox.status = fdtdec_get_is_enabled(working_fdt, nodeoffset);

  ARISC_INF("msgbox base:0x%p, size:0x%zx, status:%u\n",
    (void *)dts_cfg.msgbox.base, dts_cfg.msgbox.size, dts_cfg.msgbox.status);

  /* parse hwspinlock node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,sunxi-hwspinlock");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/hwspinlock");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,sunxi-hwspinlock] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "reg", value))) {
    ARISC_ERR("get sunxi-hwspinlock reg error\n");
    return -EINVAL;
  }

  dts_cfg.hwspinlock.base = (phys_addr_t)value[1];
  dts_cfg.hwspinlock.size = (size_t)value[3];

  dts_cfg.hwspinlock.status = fdtdec_get_is_enabled(working_fdt, nodeoffset);

  ARISC_INF("hwspinlock base:0x%p, size:0x%zx, status:%u\n",
    (void *)dts_cfg.hwspinlock.base, dts_cfg.hwspinlock.size, dts_cfg.hwspinlock.status);

  /* parse s_uart node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,s_uart");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/s_uart");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,s_uart] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "reg", value))) {
    ARISC_ERR("get s_uart reg error\n");
    return -EINVAL;
  }

  dts_cfg.s_uart.base = (phys_addr_t)value[1];
  dts_cfg.s_uart.size = (size_t)value[3];

  ARISC_INF("s_uart base:0x%p, size:0x%zx\n",
    (void *)dts_cfg.s_uart.base, dts_cfg.s_uart.size);

  dts_cfg.s_uart.status = fdtdec_get_is_enabled(working_fdt, nodeoffset);

  ARISC_INF("s_uart base:0x%p, size:0x%zx, status:%u\n",
    (void *)dts_cfg.s_uart.base, dts_cfg.s_uart.size, dts_cfg.s_uart.status);

#if defined CONFIG_ARCH_SUN50IW2P1
  /* parse s_twi node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,s_twi");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/s_twi");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,s_twi] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "reg", value))) {
    ARISC_ERR("get s_twi reg error\n");
    return -EINVAL;
  }

  dts_cfg.s_twi.base = (phys_addr_t)value[1];
  dts_cfg.s_twi.size = (size_t)value[3];

  dts_cfg.s_twi.status = fdtdec_get_is_enabled(working_fdt, nodeoffset);

  ARISC_INF("s_twi base:0x%p, size:0x%zx, status:%u\n",
    (void *)dts_cfg.s_twi.base, dts_cfg.s_twi.size, dts_cfg.s_twi.status);
#else
  /* parse s_rsb node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,s_rsb");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/s_rsb");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,s_rsb] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "reg", value))) {
    ARISC_ERR("get s_rsb reg error\n");
    return -EINVAL;
  }

  dts_cfg.s_rsb.base = (phys_addr_t)value[1];
  dts_cfg.s_rsb.size = (size_t)value[3];

  dts_cfg.s_rsb.status = fdtdec_get_is_enabled(working_fdt, nodeoffset);

  ARISC_INF("s_rsb base:0x%p, size:0x%zx, status:%u\n",
    (void *)dts_cfg.s_rsb.base, dts_cfg.s_rsb.size, dts_cfg.s_rsb.status);
#endif
  /* parse s_jtag node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,s_jtag");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/s_jtag0");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,s_jtag] device node error\n");
    return -EINVAL;
  }

  dts_cfg.s_jtag.status = fdtdec_get_is_enabled(working_fdt, nodeoffset);

  ARISC_INF("s_jtag status:%u\n", dts_cfg.s_jtag.status);

  /* parse dvfs_table node */
  if (arisc_dvfs_cfg_vf_table()) {
    ARISC_ERR("parse dvfs v-f table failed\n");
    return -EINVAL;
  }


  /* parse s_cir node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,s_cir");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/s_cir");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,s_cir] device node error\n");
    return -EINVAL;
  }

  if (!IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "ir_power_key_code", &dts_cfg.s_cir.ir_key.ir_code[0].key_code))) {
    if (!IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "ir_addr_code", &dts_cfg.s_cir.ir_key.ir_code[0].addr_code))) {
      dts_cfg.s_cir.ir_key.num = 1;
      goto print_ir_paras;
    }
  }

  dts_cfg.s_cir.ir_key.num = 0;
  for (i = 0; i < IR_NUM_KEY_SUP; i++) {
    sprintf(subkey, "%s%d", "ir_power_key_code", i);
    if (!IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, subkey, &dts_cfg.s_cir.ir_key.ir_code[i].key_code))) {
      sprintf(subkey, "%s%d", "ir_addr_code", i);
      if (!IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, subkey, &dts_cfg.s_cir.ir_key.ir_code[i].addr_code))) {
        dts_cfg.s_cir.ir_key.num++;
      }
    }
  }

print_ir_paras:
  for (i = 0; i < dts_cfg.s_cir.ir_key.num; i++) {
    ARISC_INF("scir ir_code[%u].key_code:0x%x, ir_code[%u].addr_code:0x%x\n",
      i, dts_cfg.s_cir.ir_key.ir_code[i].key_code, i, dts_cfg.s_cir.ir_key.ir_code[i].addr_code);
  }

  /* config pmu config paras */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,pmu0");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/pmu0");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,pmu0] device node error\n");
    return -EINVAL;
  }
  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "pmu_bat_shutdown_ltf", &dts_cfg.pmu.pmu_bat_shutdown_ltf))) {
    ARISC_ERR("parse pmu_bat_shutdown_ltf fail\n");
    return -EINVAL;
  }
  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "pmu_bat_shutdown_htf", &dts_cfg.pmu.pmu_bat_shutdown_htf))) {
    ARISC_ERR("parse pmu_bat_shutdown_htf fail\n");
    return -EINVAL;
  }
  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "pmu_pwroff_vol", &dts_cfg.pmu.pmu_pwroff_vol))) {
    ARISC_ERR("parse pmu_pwroff_vol fail\n");
    return -EINVAL;
  }
  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "power_start", &dts_cfg.pmu.power_start))) {
    ARISC_ERR("parse power_start fail\n");
    return -EINVAL;
  }
  ARISC_INF("pmu pmu_bat_shutdown_ltf:0x%x, pmu_bat_shutdown_htf:0x%x, pmu_pwroff_vol:0x%x, power_start:0x%x\n",
    dts_cfg.pmu.pmu_bat_shutdown_ltf, dts_cfg.pmu.pmu_bat_shutdown_htf, dts_cfg.pmu.pmu_pwroff_vol, dts_cfg.pmu.power_start);

  /* parse box_start_os node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,box_start_os");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/box_start_os0");
  if (!IS_ERR_VALUE(nodeoffset)) {
    dts_cfg.start_os.used = fdtdec_get_is_enabled(working_fdt, nodeoffset);
    if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "start_type", &dts_cfg.start_os.start_type))) {
      ARISC_ERR("parse box_start_os->start_type fail\n");
    }
    if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "irkey_used", &dts_cfg.start_os.irkey_used))) {
      ARISC_ERR("parse box_start_os->irkey_used fail\n");
    }
    if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "pmukey_used", &dts_cfg.start_os.pmukey_used))) {
      ARISC_ERR("parse box_start_os->pmukey_used fail\n");
    }
  }

  return 0;
}
#else
static void sunxi_arisc_parse_cfg(void)
{
  dts_cfg.power.powchk_used = 0;
  dts_cfg.power.power_reg = 0x02309621;
  dts_cfg.power.system_power=50;
  ARISC_INF("powchk_used:0x%x\n", dts_cfg.power.powchk_used);
  ARISC_INF("power_reg:0x%x\n", dts_cfg.power.power_reg);

  ARISC_INF("system_power:0x%x\n", dts_cfg.power.system_power);
  /* parse arisc_space node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "/soc/arisc_space");
  dts_cfg.space.sram_dst = 0x00040000;
  dts_cfg.space.sram_offset = 0x00000000;
  dts_cfg.space.sram_size = 0x00014000;

  //space2
  dts_cfg.space.dram_dst = 0x40100000;
  dts_cfg.space.dram_offset = 0x00018000;
  dts_cfg.space.dram_size = 0x00004000;

  //space3
  dts_cfg.space.para_dst = 0x40104000;
  dts_cfg.space.para_offset = 0x00000000;
  dts_cfg.space.para_size = 0x00001000;

  //space4
  dts_cfg.space.msgpool_dst = 0x40105000;
  dts_cfg.space.msgpool_offset = 0x00000000;
  dts_cfg.space.msgpool_size = 0x00001000;

  ARISC_INF("arisc_space space1 sram_dst:0x%p, sram_offset:0x%p, sram_size:0x%zx, \n",
    (void *)dts_cfg.space.sram_dst, (void *)dts_cfg.space.sram_offset, dts_cfg.space.sram_size);
  ARISC_INF("arisc_space space2 dram_dst:0x%p, dram_offset:0x%p, dram_size:0x%zx, \n",
    (void *)dts_cfg.space.dram_dst, (void *)dts_cfg.space.dram_offset, dts_cfg.space.dram_size);
  ARISC_INF("arisc_space space3 para_dst:0x%p, para_offset:0x%p, para_size:0x%zx, \n",
    (void *)dts_cfg.space.para_dst, (void *)dts_cfg.space.para_offset, dts_cfg.space.para_size);
  ARISC_INF("arisc_space space4 msgpool_dst:0x%p, msgpool_offset:0x%p, msgpool_size:0x%zx, \n",
    (void *)dts_cfg.space.msgpool_dst, (void *)dts_cfg.space.msgpool_offset, dts_cfg.space.msgpool_size);

  /* parse standby_space node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,standby_space");
  dts_cfg.space.standby_dst = 0x41020000;
  dts_cfg.space.standby_offset = 0x00000000;
  dts_cfg.space.standby_size = 0x00000800;

  ARISC_INF("standby_space space1 standby_dst:0x%p, standby_offset:0x%p, standby_size:0x%zx, \n",
    (void *)dts_cfg.space.standby_dst, (void *)dts_cfg.space.standby_offset, dts_cfg.space.standby_size);

  /* parse image para */
  #if 0
  get_image_addr_size(&dts_cfg.image.base, &dts_cfg.image.size);
  ARISC_INF("image base:0x%p, image size:0x%zx\n", (void *)dts_cfg.image.base, dts_cfg.image.size);
  #endif
  /* parse dram node */
  //memcpy((void *)&dts_cfg.dram_para, (void *)(uboot_spare_head.boot_data.dram_para), sizeof(dts_cfg.dram_para));

  /* parse prcm node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,prcm");
  dts_cfg.prcm.base = 0x01f01400;
  dts_cfg.prcm.size = 0x400;

  ARISC_INF("prcm base:0x%p, size:0x%zx\n",
    (void *)dts_cfg.prcm.base, dts_cfg.prcm.size);

  /* parse cpuscfg node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,cpuscfg");
  dts_cfg.cpuscfg.base = 0x01f01c00;
  dts_cfg.cpuscfg.size = 0x400;

  ARISC_INF("cpuscfg base:0x%p, size:0x%zx\n",
    (void *)dts_cfg.cpuscfg.base, dts_cfg.cpuscfg.size);

  /* parse msgbox node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,msgbox");
  dts_cfg.msgbox.base = 0x01c17000;
  dts_cfg.msgbox.size = 0x1000;

  dts_cfg.msgbox.status = 1;

  ARISC_INF("msgbox base:0x%p, size:0x%zx, status:%u\n",
    (void *)dts_cfg.msgbox.base, dts_cfg.msgbox.size, dts_cfg.msgbox.status);

  /* parse hwspinlock node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,sunxi-hwspinlock");
  dts_cfg.hwspinlock.base = 0x01c18000;
  dts_cfg.hwspinlock.size = 0x1000;

  dts_cfg.hwspinlock.status = 1;

  ARISC_INF("hwspinlock base:0x%p, size:0x%zx, status:%u\n",
    (void *)dts_cfg.hwspinlock.base, dts_cfg.hwspinlock.size, dts_cfg.hwspinlock.status);

  /* parse s_uart node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,s_uart");
  dts_cfg.s_uart.base = 0x01f02800;
  dts_cfg.s_uart.size = 0x400;

  ARISC_INF("s_uart base:0x%p, size:0x%zx\n",
    (void *)dts_cfg.s_uart.base, dts_cfg.s_uart.size);

  dts_cfg.s_uart.status = 1;

  ARISC_INF("s_uart base:0x%p, size:0x%zx, status:%u\n",
    (void *)dts_cfg.s_uart.base, dts_cfg.s_uart.size, dts_cfg.s_uart.status);

#if defined CONFIG_ARCH_SUN50IW2P1
  /* parse s_twi node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,s_twi");
  nodeoffset = fdt_path_offset(working_fdt, "/soc/s_twi");
  if (IS_ERR_VALUE(nodeoffset)) {
    ARISC_ERR("get [allwinner,s_twi] device node error\n");
    return -EINVAL;
  }

  if (IS_ERR_VALUE(fdt_getprop_u32(working_fdt, nodeoffset, "reg", value))) {
    ARISC_ERR("get s_twi reg error\n");
    return -EINVAL;
  }

  dts_cfg.s_twi.base = (phys_addr_t)value[1];
  dts_cfg.s_twi.size = (size_t)value[3];

  dts_cfg.s_twi.status = fdtdec_get_is_enabled(working_fdt, nodeoffset);

  ARISC_INF("s_twi base:0x%p, size:0x%zx, status:%u\n",
    (void *)dts_cfg.s_twi.base, dts_cfg.s_twi.size, dts_cfg.s_twi.status);
#else
  /* parse s_rsb node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,s_rsb");
  dts_cfg.s_rsb.base = 0x01f03400;
  dts_cfg.s_rsb.size = 0x400;

  dts_cfg.s_rsb.status = 1;

  ARISC_INF("s_rsb base:0x%p, size:0x%zx, status:%u\n",
    (void *)dts_cfg.s_rsb.base, dts_cfg.s_rsb.size, dts_cfg.s_rsb.status);
#endif
  /* parse s_jtag node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,s_jtag");
  dts_cfg.s_jtag.status = 0;

  ARISC_INF("s_jtag status:%u\n", dts_cfg.s_jtag.status);

  /* parse dvfs_table node */ 
  dts_cfg.vf[0].freq= 1200000000;
  dts_cfg.vf[0].voltage= 1300;
  dts_cfg.vf[1].freq= 1008000000;
  dts_cfg.vf[1].voltage= 1200;
  dts_cfg.vf[2].freq= 816000000;
  dts_cfg.vf[2].voltage= 1100;    
  dts_cfg.vf[3].freq= 648000000;
  dts_cfg.vf[3].voltage= 1040;
  dts_cfg.vf[4].freq= 0;
  dts_cfg.vf[4].voltage= 1300;
  dts_cfg.vf[5].freq= 0;
  dts_cfg.vf[5].voltage= 1300;    
  dts_cfg.vf[6].freq= 0;
  dts_cfg.vf[6].voltage= 1300;
  dts_cfg.vf[7].freq= 0;
  dts_cfg.vf[7].voltage= 1300;

  /* parse s_cir node */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,s_cir");
  dts_cfg.s_cir.ir_key.num = 0;

  /* config pmu config paras */
  //nodeoffset = fdt_node_offset_by_compatible(working_fdt, -1, "allwinner,pmu0");
  dts_cfg.pmu.pmu_bat_shutdown_ltf=3200;
  dts_cfg.pmu.pmu_bat_shutdown_htf=237;
  dts_cfg.pmu.pmu_pwroff_vol=3300;
  dts_cfg.pmu.power_start = 0;

  dts_cfg.start_os.used = 0;
  dts_cfg.start_os.start_type = 0;
  dts_cfg.start_os.irkey_used = 0;
  dts_cfg.start_os.pmukey_used = 0;
  ARISC_INF("pmu pmu_bat_shutdown_ltf:0x%x, pmu_bat_shutdown_htf:0x%x, pmu_pwroff_vol:0x%x, power_start:0x%x\n",
  dts_cfg.pmu.pmu_bat_shutdown_ltf, dts_cfg.pmu.pmu_bat_shutdown_htf, dts_cfg.pmu.pmu_pwroff_vol, dts_cfg.pmu.power_start);
}
#endif
static void sunxi_arisc_transform_cfg(void)
{

  memcpy((void *)&dts_cfg_64.dram_para, (const void *)&dts_cfg.dram_para, sizeof(struct dram_para));
  memcpy((void *)&dts_cfg_64.vf, (const void *)&dts_cfg.vf, sizeof(struct arisc_freq_voltage) * ARISC_DVFS_VF_TABLE_MAX);

  dts_cfg_64.space.sram_dst = (u64)dts_cfg.space.sram_dst;
  dts_cfg_64.space.sram_offset = (u64)dts_cfg.space.sram_offset;
  dts_cfg_64.space.sram_size = (u64)dts_cfg.space.sram_size;
  dts_cfg_64.space.dram_dst = (u64)dts_cfg.space.dram_dst;
  dts_cfg_64.space.dram_offset = (u64)dts_cfg.space.dram_offset;
  dts_cfg_64.space.dram_size = (u64)dts_cfg.space.dram_size;
  dts_cfg_64.space.para_dst = (u64)dts_cfg.space.para_dst;
  dts_cfg_64.space.para_offset = (u64)dts_cfg.space.para_offset;
  dts_cfg_64.space.para_size = (u64)dts_cfg.space.para_size;
  dts_cfg_64.space.msgpool_dst = (u64)dts_cfg.space.msgpool_dst;
  dts_cfg_64.space.msgpool_offset = (u64)dts_cfg.space.msgpool_offset;
  dts_cfg_64.space.msgpool_size = (u64)dts_cfg.space.msgpool_size;
  dts_cfg_64.space.standby_dst = (u64)dts_cfg.space.standby_dst;
  dts_cfg_64.space.standby_offset = (u64)dts_cfg.space.standby_offset;
  dts_cfg_64.space.standby_size = (u64)dts_cfg.space.standby_size;

  dts_cfg_64.image.base = (u64)dts_cfg.image.base;
  dts_cfg_64.image.size = (u64)dts_cfg.image.size;

  dts_cfg_64.prcm.base = (u64)dts_cfg.prcm.base;
  dts_cfg_64.prcm.size = (u64)dts_cfg.prcm.size;

  dts_cfg_64.cpuscfg.base = (u64)dts_cfg.cpuscfg.base;
  dts_cfg_64.cpuscfg.size = (u64)dts_cfg.cpuscfg.size;

  dts_cfg_64.msgbox.base = (u64)dts_cfg.msgbox.base;
  dts_cfg_64.msgbox.size = (u64)dts_cfg.msgbox.size;
  dts_cfg_64.msgbox.irq = dts_cfg.msgbox.irq;
  dts_cfg_64.msgbox.status = dts_cfg.msgbox.status;

  dts_cfg_64.hwspinlock.base = (u64)dts_cfg.hwspinlock.base;
  dts_cfg_64.hwspinlock.size = (u64)dts_cfg.hwspinlock.size;
  dts_cfg_64.hwspinlock.irq = dts_cfg.hwspinlock.irq;
  dts_cfg_64.hwspinlock.status = dts_cfg.hwspinlock.status;

  dts_cfg_64.s_uart.base = (u64)dts_cfg.s_uart.base;
  dts_cfg_64.s_uart.size = (u64)dts_cfg.s_uart.size;
  dts_cfg_64.s_uart.irq = dts_cfg.s_uart.irq;
  dts_cfg_64.s_uart.status = dts_cfg.s_uart.status;

#if defined CONFIG_ARCH_SUN50IW2P1
  dts_cfg_64.s_twi.base = (u64)dts_cfg.s_twi.base;
  dts_cfg_64.s_twi.size = (u64)dts_cfg.s_twi.size;
  dts_cfg_64.s_twi.irq = dts_cfg.s_twi.irq;
  dts_cfg_64.s_twi.status = dts_cfg.s_twi.status;
#else
  dts_cfg_64.s_rsb.base = (u64)dts_cfg.s_rsb.base;
  dts_cfg_64.s_rsb.size = (u64)dts_cfg.s_rsb.size;
  dts_cfg_64.s_rsb.irq = dts_cfg.s_rsb.irq;
  dts_cfg_64.s_rsb.status = dts_cfg.s_rsb.status;
#endif
  dts_cfg_64.s_jtag.base = (u64)dts_cfg.s_jtag.base;
  dts_cfg_64.s_jtag.size = (u64)dts_cfg.s_jtag.size;
  dts_cfg_64.s_jtag.irq = dts_cfg.s_jtag.irq;
  dts_cfg_64.s_jtag.status = dts_cfg.s_jtag.status;

  dts_cfg_64.s_cir.base = (u64)dts_cfg.s_cir.base;
  dts_cfg_64.s_cir.size = (u64)dts_cfg.s_cir.size;
  dts_cfg_64.s_cir.irq = dts_cfg.s_cir.irq;
  dts_cfg_64.s_cir.status = dts_cfg.s_cir.status;
  memcpy(&dts_cfg_64.s_cir.ir_key, &dts_cfg.s_cir.ir_key, sizeof(ir_key_t));
  memcpy(&dts_cfg_64.start_os, &dts_cfg.start_os, sizeof(box_start_os_cfg_t));
  memcpy((void *)&dts_cfg_64.pmu, (const void *)&dts_cfg.pmu, sizeof(struct pmu_cfg));
  memcpy((void *)&dts_cfg_64.power, (const void *)&dts_cfg.power, sizeof(struct power_cfg));
  WriteBackDataCacheRange((void*)&dts_cfg_64, sizeof(struct dts_cfg_64));

}

int sunxi_arisc_wait_ready(void)
{
    //if(get_boot_work_mode()!= WORK_MODE_BOOT)
   //     {
    //            return 0;
   //     }
  arm_svc_arisc_wait_ready();
  return 0;
}

int sunxi_arisc_fake_poweroff(void)
{
  arm_svc_arisc_fake_poweroff();

  return 0;
}

int sunxi_arisc_probe(void)
{
//  if(!sunxi_probe_secure_monitor())
//  {
//    return 0;
//  }

  ARISC_LOG("arisc initialize\n");
  sunxi_arisc_parse_cfg();

  /* because the uboot is arch32,
   * and the runtime server is arch64,
   * should transform the dts_cfg to dts_cfg_64.
   */
  sunxi_arisc_transform_cfg();
  ARISC_LOG("arisc para ok\n");

  if (arm_svc_arisc_startup((unsigned long)&dts_cfg_64)) {
    /* arisc initialize failed */
    ARISC_ERR("sunxi-arisc driver startup failed\n");
  } else {
    /* arisc initialize succeeded */
    ARISC_LOG("sunxi-arisc driver startup succeeded\n");
  }

  return 0;
}


