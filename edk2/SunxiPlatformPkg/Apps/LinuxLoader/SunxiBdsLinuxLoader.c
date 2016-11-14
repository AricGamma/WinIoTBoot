/** @file
*
*  Copyright (c) 2011-2012, ARM Limited. All rights reserved.
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

#include <Library/BdsLib/BdsInternal.h>
#include <Library/BdsLib/BdsLinuxLoader.h>
#include <Library/TimerLib.h>
#include <Library/SerialPortLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "SunxiLinuxHead.h"



#define ALIGN32_BELOW(addr)   ALIGN_POINTER(addr - 32,32)


EFI_STATUS
ShutdownUefiBootServices (
  VOID
  )
{
  EFI_STATUS              Status;
  UINTN                   MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR   *MemoryMap;
  UINTN                   MapKey;
  UINTN                   DescriptorSize;
  UINT32                  DescriptorVersion;
  UINTN                   Pages;

  MemoryMap = NULL;
  MemoryMapSize = 0;
  Pages = 0;

  do {
    Status = gBS->GetMemoryMap (
                    &MemoryMapSize,
                    MemoryMap,
                    &MapKey,
                    &DescriptorSize,
                    &DescriptorVersion
                    );
    if (Status == EFI_BUFFER_TOO_SMALL) {

      Pages = EFI_SIZE_TO_PAGES (MemoryMapSize) + 1;
      MemoryMap = AllocatePages (Pages);

      //
      // Get System MemoryMap
      //
      Status = gBS->GetMemoryMap (
                      &MemoryMapSize,
                      MemoryMap,
                      &MapKey,
                      &DescriptorSize,
                      &DescriptorVersion
                      );
    }

    // Don't do anything between the GetMemoryMap() and ExitBootServices()
    if (!EFI_ERROR(Status)) {
      Status = gBS->ExitBootServices (gImageHandle, MapKey);
      if (EFI_ERROR(Status)) {
        FreePages (MemoryMap, Pages);
        MemoryMap = NULL;
        MemoryMapSize = 0;
      }
    }
  } while (EFI_ERROR(Status));

  return Status;
}


STATIC
EFI_STATUS
PreparePlatformHardware (
  VOID
  )
{
  //Note: Interrupts will be disabled by the GIC driver when ExitBootServices() will be called.

  // Clean, invalidate, disable data cache
  ArmCleanInvalidateDataCache();
  ArmDisableDataCache();

  // Invalidate and disable the Instruction cache
  ArmDisableInstructionCache ();
  ArmInvalidateInstructionCache ();

  // Turn off MMU
  //ArmDisableMmu();

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
StartLinux (
  IN  EFI_PHYSICAL_ADDRESS  LinuxImage,
  IN  UINTN                 LinuxImageSize,
  IN  EFI_PHYSICAL_ADDRESS  KernelParamsAddress,
  IN  UINTN                 KernelParamsSize,
  IN  UINT32                MachineType
  )
{
  EFI_STATUS            Status;
  LINUX_KERNEL          LinuxKernel;


  // Shut down UEFI boot services. ExitBootServices() will notify every driver that created an event on
  // ExitBootServices event. Example the Interrupt DXE driver will disable the interrupts on this event.
  Status = ShutdownUefiBootServices ();
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR,"ERROR: Can not shutdown UEFI boot services. Status=0x%X\n", Status));
    goto Exit;
  }
  LinuxKernel = (LINUX_KERNEL)(UINTN)LinuxImage;
  // Check if the Linux Image is a uImage
  if (*(UINT32*)LinuxKernel == LINUX_UIMAGE_SIGNATURE) {
    // Assume the Image Entry Point is just after the uImage header (64-byte size)
    LinuxKernel = (LINUX_KERNEL)((UINTN)LinuxKernel + 64);
    LinuxImageSize -= 64;
  }

  //TODO: Check there is no overlapping between kernel and Atag

  //
  // Switch off interrupts, caches, mmu, etc
  //
  Status = PreparePlatformHardware ();
  ASSERT_EFI_ERROR(Status);

  //
  // Start the Linux Kernel
  //

  // Outside BootServices, so can't use Print();
  SerialPortWrite((UINT8*)"\nStarting the kernel:\n\n", AsciiStrLen("\nStarting the kernel:\n\n"));

  // Jump to kernel with register set
  LinuxKernel ((UINTN)0, MachineType, (UINTN)KernelParamsAddress);

  // Kernel should never exit
  // After Life services are not provided
  ASSERT(FALSE);

Exit:
  // Only be here if we fail to start Linux
  Print (L"ERROR  : Can not start the kernel. Status=0x%X\n", Status);

  // Free Runtimee Memory (kernel and FDT)
  return Status;
}

/**
  Start a Linux kernel from a Device Path

  @param  LinuxKernel           Device Path to the Linux Kernel
  @param  Parameters            Linux kernel arguments
  @param  Fdt                   Device Path to the Flat Device Tree

  @retval EFI_SUCCESS           All drivers have been connected
  @retval EFI_NOT_FOUND         The Linux kernel Device Path has not been found
  @retval EFI_OUT_OF_RESOURCES  There is not enough resource memory to store the matching results.

**/
EFI_STATUS
SunxiBdsBootLinuxAtag (
  IN EFI_PHYSICAL_ADDRESS       LinuxImageBase,
  IN  CONST CHAR8*              CommandLineArguments
  )
{
  EFI_STATUS            Status;
  UINT32                LinuxImageSize;
  UINT32                InitrdImageSize;
  UINT32                AtagSize;
  
  EFI_PHYSICAL_ADDRESS  AtagBase;
  EFI_PHYSICAL_ADDRESS  LinuxImage;
  EFI_PHYSICAL_ADDRESS  InitrdImage;
  struct fastboot_boot_img_hdr *fb_hdr;

  fb_hdr = (struct fastboot_boot_img_hdr *)((void*)(UINTN)LinuxImageBase);

  InitrdImageSize = (UINT32)fb_hdr->ramdisk_size;
  InitrdImage = (EFI_PHYSICAL_ADDRESS)fb_hdr->ramdisk_addr;


  LinuxImageSize = fb_hdr->kernel_size;
  LinuxImage = (EFI_PHYSICAL_ADDRESS)fb_hdr->kernel_addr;

  AtagSize = 0;
  AtagBase = 0;

  // By setting address=0 we leave the memory allocation to the function
  Status = PrepareAtagList (CommandLineArguments, InitrdImage, InitrdImageSize, &AtagBase, &AtagSize);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Can not prepare ATAG list. Status=0x%X\n", Status);
  }

  return StartLinux (LinuxImage, LinuxImageSize, AtagBase, AtagSize, PcdGet32(PcdArmMachineType));

}


