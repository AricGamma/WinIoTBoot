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
#include <Library/IoLib.h>

#include <platform.h>
#include <rsb.h>


EFI_STATUS
SmbusBlockRead (
  OUT UINT8       *Buffer,
  IN  UINTN       Length
  )
{
  UINT8 DeviceAddr;
  UINT8 DeviceRegister;
  UINTN DeviceBufferLength;
  EFI_STATUS Status = EFI_SUCCESS;

  DeviceAddr = Buffer[0];
  DeviceRegister = Buffer[1];
  DeviceBufferLength = Length-2;
  Status = SunxiRsbRead(DeviceAddr,DeviceRegister, &Buffer[2], DeviceBufferLength);
  
  return Status;
}


EFI_STATUS
SmbusBlockWrite (
  IN UINT8       *Buffer,
  IN UINTN       Length
  )
{
  UINT8 DeviceAddr; 
  UINT8 DeviceRegister;
  UINTN DeviceBufferLength;
  EFI_STATUS Status = EFI_SUCCESS;

  DeviceAddr = Buffer[0];
  DeviceRegister = Buffer[1];
  DeviceBufferLength = Length-2;
  Status = SunxiRsbWrite(DeviceAddr,DeviceRegister, &Buffer[2], DeviceBufferLength);
  
  return Status;
}

EFI_STATUS
SmBusInit (
    VOID
    )
{
  EFI_STATUS      Status;
  Status = 0;
  // Configure rsb controller.
  Status = SunxiRsbInit();

  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Initialize Rsb fails.\n"));
    return Status;
  }

  Status = SunxiRsbConfig(RSB_SADDR_AXP81X,RSB_RTSADDR_AXP81X);
  if(EFI_ERROR(Status))
  {
    DEBUG ((EFI_D_ERROR, "Config Rsb For Axp81X fails.\n"));
    return EFI_DEVICE_ERROR;
  }

  return Status;
}

