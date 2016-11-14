/** @file
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

#include <PiDxe.h>

#include <Library/UefiRuntimeLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/EfiResetSystemLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/CacheMaintenanceLib.h>

#include <Guid/EventGroup.h>

#include <Include/Sun50iW1P1/platform.h>

/* Round off to 4KB pages */
#define ROUND_TO_PAGE(x) (x & 0xfffff000)

STATIC UINT32    gTimerBaseAddress   = SUNXI_TIMER_BASE;
//STATIC UINT32    gRtcBaseAddress     = SUNXI_RTC_BASE;

EFI_EVENT VirtualAddressChangeEvent = NULL;


#define SUNXI_WDOG_CTL    (gTimerBaseAddress+0xB0)
#define SUNXI_WDOG_CFG    (gTimerBaseAddress+0xB4)
#define SUNXI_WDOG_MODE   (gTimerBaseAddress+0xB8)

#define SUNXI_RUN_SHUTDOWN_ADDR  (gRtcBaseAddress + 0x1f0)


/**
  Virtual address change notification call back. It converts global pointer
  to virtual address.

  @param  Event         Event whose notification function is being invoked.
  @param  Context       Pointer to the notification function's context, which is
                        always zero in current implementation.
**/
VOID
EFIAPI
VirtualAddressChangeCallBack (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  gRT->ConvertPointer(0, (VOID**)&gTimerBaseAddress);
//  gRT->ConvertPointer(0, (VOID**)&gRtcBaseAddress);
  gRT->ConvertPointer(0, (VOID**)&gRT);
}

/**
  Resets the entire platform.

  @param  ResetType             The type of reset to perform.
  @param  ResetStatus           The status code for the reset.
  @param  DataSize              The size, in bytes, of WatchdogData.
  @param  ResetData             For a ResetType of EfiResetCold, EfiResetWarm, or
                                EfiResetShutdown the data buffer starts with a Null-terminated
                                Unicode string, optionally followed by additional binary data.

**/
EFI_STATUS
EFIAPI
LibResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  )
{

  switch (ResetType) {
  case EfiResetShutdown:       // write a magic to RTC power domain and reboot to shutdown.
    // MmioWrite32(SUNXI_RUN_SHUTDOWN_ADDR,0x3<<16|0x55<<8);
    // MmioOr32(SUNXI_RUN_SHUTDOWN_ADDR,0x1<<31);
    // MmioOr32(SUNXI_RUN_SHUTDOWN_ADDR,0x0<<31);
     
  case EfiResetWarm:
  case EfiResetCold:
    // Perform cold reset of the system.
    MmioWrite32 (SUNXI_WDOG_CFG, 0x1); //reset the whole system
    MmioWrite32 (SUNXI_WDOG_MODE, 0x1);//enable the watchdog and reset the system in 0.5s.
    MmioWrite32 (SUNXI_WDOG_CTL, 0x1);//restart the watchdog
    while(1);
    break;
  default:

    break;
  }

  // If the reset didn't work, return an error.
  ASSERT (FALSE);
  return EFI_DEVICE_ERROR;
}
  


/**
  Initialize any infrastructure required for LibResetSystem () to function.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
LibInitializeResetSystem (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  TimerMemoryDescriptor;

  /*
  * Get the GCD Memory Descriptor specified by WdtBaseAddress page boundary
  */
  Status = gDS->GetMemorySpaceDescriptor (ROUND_TO_PAGE(gTimerBaseAddress),
                                          &TimerMemoryDescriptor);
  ASSERT_EFI_ERROR (Status);

  /*
  * Mark the 4KB region as EFI_RUNTIME_MEMORY so the OS
  * will allocate a virtual address range.
  */
  Status = gDS->SetMemorySpaceAttributes (
                                          ROUND_TO_PAGE(gTimerBaseAddress),
                                          EFI_PAGE_SIZE,
                                          TimerMemoryDescriptor.Attributes|EFI_MEMORY_RUNTIME);
  ASSERT_EFI_ERROR (Status);

#if 0
  Status = gDS->GetMemorySpaceDescriptor (ROUND_TO_PAGE(gRtcBaseAddress),
                                          &TimerMemoryDescriptor);
  ASSERT_EFI_ERROR (Status);

  Status = gDS->SetMemorySpaceAttributes (
                                          ROUND_TO_PAGE(gRtcBaseAddress),
                                          EFI_PAGE_SIZE,
                                          TimerMemoryDescriptor.Attributes|EFI_MEMORY_RUNTIME);
  ASSERT_EFI_ERROR (Status);
#endif 

  Status = gBS->CreateEventEx (
                               EVT_NOTIFY_SIGNAL,
                               TPL_NOTIFY,
                               VirtualAddressChangeCallBack,
                               NULL,
                               &gEfiEventVirtualAddressChangeGuid,
                               &VirtualAddressChangeEvent
                               );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

