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
#include <Axp809.h>

STATIC EFI_SMBUS_HC_PROTOCOL *Smbus;

STATIC EFI_STATUS Axp809PmBusRead(UINT8 DeviceRegister, UINT8 *Buffer)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1 + 1; //deviceID + device address + data
  UINT8                    DeviceBuffer[3];
  EFI_SMBUS_DEVICE_ADDRESS AxpSlaveAddress; 

  AxpSlaveAddress.SmbusDeviceAddress = AXP809_ADDR>>1;
  DeviceBuffer[0] = AXP809_ADDR;
  DeviceBuffer[1] = DeviceRegister;
  Status = Smbus->Execute(Smbus,AxpSlaveAddress,0,\
     EfiSmbusReadBlock, FALSE, &DeviceBufferLength, DeviceBuffer);
  Buffer[0] = DeviceBuffer[2];
  return Status;
}

STATIC EFI_STATUS Axp809PmBusWrite(UINT8 DeviceRegister, UINT8 data)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1 + 1; //deviceID + device address + data
  UINT8                    DeviceBuffer[3];
  EFI_SMBUS_DEVICE_ADDRESS AxpSlaveAddress; 

  AxpSlaveAddress.SmbusDeviceAddress = AXP809_ADDR>>1;
  DeviceBuffer[0] = AXP809_ADDR;
  DeviceBuffer[1] = DeviceRegister;
  DeviceBuffer[2] = data;

  Status = Smbus->Execute(Smbus, AxpSlaveAddress, 0,\
     EfiSmbusWriteBlock, FALSE, &DeviceBufferLength, DeviceBuffer);
  return Status;
}

AXP_PM_BUS_OPS Axp809PmBusOps = {
.AxpPmBusRead  = Axp809PmBusRead,
.AxpPmBusWrite = Axp809PmBusWrite
};

STATIC AXP_POWER_PROTOCOL Axp809PowerProtocol = {
 .AxpPowerId         = AXP_POWER_ID_AXP809,
 .Probe          = Axp809Probe,
 .SetSupplyStatus        = Axp809SetSupplyStatus,
 .SetSupplyStatusByName  = Axp809SetSupplyStatusByName,
 .ProbeSupplyStatus      = Axp809ProbeSupplyStatus,
 .ProbeSupplyStatusByName= Axp809ProbeSupplyStatusByName,
 
 .SetNextSystemMode      = Axp809SetNextSysMode,
 .ProbePreSystemMode     = Axp809ProbePreSysMode,
 .ProbeThisPowerOnCause  = Axp809ProbeThisPowerOnCause,
 
 .ProbePowerBusExistance = Axp809ProbePowerBusExistance,
 
 .ProbeBatteryVoltage    = Axp809ProbeBatteryVoltage,
 .ProbeBatteryRatio      = Axp809ProbeBatteryRatio,
 .ProbeBatteryExistance  = Axp809ProbeBatteryExistance,
 
 .ProbePowerKey          = Axp809ProbePowerKey,
 
 .SetPowerOff            = Axp809SetPowerOff,
 .SetPowerOnOffVoltage   = Axp809SetPowerOnoffVoltage,
 
 .SetChargerOnOff        = Axp809SetChargerOnOff,
 .SetVbusCurrentLimit    = Axp809SetVbusCurrentLimit,
 .ProbeVbusCurrentLimit  = Axp809ProbeVbusCurrentLimit,
 .SetVbusVoltageLimit    = Axp809SetVbusVoltagelimit,
 .SetChargeCurrent       = Axp809SetChargeCurrent,
 .ProbeChargeCurrent     = Axp809ProbeChargeCurrent,
 
 .ProbeIntPendding       = Axp809ProbeIntPending,
 .ProbeIntEnable         = Axp809ProbeIntEnable,
 .SetIntEnable           = Axp809SetIntEnable,
 .SetIntDisable          = Axp809SetIntDisable,
 .SetCoulombmeterOnOff   = Axp809SetCoulombmeterOnOff
};

EFI_STATUS
Axp809Initialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;
  
  Status = gBS->LocateProtocol(&gEfiSmbusHcProtocolGuid, NULL, (VOID **)&Smbus);
  ASSERT_EFI_ERROR(Status);
  
  Status = gBS->InstallMultipleProtocolInterfaces(&ImageHandle, &gAxpPowerProtocolGuid, &Axp809PowerProtocol, NULL);
  return Status;
}
