/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  wangwei <wangwei@allwinnertech.com>
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
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/DebugLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/LoadedImage.h>
#include <Library/TimerLib.h>
#include <Library/PcdLib.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiBootInfoLib.h>

extern EFI_GUID gEfiBlockIoProtocolGuid;

extern int sunxi_card_sprite_main(int workmode, char *name);
extern int SunxiSpriteInit(void);

EFI_RUNTIME_SERVICES              *RuntimeServices = NULL;
EFI_BOOT_SERVICES                 *BootServices = NULL;

//struct spare_boot_head_t* uboot_spare_head = NULL;


/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the image goes into a library that calls this
  function.


  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
CardBurnMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS           Status;
  EFI_LOADED_IMAGE_PROTOCOL*   LoadedImage;

  AsciiPrint("WorkMode = %x, StorageType = %x\n",SunxiGetBootWorkMode(),SunxiGetBurnStorage());
  
  RuntimeServices = SystemTable->RuntimeServices;
  BootServices    = SystemTable->BootServices;
  Status = BootServices->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
  if (EFI_ERROR (Status)) {
    return Status;
  } 
  if(SunxiSpriteInit())
  {
    AsciiPrint("SunxiSpriteInit fail\n");
    return EFI_DEVICE_ERROR;
  }
  
  if(0 == sunxi_card_sprite_main(0, NULL))
  {
    AsciiPrint("card sprint success\n");
  }
   
  return Status;
}

void SpriteDelayMS(UINT32 ms)
{
  MicroSecondDelay (ms*1000);
}

void SpriteDelayUS(UINT32 us)
{
  MicroSecondDelay (us);
}

int sunxi_board_restart(int next_mode)
{
  Print(L"reboot system\n");
  return 0;
}