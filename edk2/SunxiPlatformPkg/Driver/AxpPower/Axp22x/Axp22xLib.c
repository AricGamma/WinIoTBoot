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

#include <Library/DebugLib.h>
#include <Library/SunxiSmBusLib.h>
#include <Protocol/AxpPower.h>
#include <Axp22x.h>


STATIC EFI_STATUS Axp22xPmBusRead(UINT8 DeviceRegister, UINT8 *Buffer)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1; //device address + data
  UINT8                    DeviceBuffer[2];

  DeviceBuffer[0] = DeviceRegister;
  Status = SmbusBlockRead(DeviceBuffer,DeviceBufferLength);
  *Buffer = DeviceBuffer[1];
  return Status;
}

STATIC EFI_STATUS Axp22xPmBusWrite(UINT8 DeviceRegister, UINT8 data)
{
  EFI_STATUS               Status;
  UINTN                    DeviceBufferLength = 1 + 1; //device address + data
  UINT8                    DeviceBuffer[2];

  DeviceBuffer[0] = DeviceRegister;
  DeviceBuffer[1] = data;

  Status = SmbusBlockWrite(DeviceBuffer,DeviceBufferLength);
  return Status;
}

AXP_PM_BUS_OPS Axp22xPmBusOps = {
.AxpPmBusRead  = Axp22xPmBusRead,
.AxpPmBusWrite = Axp22xPmBusWrite
};
#endif
