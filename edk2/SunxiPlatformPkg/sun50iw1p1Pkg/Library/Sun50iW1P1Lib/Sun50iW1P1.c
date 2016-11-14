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
#include <Library/ArmArchTimerLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>
#include <Ppi/ArmMpCoreInfo.h>

#include <Include/Sun50iW1P1/platform.h>
#include <Include/Sun50iW1P1/ccmu.h>
#include <Include/Sun50iW1P1/cpu.h>
#include <Include/Sun50iW1P1/clock.h>

#include <Interinc/sunxi_uefi.h>

#include <Library/SunxiSmBusLib.h>
#include <Library/SysConfigLib.h>
#include <Library/SunxiSmcLib.h>

#include <Protocol/AxpPower.h>

#include <Driver/AxpPower/Axp81X/Axp81X.h>

#include <Sunxi_type/Sunxi_type.h>
#include <Sun50iW1P1/platform.h>
#include <Library/SunxiBootInfoLib.h>

#include <Sun50iW1P1/gic.h>
#include <Sun50iW1P1/usb.h>
#include <Sun50iW1P1/key.h>
#include "Arisc.h"

#define AW_CPU_JUMP_MAIL_ADDRESS (SUNXI_RPRCM_BASE+0x164)

#define SUNXI_RUN_SHUTDOWN_ADDR  (SUNXI_RPRCM_BASE + 0x1f0)

#define SUNXI_WDOG_CTL    (SUNXI_TIMER_BASE+0xB0)
#define SUNXI_WDOG_CFG    (SUNXI_TIMER_BASE+0xB4)
#define SUNXI_WDOG_MODE   (SUNXI_TIMER_BASE+0xB8)

extern int SunxiClockSetC0CorePll(int frequency);
extern int SunxiClockGetC0CorePll(void);

#define PMU_SCRIPT_NAME          ("pmu0")


ARM_CORE_INFO mSun50iW1P1MpCoreInfo[] = {
  {
    // Cluster 0, Core 0
    0x0, 0x0,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (UINT64)0xFFFFFFFF
  },
  {
    // Cluster 0, Core 1
    0x0, 0x1,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (UINT64)0xFFFFFFFF
  },
  {
    // Cluster 0, Core 2
    0x0, 0x2,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (UINT64)0xFFFFFFFF
  },
  {
    // Cluster 0, Core 3
    0x0, 0x3,

    // MP Core MailBox Set/Get/Clear Addresses and Clear Value
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (EFI_PHYSICAL_ADDRESS)AW_CPU_JUMP_MAIL_ADDRESS,
    (UINT64)0xFFFFFFFF
  }

};

#if 1
static RETURN_STATUS Cluster0CpuVFS(UINT32 Corevoltage,UINT32 CoreClock)
{
  UINTN Pll;
  if(!CoreClock || !Corevoltage)
  {
    CoreClock = 1008;
    Corevoltage = 1100;
  }
  if(Axp81XProbe() == EFI_SUCCESS)
  {
    if(!Axp81XSetSupplyStatus(PMU_SUPPLY_DCDC2, Corevoltage, -1))
    {
      DEBUG((EFI_D_INIT,"AXP81X: dcdc2 %d\n", Corevoltage));
      SunxiClockSetC0CorePll(CoreClock);
    }
    else
    {  
      DEBUG((EFI_D_ERROR,"Set core voltage failed\n"));
    }
  }
  else
  {
    DEBUG((EFI_D_ERROR,"Axp Probe Error\n"));
  }

  Pll = SunxiClockGetC0CorePll();
  DEBUG((EFI_D_ERROR,"CPU Cluster0: pll %d Mhz\n",Pll));

  return RETURN_SUCCESS;
}
#endif

RETURN_STATUS AxpSetChargeCurrentLimit(void)
{
  INT32 Ret;
  INT32 ChargeCurrent;
  Ret = script_parser_fetch(PMU_SCRIPT_NAME, "pmu_runtime_chgcur", &ChargeCurrent, 1);
  DEBUG((EFI_D_INIT,"PMU: Charge Current= %d\n", ChargeCurrent));
  if(!Ret)
  {    
    ASSERT_EFI_ERROR(Axp81XSetChargeCurrent(ChargeCurrent));
  }else
  {
    DEBUG((EFI_D_ERROR,"PMU: Fetch Charge Current failed\n"));
  }
  return RETURN_SUCCESS;
}

RETURN_STATUS AxpSetUsbVoltageAndCurrentLimit(void)
{
  INT32 Ret;
  INT32 AcVoltageLimit,AcCurrentLimit;
  INT32 PcVoltageLimit,PcCurrentLimit;

  Ret = script_parser_fetch(PMU_SCRIPT_NAME, "pmu_ac_vol", &AcVoltageLimit, 1);
  if(!Ret)
  {
    DEBUG((EFI_D_INIT,"PMU: pmu_ac_vol= %dmV\n", AcVoltageLimit));
  }else
  {
    DEBUG((EFI_D_ERROR,"PMU: Fetch Usb AC Volage Limitation failed\n"));
  }

  Ret = script_parser_fetch(PMU_SCRIPT_NAME, "pmu_ac_cur", &AcCurrentLimit, 1);
  if(!Ret)
  {
    DEBUG((EFI_D_INIT,"PMU: pmu_ac_cur= %dmV\n", AcCurrentLimit));
  }else
  {
    DEBUG((EFI_D_ERROR,"PMU: Fetch Usb AC Current Limitation failed\n"));
  }
  
  Ret = script_parser_fetch(PMU_SCRIPT_NAME, "pmu_usbpc_vol", &PcVoltageLimit, 1);
  
  if(!Ret)
  {  if(!PcVoltageLimit) PcVoltageLimit = 0;
    DEBUG((EFI_D_INIT,"PMU: pmu_usbpc_vol= %dmV\n", PcVoltageLimit));
    ASSERT_EFI_ERROR(Axp81XSetVbusVoltagelimit(PcVoltageLimit));
  }else
  {
    DEBUG((EFI_D_ERROR,"PMU: Fetch Usb PC Volage failed\n"));
  }

  Ret = script_parser_fetch(PMU_SCRIPT_NAME, "pmu_usbpc_cur", &PcCurrentLimit, 1);
  
  if(!Ret)
  { 
    if(!PcCurrentLimit) PcCurrentLimit = 0;
    DEBUG((EFI_D_INIT,"PMU: pmu_usbpc_cur= %dmA\n", PcCurrentLimit));
    ASSERT_EFI_ERROR(Axp81XSetVbusCurrentLimit(PcCurrentLimit));
  }else
  {
    DEBUG((EFI_D_ERROR,"PMU: Fetch Usb PC Current failed\n"));
  }
  
  return RETURN_SUCCESS;
}

RETURN_STATUS AxpSetPowerOnVoltage(void)
{
  INT32 Ret;
  INT32 Volage;
  Ret = script_parser_fetch(PMU_SCRIPT_NAME, "pmu_pwron_vol", &Volage, 1);
  if(!Ret)
  {    
    DEBUG((EFI_D_INIT,"PMU: pmu_pwron_vol= %dmV\n", Volage));
    ASSERT_EFI_ERROR(Axp81XSetPowerOnoffVoltage(Volage,1));
  }else
  {
    DEBUG((EFI_D_ERROR,"PMU: Fetch Usb Volage failed,set to default\n"));
    ASSERT_EFI_ERROR(Axp81XSetPowerOnoffVoltage(0,1));
  }
  
  return RETURN_SUCCESS;
}

RETURN_STATUS AxpPowerSupplySetOutPut(void)
{
  INTN  Ret, OnOff;
  UINTN PowerSupplyHd;
  char PowerName[16];
  int  PowerVoltage, PowerIndex = 0;

  PowerSupplyHd = script_parser_fetch_subkey_start("power_sply");
  if(!PowerSupplyHd)
  {
    DEBUG((EFI_D_ERROR,"unable to set power supply\n"));
    return RETURN_LOAD_ERROR;
  }
  do
  {
    ZeroMem(PowerName,sizeof(PowerName));
    Ret = script_parser_fetch_subkey_next(PowerSupplyHd, PowerName, &PowerVoltage, &PowerIndex);
    if(Ret < 0)
    {
      DEBUG((EFI_D_INIT,"find power_sply to end\n"));
      return RETURN_SUCCESS;
    }
    DEBUG((EFI_D_INIT,"%a = %d\n", PowerName, PowerVoltage));
    OnOff = 0;
    if(PowerVoltage>10000)
    {
      OnOff = 1;
      PowerVoltage = PowerVoltage % 10000;
    }

    if(Axp81XSetSupplyStatusByName(PowerName, PowerVoltage, OnOff))
    {
      DEBUG((EFI_D_ERROR,"axp set %a to %d failed\n", PowerName, PowerVoltage));
    }
  }
  while(1);

  return RETURN_SUCCESS;
}

STATIC UINT32 GicIrqDisable(UINT32 IrqNo)
{
  UINT32 reg_val;
  UINT32 offset;

  if (IrqNo >= GIC_IRQ_NUM)
  {
    DEBUG((DEBUG_INIT,"irq NO.(%d) > GIC_IRQ_NUM(%d) !!\n", IrqNo, GIC_IRQ_NUM));
    return -1;
  }

  offset   = IrqNo >> 5; // ��32
  reg_val = (1 << (IrqNo & 0x1f));
  MmioWrite32(GIC_CLR_EN(offset),reg_val);

  return 0;
}
VOID SUsbDevConectSwitch(UINT32 IsOn)
{
  UINT32 RegValue;

  RegValue = MmioRead32(USBC_REG_PCTL(USBC0_BASE));
  if(IsOn)
  {
    RegValue |= USBC_BP_POWER_D_SOFT_CONNECT;
  }
  else
  {
    RegValue &= ~USBC_BP_POWER_D_SOFT_CONNECT;
  }
  MmioWrite32(USBC_REG_PCTL(USBC0_BASE),RegValue);
  GicIrqDisable(AW_IRQ_USB_OTG);
}

EFI_STATUS  SecondaryCpuBringUp(UINTN BringUpEntry){
  UINTN i;
  UINTN psci_status;
  for(i=1;i<4;i++){ 
    psci_status = arm_svc_cpu_on(i,BringUpEntry);
    DEBUG((EFI_D_INFO,"enable cpu %d with status 0x%x\n",i,psci_status));      
  }
  return EFI_SUCCESS;
}

void RequestDeviceIoFromSysConfig(void){

gpio_request_simple("codec",NULL);
gpio_request_simple("wlan",NULL);
gpio_request_simple("gmac0",NULL);
gpio_request_simple("ctp_para",NULL);
gpio_request_simple("i2s0",NULL);
gpio_request_simple("pcm1",NULL);
gpio_request_simple("csi0",NULL);
gpio_request_simple("amp",NULL);
}

/**
  Return the current Boot Mode

  This function returns the boot reason on the platform

**/
EFI_BOOT_MODE
ArmPlatformGetBootMode (
  VOID
  )
{
  EFI_STATUS Status;
  UINT32 bootMode;

  //UINT32 ShutDownMagic;

  struct spare_boot_head_t* spare_head;

  spare_head = get_spare_head(PcdGet32 (PcdFdBaseAddress));

  SUsbDevConectSwitch(0);

  bootMode = SunxiLibGetBootMode();
  if (bootMode != WORK_MODE_BOOT )
  {
    return BOOT_WITH_FULL_CONFIGURATION;
  }
  
  DEBUG((EFI_D_INIT," sunxi_arisc_probe\n"));
  sunxi_arisc_probe();
  
  DEBUG((EFI_D_INIT,"SmBusInit\n"));
  Status =  SmBusInit();
  
  ASSERT_EFI_ERROR (Status);

  #if 1
  //Cluster0CpuVFS(0,0);
  Cluster0CpuVFS(spare_head->boot_data.run_core_vol,spare_head->boot_data.run_clock);
  AxpSetChargeCurrentLimit();
  AxpSetUsbVoltageAndCurrentLimit();

  AxpPowerSupplySetOutPut();
  AxpSetPowerOnVoltage();
  #endif

  RequestDeviceIoFromSysConfig();
  SecondaryCpuBringUp(PcdGet32 (PcdFdBaseAddress));

  return BOOT_WITH_FULL_CONFIGURATION;
}

/**
  Initialize controllers that must setup at the early stage

  Some peripherals must be initialized in Secure World.
  For example, some L2x0 requires to be initialized in Secure World

**/
RETURN_STATUS
ArmPlatformInitialize (
  IN  UINTN                     MpId
  )
{
 UINT32 Value;
  //use the Primary core to do voltage and frequency scaling.
  if (!ArmPlatformIsPrimaryCore (MpId)) {
    return RETURN_SUCCESS;
  }

  ArmArchTimerEnableTimer();

  //set sram for vedio use, default is boot use
  Value = MmioRead32(SUNXI_SYSCRL_BASE+0x4);
  Value &= ~(0x1<<24);
  MmioWrite32(SUNXI_SYSCRL_BASE+0x4,Value);
  
  //VE gating :brom set this bit, but not require now
  Value = MmioRead32(CCMU_BUS_CLK_GATING_REG1);
  Value &= ~(0x1<<0);
  MmioWrite32(CCMU_BUS_CLK_GATING_REG1,Value);
  
  //VE Bus Reset: brom set this bit, but not require now
  Value = MmioRead32(CCMU_BUS_SOFT_RST_REG1);
  Value &= ~(0x1<<0);
  MmioWrite32(CCMU_BUS_SOFT_RST_REG1,Value);
  
  return RETURN_SUCCESS;
}

/**
  Initialize the system (or sometimes called permanent) memory

  This memory is generally represented by the DRAM.

**/
VOID
ArmPlatformInitializeSystemMemory (
  VOID
  )
{ 
  // We do not need to initialize the System Memory,we do it in boot0 
  //but in order to configure the dram memory size at run time,we should fetch the
  //dram memory size parameter which stored in uefi head and apply it to the pcd 
  //configure database.
  //notice that, this function invoke isn't allowed as the cpu stack has not been 
  //set yet for both the BSP and AP,so stack need be get prepared in boot0.

  UINT64 *pDramSize=0;
  UINT64 *FbBase =0;
  UINT64 *MpParkSharedBase =0;
  UINT64 DramSizeForUefi =0;
  UINT64 ActualDramSize =0;
  
  struct spare_boot_head_t* spare_head;
  spare_head = get_spare_head(PcdGet32 (PcdFdBaseAddress));

  if(((spare_head->boot_data.dram_para[5]>>16)&0xffff)>2048) {
    spare_head->boot_data.dram_para[5] = (spare_head->boot_data.dram_para[5] & 0xffff) | (2048 << 16);
  }
    
  FbBase = (UINT64*)(&PcdGet64(PcdFrameBufferBase));
  MpParkSharedBase = (UINT64*)(&PcdGet64(PcdMpParkSharedBase)); 
  pDramSize  = (UINT64*)(&(PcdGet64(PcdSystemMemorySize)));

  if(!((spare_head->boot_data.dram_para[5]>>16)&0xffff)){
    spare_head->boot_data.dram_para[5] |= ((((*pDramSize)+PcdGet64(PcdMpParkSharedSize)+\
    PcdGet64(PcdFrameBufferSize))>>20)<<16)&(spare_head->boot_data.dram_para[5] & 0xffff);
  }

  ActualDramSize = ((spare_head->boot_data.dram_para[5]>>16)&0xffff)*1024*1024;
  if(ArmPlatformIsPrimaryCore (ArmReadMpidr())){
    if(ActualDramSize){   
      *pDramSize = ActualDramSize - 0x1000000;
      *FbBase    = PcdGet64(PcdSystemMemoryBase) + *pDramSize - PcdGet64(PcdFrameBufferSize);
      *MpParkSharedBase = *FbBase - PcdGet64(PcdMpParkSharedSize);        
      *pDramSize -=PcdGet64(PcdFrameBufferSize);
      *pDramSize -=PcdGet64(PcdMpParkSharedSize);
    }
  }
  else
  {
    DramSizeForUefi = ActualDramSize -(0x1000000+PcdGet64(PcdFrameBufferSize)+PcdGet64(PcdMpParkSharedSize));
    while(DramSizeForUefi != *pDramSize);
  }
  
  return;
}


EFI_STATUS
PrePeiCoreGetMpCoreInfo (
  OUT UINTN                   *CoreCount,
  OUT ARM_CORE_INFO           **ArmCoreTable
  )
{
  *CoreCount    = sizeof(mSun50iW1P1MpCoreInfo) / sizeof(ARM_CORE_INFO);
  *ArmCoreTable = mSun50iW1P1MpCoreInfo;

  return EFI_SUCCESS;
}

// Needs to be declared in the file. Otherwise gArmMpCoreInfoPpiGuid is undefined in the contect of PrePeiCore
EFI_GUID mArmMpCoreInfoPpiGuid = ARM_MP_CORE_INFO_PPI_GUID;
ARM_MP_CORE_INFO_PPI mMpCoreInfoPpi = { PrePeiCoreGetMpCoreInfo };

EFI_PEI_PPI_DESCRIPTOR gPlatformPpiTable[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &mArmMpCoreInfoPpiGuid,
    &mMpCoreInfoPpi
  }
};

VOID
ArmPlatformGetPlatformPpiList (
  OUT UINTN                   *PpiListSize,
  OUT EFI_PEI_PPI_DESCRIPTOR  **PpiList
  )
{
  
  *PpiListSize = sizeof(gPlatformPpiTable);
  *PpiList = gPlatformPpiTable;
}
