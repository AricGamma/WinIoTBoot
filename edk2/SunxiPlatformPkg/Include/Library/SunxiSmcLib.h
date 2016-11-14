/** @file

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SUNXI_SMC_LIB_H__
#define __SUNXI_SMC_LIB_H__

#define PSCI_SUCCESS (0)
#define PSCI_NOT_SUPPORTED (-1)
#define PSCI_INVAILD_PARAMETERS (-2)
#define PSCI_DENIED (-3)
#define PSCI_ALREADY_ON (-4)
#define PSCI_ON_PENDING (-5)
#define PSCI_INTERNAL_FAILURE (-6)
#define PSCI_NOT_PRESENT (-7)
#define PSCI_DISABLED (-8)


UINT32 smc_readl(UINT32 addr);
void smc_writel(UINT32 val,UINT32 addr);

UINT32 sunxi_smc_call(UINT32 arg0, UINT32 arg1, UINT32 arg2, UINT32 arg3,UINT32 pResult);
INT32 arm_svc_version(UINT32* major, UINT32* minor);
INT32 arm_svc_call_count(void);
INT32 arm_svc_uuid(UINT32 *uuid);
INT32 arm_svc_run_os(UINT32 kernel, UINT32 fdt, UINT32 arg2);
UINT32 arm_svc_read_sec_reg(UINT32 reg);
INT32 arm_svc_write_sec_reg(UINT32 val,UINT32 reg);
INT32 arm_svc_arisc_startup(UINT32 cfg_base);
INT32 arm_svc_arisc_wait_ready(void);
INT32 arm_svc_arisc_fake_poweroff(void);
UINT32 arm_svc_arisc_read_pmu(UINT32 addr);
INT32 arm_svc_arisc_write_pmu(UINT32 addr,UINT32 value);

INT32 arm_svc_efuse_read(void *key_buf, void *read_buf);
INT32 arm_svc_efuse_write(void *key_buf);

INT32 arm_svc_cpu_on(UINT32 cpuid,UINT32 entrypoint);

INT32 smc_init(void);

#endif
