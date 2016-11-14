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
#include <Axp808.h>

STATIC EFI_SMBUS_HC_PROTOCOL *Smbus;

STATIC EFI_STATUS Axp808DxePmBusRead(UINT8 DeviceRegister, UINT8 *Buffer)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1 + 1; //deviceID + device address + data
  UINT8                    DeviceBuffer[3];
  EFI_SMBUS_DEVICE_ADDRESS AxpSlaveAddress; 
    
  AxpSlaveAddress.SmbusDeviceAddress = AXP808_ADDR>>1;
  DeviceBuffer[0] = AXP808_ADDR;
  DeviceBuffer[1] = DeviceRegister;
  Status = Smbus->Execute(Smbus,AxpSlaveAddress,0,\
  EfiSmbusReadBlock, FALSE, &DeviceBufferLength, DeviceBuffer);
  Buffer[0] = DeviceBuffer[2];
  return Status;
}

STATIC EFI_STATUS Axp808DxePmBusWrite(UINT8 DeviceRegister, UINT8 data)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1 + 1; //deviceID + device address + data
  UINT8                    DeviceBuffer[3];
  EFI_SMBUS_DEVICE_ADDRESS AxpSlaveAddress; 
    
  AxpSlaveAddress.SmbusDeviceAddress = AXP808_ADDR>>1;
  DeviceBuffer[0] = AXP808_ADDR;
  DeviceBuffer[1] = DeviceRegister;
  DeviceBuffer[2] = data;

  Status = Smbus->Execute(Smbus, AxpSlaveAddress, 0,\
       EfiSmbusWriteBlock, FALSE, &DeviceBufferLength, DeviceBuffer);
  return Status;
}

AXP_PM_BUS_OPS Axp808PmBusOps = {
.AxpPmBusRead  = Axp808DxePmBusRead,
.AxpPmBusWrite = Axp808DxePmBusWrite
};

STATIC AXP_POWER_PROTOCOL Axp808PowerProtocol = {
 .AxpPowerId         = AXP_POWER_ID_AXP808,
 .Probe          = Axp808Probe,
 .SetSupplyStatus        = Axp808SetSupplyStatus,
 .SetSupplyStatusByName  = Axp808SetSupplyStatusByName,
 .ProbeSupplyStatus      = Axp808ProbeSupplyStatus,
 .ProbeSupplyStatusByName= Axp808ProbeSupplyStatusByName,
 
 .SetNextSystemMode      = Axp808SetNextSysMode,
 .ProbePreSystemMode     = Axp808ProbePreSysMode,
 .ProbeThisPowerOnCause  = Axp808ProbeThisPowerOnCause,
 
 .ProbePowerBusExistance = Axp808ProbePowerBusExistance,
 
 .ProbeBatteryVoltage    = Axp808ProbeBatteryVoltage,
 .ProbeBatteryRatio      = Axp808ProbeBatteryRatio,
 .ProbeBatteryExistance  = Axp808ProbeBatteryExistance,
 
 .ProbePowerKey          = Axp808ProbePowerKey,
 
 .SetPowerOff            = Axp808SetPowerOff,
 .SetPowerOnOffVoltage   = Axp808SetPowerOnoffVoltage,
 
 .SetChargerOnOff        = Axp808SetChargerOnOff,
 .SetVbusCurrentLimit    = Axp808SetVbusCurrentLimit,
 .SetVbusVoltageLimit    = Axp808SetVbusVoltagelimit,
 .SetChargeCurrent       = Axp808SetChargeCurrent,
 .ProbeChargeCurrent     = Axp808ProbeChargeCurrent,
 
 .ProbeIntPendding       = Axp808ProbeIntPending,
 .ProbeIntEnable         = Axp808ProbeIntEnable,
 .SetIntEnable           = Axp808SetIntEnable,
 .SetIntDisable          = Axp808SetIntDisable
};

EFI_STATUS
Axp808Initialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;
  
  Status = gBS->LocateProtocol(&gEfiSmbusHcProtocolGuid, NULL, (VOID **)&Smbus);
  ASSERT_EFI_ERROR(Status);
  
  Status = gBS->InstallMultipleProtocolInterfaces(&ImageHandle, &gAxpPowerProtocolGuid, &Axp808PowerProtocol, NULL);
  return Status;
}
