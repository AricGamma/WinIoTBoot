/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  WangWei <wangwei@allwinnertech.com>
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


#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/SmbusHc.h>
#include <Protocol/AxpPower.h>
#include <Axp81X.h>

STATIC EFI_SMBUS_HC_PROTOCOL *Smbus;

STATIC EFI_STATUS Axp81XPmBusRead(UINT8 DeviceRegister, UINT8 *Buffer)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1 + 1; //deviceID + device address + data
  UINT8                    DeviceBuffer[3];
  EFI_SMBUS_DEVICE_ADDRESS AxpSlaveAddress; 

  AxpSlaveAddress.SmbusDeviceAddress = AXP81X_ADDR>>1;
  DeviceBuffer[0] = AXP81X_ADDR;
  DeviceBuffer[1] = DeviceRegister;
  Status = Smbus->Execute(Smbus,AxpSlaveAddress,0,\
     EfiSmbusReadBlock, FALSE, &DeviceBufferLength, DeviceBuffer);
  Buffer[0] = DeviceBuffer[2];
  return Status;
}

STATIC EFI_STATUS Axp81XPmBusWrite(UINT8 DeviceRegister, UINT8 data)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1 + 1; //deviceID + device address + data
  UINT8                    DeviceBuffer[3];
  EFI_SMBUS_DEVICE_ADDRESS AxpSlaveAddress; 

  AxpSlaveAddress.SmbusDeviceAddress = AXP81X_ADDR>>1;
  DeviceBuffer[0] = AXP81X_ADDR;
  DeviceBuffer[1] = DeviceRegister;
  DeviceBuffer[2] = data;

  Status = Smbus->Execute(Smbus, AxpSlaveAddress, 0,\
     EfiSmbusWriteBlock, FALSE, &DeviceBufferLength, DeviceBuffer);
  return Status;
}

AXP_PM_BUS_OPS Axp81XPmBusOps = {
.AxpPmBusRead  = Axp81XPmBusRead,
.AxpPmBusWrite = Axp81XPmBusWrite
};

STATIC AXP_POWER_PROTOCOL Axp81XPowerProtocol = {
 .AxpPowerId         = AXP_POWER_ID_AXP81X,
 .Probe          = Axp81XProbe,
 .SetSupplyStatus        = Axp81XSetSupplyStatus,
 .SetSupplyStatusByName  = Axp81XSetSupplyStatusByName,
 .ProbeSupplyStatus      = Axp81XProbeSupplyStatus,
 .ProbeSupplyStatusByName= Axp81XProbeSupplyStatusByName,
 
 .SetNextSystemMode      = Axp81XSetNextSysMode,
 .ProbePreSystemMode     = Axp81XProbePreSysMode,
 .ProbeThisPowerOnCause  = Axp81XProbeThisPowerOnCause,
 
 .ProbePowerBusExistance = Axp81XProbePowerBusExistance,
 
 .ProbeBatteryVoltage    = Axp81XProbeBatteryVoltage,
 .ProbeBatteryRatio      = Axp81XProbeBatteryRatio,
 .ProbeBatteryExistance  = Axp81XProbeBatteryExistance,
 
 .ProbePowerKey          = Axp81XProbePowerKey,
 
 .SetPowerOff            = Axp81XSetPowerOff,
 .SetPowerOnOffVoltage   = Axp81XSetPowerOnoffVoltage,
 
 .SetChargerOnOff        = Axp81XSetChargerOnOff,
 .SetVbusCurrentLimit    = Axp81XSetVbusCurrentLimit,
 .ProbeVbusCurrentLimit  = Axp81XProbeVbusCurrentLimit,
 .SetVbusVoltageLimit    = Axp81XSetVbusVoltagelimit,
 .SetChargeCurrent       = Axp81XSetChargeCurrent,
 .ProbeChargeCurrent     = Axp81XProbeChargeCurrent,
 
 .ProbeIntPendding       = Axp81XProbeIntPending,
 .ProbeIntEnable         = Axp81XProbeIntEnable,
 .SetIntEnable           = Axp81XSetIntEnable,
 .SetIntDisable          = Axp81XSetIntDisable,
 .SetCoulombmeterOnOff   = Axp81XSetCoulombmeterOnOff
};

EFI_STATUS
Axp81XInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;
  
  Status = gBS->LocateProtocol(&gEfiSmbusHcProtocolGuid, NULL, (VOID **)&Smbus);
  ASSERT_EFI_ERROR(Status);
  
  Status = gBS->InstallMultipleProtocolInterfaces(&ImageHandle, &gAxpPowerProtocolGuid, &Axp81XPowerProtocol, NULL);
  return Status;
}
