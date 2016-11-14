/** @file
*
*  Copyright (c) 2011-2014, ARM Limited. All rights reserved.
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
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/SysConfigLib.h>
#include <Library/GraphicsLib.h>
#include <Library/TimerLib.h>

#include <Guid/SunxiVariableHob.h>

#include <Protocol/AxpPower.h>

#define PMU_SCRIPT_NAME  ("pmu0")

STATIC VOID ShutDown(AXP_POWER_PROTOCOL  *AxpPower)
{
  INT32 VolValue = 0;
  EFI_STATUS Status;
  if(script_parser_fetch("pmu_para", "pmu_pwroff_vol", &VolValue, 1))
  {
    AsciiPrint("set power off vol to default\n");
  }

  AsciiPrint("set next system normal\n");
  Status = AxpPower->SetNextSystemMode(0);
  if(EFI_ERROR(Status))
  {
    AsciiPrint("set next system mode error\n");
  }

  AsciiPrint("PowerOff\n");
  Status = AxpPower->SetPowerOnOffVoltage(VolValue,0);
  if(EFI_ERROR(Status))
  {
    AsciiPrint("set Power OnOff Voltage error\n");
  }
  Status = AxpPower->SetPowerOff();
}

STATIC INT32 ProbePreSystemMode(AXP_POWER_PROTOCOL  *AxpPower)
{
  UINTN PreSysMode = 0;
  EFI_STATUS Status;

  Status = AxpPower->ProbePreSystemMode(&PreSysMode);
  ASSERT_EFI_ERROR(Status);

  if(PreSysMode == PMU_PRE_SYS_MODE )
  {
    AsciiPrint("pre sys mode\n");
    return PMU_PRE_SYS_MODE;
  }
  else if(PreSysMode == PMU_PRE_BOOT_MODE)
  {
    AsciiPrint("pre boot mode\n");
    return PMU_PRE_BOOT_MODE;
  }
  else if(PreSysMode == PMU_PRE_FASTBOOT_MODE)
  {
    AsciiPrint("pre fastboot mode\n");
    return PMU_PRE_FASTBOOT_MODE;
  }

  return 0;
}

STATIC INT32 ProbeStartupCause(AXP_POWER_PROTOCOL  *AxpPower)
{
  UINTN PowerOnCause = 0;
  UINTN NextAction = 0;
  INT32 ret = 0;
  INT32 PreSysMode = 0;
  EFI_STATUS Status;

  PreSysMode = ProbePreSystemMode(AxpPower);
  if(PreSysMode)
  {
    return PreSysMode == PMU_PRE_BOOT_MODE ? PMU_PRE_BOOT_MODE:0;
  }

  Status = AxpPower->ProbeThisPowerOnCause(&PowerOnCause);
  ASSERT_EFI_ERROR(Status);
  NextAction = 0;
  if(PowerOnCause == AXP_POWER_ON_BY_POWER_KEY)
  {
    AsciiPrint("key trigger\n");
    NextAction = PMU_PRE_SYS_MODE;
    ret = 0;
  }
  else if(PowerOnCause == AXP_POWER_ON_BY_POWER_TRIGGER)
  {
    AsciiPrint("power trigger\n");
    NextAction = PMU_PRE_SYS_MODE;
    ret = AXP_POWER_ON_BY_POWER_TRIGGER;
  }
  AxpPower->SetNextSystemMode(NextAction);
  return ret;
}


//check battery and voltage
STATIC BOOLEAN PowerBatteryRatioEnougth(AXP_POWER_PROTOCOL  *AxpPower, UINTN *Val)
{
  UINTN  Ratio ;
  EFI_STATUS Status;

  Status = AxpPower->ProbeBatteryRatio(&Ratio);
  
  ASSERT_EFI_ERROR(Status);

  if(Val)
  {
    *Val = Ratio;
  }

  if(Ratio < 1)
  {
    if(AxpPower->SetCoulombmeterOnOff)
    {
      AxpPower->SetCoulombmeterOnOff(0);
      AxpPower->SetCoulombmeterOnOff(1); 
    }
  }
  
  if(Ratio >= 1)
  {
    return TRUE;
  }

  return FALSE;

}

STATIC VOID UpdateChargeVariable(VOID)
{
  EFI_STATUS Status;
  INT32 ChargeMode = 0;
  CHAR8 ChargeVar[64] = {0};

  if(0 == script_parser_fetch("charging_type", "charging_type", &ChargeMode, 1))
  {
    if(ChargeMode)
    {
      //set android chargemode
      AsciiStrCpy(ChargeVar, "Enable");
      Status = gRT->SetVariable (
        L"SunxiChargeMode",
        &gSunxiVariableGuid,
        (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
        AsciiStrLen(ChargeVar)+1, //add '\0' in the end
      ChargeVar);
      if(EFI_ERROR(Status))
      {
          AsciiPrint("Set SunxiChargeMode Variable Error\n");
      }
    }

  }
}
BOOLEAN PowerCheck(VOID)
{
  EFI_STATUS Status;
  AXP_POWER_PROTOCOL  *AxpPower = NULL;

  UINTN PressedKey = 0;
  UINTN BatExist = 0;
  UINTN PowerBus=0;
  UINTN BatVol=0;
  UINTN BatRatio=0;

  INT32 PowerStart = 0;
  INT32 SafeVol =0;
  INT32 PowerOnCause = 0;
  INT32 Ret = 0;
  BOOLEAN ChargeEnable=FALSE;
  BOOLEAN BatRatioEnougth;

  Status = gBS->LocateProtocol (&gAxpPowerProtocolGuid, NULL, (VOID **)&AxpPower);
  if(EFI_ERROR(Status))
  {
    return FALSE;
  }
  Status = AxpPower->ProbePowerKey(&PressedKey);

  //power_start
  // 0:  not allow staring up by insert dcin: press power key in long time & pre state is system state(Battery ratio shouled enough)
  // 1: allow starting up directly  by insert dcin:( Battery ratio shouled enough)
  // 2: not allow staring up by insert dcin:press power key in long time & pre state is system state(do not check battery ratio)
  // 3: allow starting up directly  by insert dcin:( do not check battery ratio)
    //
  script_parser_fetch(PMU_SCRIPT_NAME, "power_start", &PowerStart, 1);
    
  //check battery
  Status = AxpPower->ProbeBatteryExistance(&BatExist);
  if(BatExist <= 0)
  {
    AsciiPrint("no battery exist\n");
    return FALSE;
  }

  //check power bus
  Status = AxpPower->ProbePowerBusExistance(&PowerBus);
  if(EFI_ERROR(Status))
  {
    return FALSE;
  }

  AsciiPrint("PowerBus = %x(0: not exist 1:vBus 2:acBus 3:vBus&acBus)\n", PowerBus);

  //check battery ratio
  BatRatioEnougth = PowerBatteryRatioEnougth(AxpPower,&BatRatio);
  Status = AxpPower->ProbeBatteryVoltage(&BatVol);
  AsciiPrint("Battery Voltage=%d, Ratio=%d\n", BatVol, BatRatio);

  Ret = script_parser_fetch(PMU_SCRIPT_NAME, "pmu_safe_vol", &SafeVol, 1);
  if((Ret) || (SafeVol < 3000))
  {
    SafeVol = 3500;
  }

  if(!BatRatioEnougth)//low battery ratio
  {
    //power source not exist,shutdown directly
    if(PowerBus == 0)
    {
      AsciiPrint("battery ratio is low without  dc or ac, should be ShowDown\n");
      SunxiDisplayBmp("bat\\low_pwr.bmp");
      MicroSecondDelay(3000*1000);
      ShutDown(AxpPower);
    }

    //power soure exist
    if(PowerStart == 3)
    {
      return  FALSE;
    }
    else if((PowerStart == 0x0 || PowerStart == 0x1))
    {
      if(BatVol<SafeVol )
      {
        AsciiPrint("battery low power and vol with dc or ac, should charge longer\n");
        SunxiDisplayBmp("bat\\bempty.bmp");
        MicroSecondDelay(3000*1000);
        ShutDown(AxpPower);
      }
      else
      {
        PowerOnCause = ProbeStartupCause(AxpPower);
        if(PowerOnCause == PMU_PRE_BOOT_MODE || PowerOnCause ==AXP_POWER_ON_BY_POWER_TRIGGER)
        {
          //power source insert  or PreMode  is bootmode
          ChargeEnable = TRUE;
        }
        else
        {
          //key  trigger
          AsciiPrint("battery low power with dc or ac, should charge longer\n");
          SunxiDisplayBmp("bat\\bempty.bmp");
          MicroSecondDelay(3000*1000);
          ShutDown(AxpPower);
        }
      }

    }
  
  }
  else
  {
    //battery enougth
    PowerOnCause= ProbeStartupCause(AxpPower);

    if((PowerOnCause == PMU_PRE_BOOT_MODE) && (PowerBus==0))
    {
      ShutDown(AxpPower);
    }
    else if((PowerOnCause > 0) && PowerBus)
    {
      ChargeEnable = TRUE;
    }
  }
   
  if(ChargeEnable)
  {
    AsciiPrint("System Charge\n");
    //update variable
    UpdateChargeVariable();
  }

  return ChargeEnable;
    
}

