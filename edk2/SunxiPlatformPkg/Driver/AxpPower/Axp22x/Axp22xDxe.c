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


#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/SmbusHc.h>
#include <Protocol/AxpPower.h>
#include <Axp22x.h>

STATIC EFI_SMBUS_HC_PROTOCOL *Smbus;

STATIC EFI_STATUS Axp22xPmBusRead(UINT8 DeviceRegister, UINT8 *Buffer)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1; //device address + data
  UINT8                    DeviceBuffer[2];
  EFI_SMBUS_DEVICE_ADDRESS AxpSlaveAddress; 
    
  AxpSlaveAddress.SmbusDeviceAddress = AXP22X_ADDR>>1;
  DeviceBuffer[0] = DeviceRegister;
  Status = Smbus->Execute(Smbus,AxpSlaveAddress,0,\
  EfiSmbusReadBlock, FALSE, &DeviceBufferLength, DeviceBuffer);
  Buffer[0] = DeviceBuffer[1];
  return Status;
}

STATIC EFI_STATUS Axp22xPmBusWrite(UINT8 DeviceRegister, UINT8 data)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1; //device address + data
  UINT8                    DeviceBuffer[2];
  EFI_SMBUS_DEVICE_ADDRESS AxpSlaveAddress; 
    
  AxpSlaveAddress.SmbusDeviceAddress = AXP22X_ADDR>>1;
  DeviceBuffer[0] = DeviceRegister;
  DeviceBuffer[1] = data;

  Status = Smbus->Execute(Smbus, AxpSlaveAddress, 0,\
  EfiSmbusWriteBlock, FALSE, &DeviceBufferLength, DeviceBuffer);
  return Status;
}

AXP_PM_BUS_OPS Axp22xPmBusOps = {
.AxpPmBusRead  = Axp22xPmBusRead,
.AxpPmBusWrite = Axp22xPmBusWrite
};

STATIC AXP_POWER_PROTOCOL Axp22xPowerProtocol = {
 .AxpPowerId         = AXP_POWER_ID_AXP221,
 .Probe          = Axp22xProbe,
 .SetSupplyStatus        = Axp22xSetSupplyStatus,
 .SetSupplyStatusByName  = Axp22xSetSupplyStatusByName,
 .ProbeSupplyStatus      = Axp22xProbeSupplyStatus,
 
 .SetNextSystemMode      = Axp22xSetNextSysMode,
 .ProbePreSystemMode     = Axp22xProbePreSysMode,
 .ProbeThisPowerOnCause  = Axp22xProbeThisPowerOnCause,
 
 .ProbePowerBusExistance = Axp22xProbePowerBusExistance,
 
 .ProbeBatteryVoltage    = Axp22xProbeBatteryVoltage,
 .ProbeBatteryRatio      = Axp22xProbeBatteryRatio,
 .ProbeBatteryExistance  = Axp22xProbeBatteryExistance,
 
 .ProbePowerKey          = Axp22xProbePowerKey,
 
 .SetPowerOff            = Axp22xSetPowerOff,
 .SetPowerOnOffVoltage   = Axp22xSetPowerOnoffVoltage,
 
 .SetChargerOnOff        = Axp22xSetChargerOnOff,
 .SetVbusCurrentLimit    = Axp22xSetVbusCurrentLimit,
 .SetVbusVoltageLimit    = Axp22xSetVbusVoltagelimit,
 .SetChargeCurrent       = Axp22xSetChargeCurrent,
 .ProbeChargeCurrent     = Axp22xProbeChargeCurrent,
 
 .ProbeIntPendding       = Axp22xProbeIntPending,
 .ProbeIntEnable         = Axp22xProbeIntEnable,
 .SetIntEnable           = Axp22xSetIntEnable,
 .SetIntDisable          = Axp22xSetIntDisable
};

EFI_STATUS
Axp22xInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;
  
  Status = gBS->LocateProtocol(&gEfiSmbusHcProtocolGuid, NULL, (VOID **)&Smbus);
  ASSERT_EFI_ERROR(Status);
  
  Status = gBS->InstallMultipleProtocolInterfaces(&ImageHandle, &gAxpPowerProtocolGuid, &Axp22xPowerProtocol, NULL);
  return Status;
}
