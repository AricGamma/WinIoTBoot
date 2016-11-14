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
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>

#include <Library/SysConfigLib.h>

#include <Protocol/SmbusHc.h>
#include <ccmu.h>
#include <rsb.h>


STATIC
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
  DeviceBufferLength = Length - 2;
  Status = SunxiRsbRead(DeviceAddr,DeviceRegister, &Buffer[2], DeviceBufferLength);
  
  return Status;
}

STATIC
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
  DeviceBufferLength = Length - 2;
  Status = SunxiRsbWrite(DeviceAddr, DeviceRegister, &Buffer[2], DeviceBufferLength);
  
  return Status;
}


//
// Public Functions.
//
EFI_STATUS
EFIAPI
SmbusExecute (
  IN CONST EFI_SMBUS_HC_PROTOCOL    *This,
  IN CONST EFI_SMBUS_DEVICE_ADDRESS SlaveAddress,
  IN CONST EFI_SMBUS_DEVICE_COMMAND Command,
  IN CONST EFI_SMBUS_OPERATION      Operation,
  IN CONST BOOLEAN                  PecCheck,
  IN OUT   UINTN                    *Length,
  IN OUT   VOID                     *Buffer
  )
{
  UINT8      *ByteBuffer  = Buffer;
  EFI_STATUS Status       = EFI_SUCCESS;
  //UINT8      SlaveAddr    = (UINT8)(SlaveAddress.SmbusDeviceAddress);

  if (PecCheck) {
    return EFI_UNSUPPORTED;
  }

  if ((Operation != EfiSmbusWriteBlock) && (Operation != EfiSmbusReadBlock)) {
    return EFI_UNSUPPORTED;
  }

  if (Operation == EfiSmbusReadBlock) {
    Status = SmbusBlockRead(ByteBuffer, *Length);
  } else if (Operation == EfiSmbusWriteBlock) {
    Status = SmbusBlockWrite(ByteBuffer, *Length);
  }

  return Status;
}

EFI_STATUS
EFIAPI
SmbusArpDevice (
  IN CONST EFI_SMBUS_HC_PROTOCOL    *This,
  IN       BOOLEAN                  ArpAll,
  IN       EFI_SMBUS_UDID           *SmbusUdid OPTIONAL,
  IN OUT   EFI_SMBUS_DEVICE_ADDRESS *SlaveAddress OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
SmbusGetArpMap (
  IN CONST EFI_SMBUS_HC_PROTOCOL    *This,
  IN OUT   UINTN                    *Length,
  IN OUT   EFI_SMBUS_DEVICE_MAP     **SmbusDeviceMap
  )
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
SmbusNotify (
  IN CONST  EFI_SMBUS_HC_PROTOCOL     *This,
  IN CONST  EFI_SMBUS_DEVICE_ADDRESS  SlaveAddress,
  IN CONST  UINTN                     Data,
  IN CONST  EFI_SMBUS_NOTIFY_FUNCTION NotifyFunction
  )
{
  return EFI_UNSUPPORTED;
}

EFI_SMBUS_HC_PROTOCOL SmbusProtocol =
{
  SmbusExecute,
  SmbusArpDevice,
  SmbusGetArpMap,
  SmbusNotify
};

#if 1
EFI_STATUS
InitializeTwiBus (
  VOID
    )
{
  EFI_STATUS Status=EFI_SUCCESS;
  INT32 Ret;
  INT32 Used;
  UINT32 i;//skip the p2wi0
  CHAR8 TwiIoPara[16]={0};

  //select 24M OSC as APB1 clock source, and don't divide the clock.
  // that means deivces will get a 24M clock from APB1.
  //MmioWrite32(CCM_APB1_RATIO_CTRL, 0x0<<24);
    
  for(i=0;i<3;i++){
    AsciiSPrint(TwiIoPara, sizeof(TwiIoPara), "twi%d",i);
    
    Ret = script_parser_fetch(TwiIoPara, "twi_used", &Used, sizeof(UINT32)/4);
    if (!Ret) {
      if (Used) {
        DEBUG ((EFI_D_INFO, "twi%d used\n",i));
        Ret =gpio_request_ex(TwiIoPara, NULL);
        if(!Ret){
          DEBUG ((EFI_D_ERROR,"request twi%d gpio failed!\n",i));
          Status = EFI_DEVICE_ERROR;
        }
        MmioOr32(CCMU_BUS_CLK_GATING_REG3,1<<i);//open APB2 gating for this twi controller.
        MmioOr32(CCMU_BUS_SOFT_RST_REG4,1<<i);//de-assert this twi controller.
      }
    }
  }
  
  return Status;
}
#endif

EFI_STATUS
InitializeSmbus (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
  EFI_HANDLE      Handle = NULL;
  EFI_STATUS      Status;
 //we have already config the rsb in prepi phase.
 //so we don't need to reinitialize the rsb hardware.
  Status = InitializeTwiBus();
  ASSERT_EFI_ERROR(Status);

  // Install the SMBUS interface
  Status = gBS->InstallMultipleProtocolInterfaces(&Handle, &gEfiSmbusHcProtocolGuid, &SmbusProtocol, NULL);
  ASSERT_EFI_ERROR(Status);
  
  return Status;
}

