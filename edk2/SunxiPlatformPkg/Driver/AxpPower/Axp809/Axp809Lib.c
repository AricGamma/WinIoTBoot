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

#include <Library/DebugLib.h>
#include <Library/SunxiSmBusLib.h>
#include <Protocol/AxpPower.h>
#include <Axp809.h>

STATIC EFI_STATUS Axp809PmBusRead(UINT8 DeviceRegister, UINT8 *Buffer)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1 + 1; //deviceid + device address + data
  UINT8                    DeviceBuffer[3];

  DeviceBuffer[0] = RSB_RTSADDR_AXP809;
  DeviceBuffer[1] = DeviceRegister;
  Status = SmbusBlockRead(DeviceBuffer,DeviceBufferLength);
  *Buffer = DeviceBuffer[2];
  return Status;
}

STATIC EFI_STATUS Axp809PmBusWrite(UINT8 DeviceRegister, UINT8 data)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1 + 1; //deviceid + device address + data
  UINT8                    DeviceBuffer[3];
  
  DeviceBuffer[0] = RSB_RTSADDR_AXP809; 
  DeviceBuffer[1] = DeviceRegister;
  DeviceBuffer[2] = data;

  Status = SmbusBlockWrite(DeviceBuffer,DeviceBufferLength);
  return Status;
}

AXP_PM_BUS_OPS Axp809PmBusOps = {
.AxpPmBusRead  = Axp809PmBusRead,
.AxpPmBusWrite = Axp809PmBusWrite
};

