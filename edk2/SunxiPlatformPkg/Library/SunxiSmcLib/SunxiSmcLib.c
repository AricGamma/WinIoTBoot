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

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

#define ARM_SVC_CALL_COUNT          0x8000ff00
#define ARM_SVC_UID                 0x8000ff01
//0x8000ff02 reserved
#define ARM_SVC_VERSION             0x8000ff03
#define ARM_SVC_RUNNSOS             0x8000ff04

#define ARM_SVC_READ_SEC_REG        0x8000ff05
#define ARM_SVC_WRITE_SEC_REG       0x8000ff06

//arisc
#define ARM_SVC_ARISC_STARTUP                   0x8000ff10
#define ARM_SVC_ARISC_WAIT_READY                0x8000ff11
#define ARM_SVC_ARISC_READ_PMU                  0x8000ff12
#define ARM_SVC_ARISC_WRITE_PMU                 0x8000ff13
#define ARM_SVC_ARISC_FAKE_POWER_OFF_REQ_ARCH32 0x8000ff14


//psci
#define ARM_SVC_PSCI_VERSION                    0x84000000
#define ARM_SVC_PSCI_CPU_SUSPEND                0x84000001
#define ARM_SVC_PSCI_CPU_OFF                    0x84000002
#define ARM_SVC_PSCI_CPU_ON                     0x84000003
#define ARM_SVC_PSCI_CPU_MIGRATE                0x84000004
#define ARM_SVC_PSCI_CPU_AFFINITY_INFO          0x84000005
#define ARM_SVC_PSCI_CPU_MIGRATE_INFO_TYPE      0x84000006

//efuse
#define ARM_SVC_EFUSE_READ         (0x8000fe00)
#define ARM_SVC_EFUSE_WRITE        (0x8000fe01)
/*
*note pResult is will
*/
extern UINT32 __sunxi_smc_call(UINT32 arg0,UINT32 arg1,UINT32 arg2,UINT32 arg3);

UINT32 sunxi_smc_call(UINT32 arg0, UINT32 arg1, UINT32 arg2, UINT32 arg3, UINT32 pResult)
{
  UINT32 ret = 0;
  //static UINT32 result[4] = {0};
  ret = __sunxi_smc_call(arg0,arg1,arg2,arg3);
  #if 0
  if(pResult != 0)
  {
    __asm volatile("str r0,[%0]": : "r" (result+0));
    __asm volatile("str r1,[%0]": : "r" (result+1));
    __asm volatile("str r2,[%0]": : "r" (result+2));
    __asm volatile("str r3,[%0]": : "r" (result+3));
    __asm volatile("": :  : "memory");
    *(UINT32*)pResult = (UINT32)result;
  }
  #endif
  return ret;
}

INT32 arm_svc_version(INT32* major, INT32* minor)
{
  UINT32 *pResult = NULL;
  INT32 ret = 0;
  ret = sunxi_smc_call(ARM_SVC_VERSION, 0, 0, 0, (UINT32)&pResult);
  if(ret < 0)
  {
    return ret;
  }

  *major = pResult[0];
  *minor = pResult[1];
  return ret;
}

INT32 arm_svc_call_count(void)
{
  return sunxi_smc_call(ARM_SVC_CALL_COUNT, 0, 0, 0,0);
}

INT32 arm_svc_uuid(UINT32 *uuid)
{
  return sunxi_smc_call(ARM_SVC_UID, 0, 0, 0, (UINT32)uuid);
}

INT32 arm_svc_run_os(UINT32 kernel, UINT32 fdt, UINT32 arg2)
{
  return sunxi_smc_call(ARM_SVC_RUNNSOS, kernel, fdt, arg2,0);
}

UINT32 arm_svc_read_sec_reg(UINT32 reg)
{
  return sunxi_smc_call(ARM_SVC_READ_SEC_REG, reg, 0, 0, 0);
}

INT32 arm_svc_write_sec_reg(UINT32 val,UINT32 reg)
{
  sunxi_smc_call(ARM_SVC_WRITE_SEC_REG, reg, val, 0,0);
  return 0;

}

INT32 arm_svc_arisc_startup(UINT32 cfg_base)
{
  return sunxi_smc_call(ARM_SVC_ARISC_STARTUP,cfg_base, 0, 0,0);
}

INT32 arm_svc_arisc_wait_ready(void)
{
  return sunxi_smc_call(ARM_SVC_ARISC_WAIT_READY, 0, 0, 0, 0);
}

int arm_svc_arisc_fake_poweroff(void)
{
  return sunxi_smc_call(ARM_SVC_ARISC_FAKE_POWER_OFF_REQ_ARCH32, 0, 0, 0, 0);
}

UINT32 arm_svc_arisc_read_pmu(UINT32 addr)
{
  return sunxi_smc_call(ARM_SVC_ARISC_READ_PMU,addr, 0, 0, 0);
}

INT32 arm_svc_arisc_write_pmu(UINT32 addr,UINT32 value)
{
  return sunxi_smc_call(ARM_SVC_ARISC_WRITE_PMU,addr, value, 0, 0);
}

int arm_svc_efuse_read(void *key_buf, void *read_buf)
{
  return sunxi_smc_call(ARM_SVC_EFUSE_READ, (UINT32)key_buf, (UINT32)read_buf, 0, 0);
}

INT32 arm_svc_efuse_write(void *key_buf)
{
  return sunxi_smc_call(ARM_SVC_EFUSE_WRITE, (UINT32)key_buf, 0, 0, 0);
}

INT32 arm_svc_cpu_on(UINT32 cpuid,UINT32 entrypoint)
{
  return sunxi_smc_call(ARM_SVC_PSCI_CPU_ON, cpuid, entrypoint, 0, 0);
}

static __inline UINT32 smc_readl_normal(UINT32 addr)
{
  //return readl(addr);
  return MmioRead32(addr);
}
static __inline INT32 smc_writel_normal(UINT32 value, UINT32 addr)
{
  //writel(value, addr);
  MmioWrite32(addr,value);
  return 0;
}

UINT32 (* smc_readl_pt)(UINT32 addr) = smc_readl_normal;
INT32 (* smc_writel_pt)(UINT32 value, UINT32 addr) = smc_writel_normal;

UINT32 smc_readl(UINT32 addr)
{
  return smc_readl_pt(addr);
}

void smc_writel(UINT32 val,UINT32 addr)
{
  smc_writel_pt(val,addr);
}

INT32 smc_efuse_readl(void *key_buf, void *read_buf)
{
  DEBUG((DEBUG_INFO,"smc_efuse_readl is just a interface,you must override it\n"));
  return 0;
}

INT32 smc_efuse_writel(void *key_buf)
{
  DEBUG((DEBUG_INFO,"smc_efuse_writel is just a interface,you must override it\n"));
  return 0;
}


INT32 smc_init(void)
{
//  if(sunxi_probe_secure_monitor())
  if(1)
  {
    smc_readl_pt = arm_svc_read_sec_reg;
    smc_writel_pt = arm_svc_write_sec_reg;
  }
  else
  {
    AsciiPrint("uboot:normal mode\n");
  }
  return 0;
}

