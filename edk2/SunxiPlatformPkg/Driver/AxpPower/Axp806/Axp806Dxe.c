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
#include <Axp806.h>

STATIC EFI_SMBUS_HC_PROTOCOL *Smbus;

STATIC EFI_STATUS Axp806DxePmBusRead(UINT8 DeviceRegister, UINT8 *Buffer)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1 + 1; //deviceID + device address + data
  UINT8                    DeviceBuffer[3];
  EFI_SMBUS_DEVICE_ADDRESS AxpSlaveAddress; 
    
  AxpSlaveAddress.SmbusDeviceAddress = AXP806_ADDR>>1;
  DeviceBuffer[0] = AXP806_ADDR;
  DeviceBuffer[1] = DeviceRegister;
  Status = Smbus->Execute(Smbus,AxpSlaveAddress,0,\
  EfiSmbusReadBlock, FALSE, &DeviceBufferLength, DeviceBuffer);
  Buffer[0] = DeviceBuffer[2];
  return Status;
}

STATIC EFI_STATUS Axp806DxePmBusWrite(UINT8 DeviceRegister, UINT8 data)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1 + 1; //deviceID + device address + data
  UINT8                    DeviceBuffer[3];
  EFI_SMBUS_DEVICE_ADDRESS AxpSlaveAddress; 
    
  AxpSlaveAddress.SmbusDeviceAddress = AXP806_ADDR>>1;
  DeviceBuffer[0] = AXP806_ADDR;
  DeviceBuffer[1] = DeviceRegister;
  DeviceBuffer[2] = data;

  Status = Smbus->Execute(Smbus, AxpSlaveAddress, 0,\
  EfiSmbusWriteBlock, FALSE, &DeviceBufferLength, DeviceBuffer);
  return Status;
}

AXP_PM_BUS_OPS Axp806PmBusOps = {
.AxpPmBusRead  = Axp806DxePmBusRead,
.AxpPmBusWrite = Axp806DxePmBusWrite
};

STATIC AXP_POWER_PROTOCOL Axp806PowerProtocol = {
 .AxpPowerId         = AXP_POWER_ID_AXP806,
 .Probe          = Axp806Probe,
 .SetSupplyStatus        = Axp806SetSupplyStatus,
 .SetSupplyStatusByName  = Axp806SetSupplyStatusByName,
 .ProbeSupplyStatus      = Axp806ProbeSupplyStatus,
 .ProbeSupplyStatusByName= Axp806ProbeSupplyStatusByName,
 
 .SetNextSystemMode      = Axp806SetNextSysMode,
 .ProbePreSystemMode     = Axp806ProbePreSysMode,
 .ProbeThisPowerOnCause  = Axp806ProbeThisPowerOnCause,
 
 .ProbePowerBusExistance = Axp806ProbePowerBusExistance,
 
 .ProbeBatteryVoltage    = Axp806ProbeBatteryVoltage,
 .ProbeBatteryRatio      = Axp806ProbeBatteryRatio,
 .ProbeBatteryExistance  = Axp806ProbeBatteryExistance,
 
 .ProbePowerKey          = Axp806ProbePowerKey,
 
 .SetPowerOff            = Axp806SetPowerOff,
 .SetPowerOnOffVoltage   = Axp806SetPowerOnoffVoltage,
 
 .SetChargerOnOff        = Axp806SetChargerOnOff,
 .SetVbusCurrentLimit    = Axp806SetVbusCurrentLimit,
 .SetVbusVoltageLimit    = Axp806SetVbusVoltagelimit,
 .SetChargeCurrent       = Axp806SetChargeCurrent,
 .ProbeChargeCurrent     = Axp806ProbeChargeCurrent,
 
 .ProbeIntPendding       = Axp806ProbeIntPending,
 .ProbeIntEnable         = Axp806ProbeIntEnable,
 .SetIntEnable           = Axp806SetIntEnable,
 .SetIntDisable          = Axp806SetIntDisable
};

EFI_STATUS
Axp806Initialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;
  
  Status = gBS->LocateProtocol(&gEfiSmbusHcProtocolGuid, NULL, (VOID **)&Smbus);
  ASSERT_EFI_ERROR(Status);
  
  Status = gBS->InstallMultipleProtocolInterfaces(&ImageHandle, &gAxpPowerProtocolGuid, &Axp806PowerProtocol, NULL);
  return Status;
}
