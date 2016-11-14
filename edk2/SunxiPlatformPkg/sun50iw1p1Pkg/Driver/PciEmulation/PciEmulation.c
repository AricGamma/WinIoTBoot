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

#include "PciEmulation.h"
#include <Interinc/sunxi_uefi.h>
#include "Sunxi_USB2Phy.h"

#include <Library/SunxiBootInfoLib.h>

#define HOST_CONTROLLER_OPERATION_REG_SIZE  0x44
#define USBDRD_CONTROLLER_OPERATION_REG_SIZE 0x400

typedef struct {
  ACPI_HID_DEVICE_PATH      AcpiDevicePath;
  PCI_DEVICE_PATH           PciDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  EndDevicePath;
} EFI_PCI_IO_DEVICE_PATH;

typedef struct {
  UINT32                  Signature;
  EFI_PCI_IO_DEVICE_PATH  DevicePath;
  EFI_PCI_IO_PROTOCOL     PciIoProtocol;
  PCI_TYPE00              *ConfigSpace;
  PCI_ROOT_BRIDGE         RootBridge;
  UINTN                   Segment;
} EFI_PCI_IO_PRIVATE_DATA;

#define EFI_PCI_IO_PRIVATE_DATA_SIGNATURE     SIGNATURE_32('p', 'c', 'i', 'o')
#define EFI_PCI_IO_PRIVATE_DATA_FROM_THIS(a)  CR(a, EFI_PCI_IO_PRIVATE_DATA, PciIoProtocol, EFI_PCI_IO_PRIVATE_DATA_SIGNATURE)

EFI_PCI_IO_DEVICE_PATH PciIoDevicePathUSBHost[2] = {
{
  {
    { ACPI_DEVICE_PATH, ACPI_DP, sizeof (ACPI_HID_DEVICE_PATH), 0},
    EISA_PNP_ID(0x0A03),  // HID
    0                     // UID
  },
  {
    { HARDWARE_DEVICE_PATH, HW_PCI_DP, sizeof (PCI_DEVICE_PATH), 0},
    0,
    0
  },
  { END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, sizeof (EFI_DEVICE_PATH_PROTOCOL), 0}
},
  
{
  {
    { ACPI_DEVICE_PATH, ACPI_DP, sizeof (ACPI_HID_DEVICE_PATH), 0},
    EISA_PNP_ID(0x0A03),  // HID
    1                     // UID
  },
  {
    { HARDWARE_DEVICE_PATH, HW_PCI_DP, sizeof (PCI_DEVICE_PATH), 0},
    0,
    1
  },
{ END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, sizeof (EFI_DEVICE_PATH_PROTOCOL), 0}
  
}

};

STATIC
VOID
ConfigureUSBHost (
  )
{
  SunxiUsb20Init();
}


EFI_STATUS
PciIoPollMem (
  IN EFI_PCI_IO_PROTOCOL           *This,
  IN  EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN  UINT8                        BarIndex,
  IN  UINT64                       Offset,
  IN  UINT64                       Mask,
  IN  UINT64                       Value,
  IN  UINT64                       Delay,
  OUT UINT64                       *Result
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
PciIoPollIo (
  IN EFI_PCI_IO_PROTOCOL           *This,
  IN  EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN  UINT8                        BarIndex,
  IN  UINT64                       Offset,
  IN  UINT64                       Mask,
  IN  UINT64                       Value,
  IN  UINT64                       Delay,
  OUT UINT64                       *Result
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
PciIoMemRead (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        BarIndex,
  IN     UINT64                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  EFI_PCI_IO_PRIVATE_DATA *Private = EFI_PCI_IO_PRIVATE_DATA_FROM_THIS(This);

  return PciRootBridgeIoMemRead (&Private->RootBridge.Io,
                                (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                                Private->ConfigSpace->Device.Bar[BarIndex] + Offset,
                                Count,
                                Buffer
                                );
}

EFI_STATUS
PciIoMemWrite (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        BarIndex,
  IN     UINT64                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  EFI_PCI_IO_PRIVATE_DATA *Private = EFI_PCI_IO_PRIVATE_DATA_FROM_THIS(This);

  return PciRootBridgeIoMemWrite (&Private->RootBridge.Io,
                                 (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                                 Private->ConfigSpace->Device.Bar[BarIndex] + Offset,
                                 Count,
                                 Buffer
                                 );
}

EFI_STATUS
PciIoIoRead (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        BarIndex,
  IN     UINT64                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
PciIoIoWrite (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        BarIndex,
  IN     UINT64                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
PciIoPciRead (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT32                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  EFI_PCI_IO_PRIVATE_DATA *Private = EFI_PCI_IO_PRIVATE_DATA_FROM_THIS(This);

  return PciRootBridgeIoMemRW ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH)Width,
                               Count,
                               TRUE,
                               (PTR)(UINTN)Buffer,
                               TRUE,
                               (PTR)(UINTN)(((UINT8 *)Private->ConfigSpace) + Offset)
                              );
}

EFI_STATUS
PciIoPciWrite (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT32                       Offset,
  IN     UINTN                        Count,
  IN OUT VOID                         *Buffer
  )
{
  EFI_PCI_IO_PRIVATE_DATA *Private = EFI_PCI_IO_PRIVATE_DATA_FROM_THIS(This);

  return PciRootBridgeIoMemRW ((EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH) Width,
                               Count,
                               TRUE,
                               (PTR)(UINTN)(((UINT8 *)Private->ConfigSpace) + Offset),
                               TRUE,
                               (PTR)(UINTN)Buffer
                               );
}

EFI_STATUS
PciIoCopyMem (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     EFI_PCI_IO_PROTOCOL_WIDTH    Width,
  IN     UINT8                        DestBarIndex,
  IN     UINT64                       DestOffset,
  IN     UINT8                        SrcBarIndex,
  IN     UINT64                       SrcOffset,
  IN     UINTN                        Count
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
PciIoMap (
  IN EFI_PCI_IO_PROTOCOL                *This,
  IN     EFI_PCI_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                           *HostAddress,
  IN OUT UINTN                          *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS           *DeviceAddress,
  OUT    VOID                           **Mapping
  )
{
  DMA_MAP_OPERATION   DmaOperation;

  if (Operation == EfiPciIoOperationBusMasterRead) {
    DmaOperation = MapOperationBusMasterRead;
  } else if (Operation == EfiPciIoOperationBusMasterWrite) {
    DmaOperation = MapOperationBusMasterWrite;
  } else if (Operation == EfiPciIoOperationBusMasterCommonBuffer) {
    DmaOperation = MapOperationBusMasterCommonBuffer;
  } else {
    return EFI_INVALID_PARAMETER;
  }
  return DmaMap (DmaOperation, HostAddress, NumberOfBytes, DeviceAddress, Mapping);
}

EFI_STATUS
PciIoUnmap (
  IN EFI_PCI_IO_PROTOCOL           *This,
  IN  VOID                         *Mapping
  )
{
  return DmaUnmap (Mapping);
}

EFI_STATUS
PciIoAllocateBuffer (
  IN EFI_PCI_IO_PROTOCOL           *This,
  IN  EFI_ALLOCATE_TYPE            Type,
  IN  EFI_MEMORY_TYPE              MemoryType,
  IN  UINTN                        Pages,
  OUT VOID                         **HostAddress,
  IN  UINT64                       Attributes
  )
{
  if (Attributes & EFI_PCI_ATTRIBUTE_INVALID_FOR_ALLOCATE_BUFFER) {
    // Check this
    return EFI_UNSUPPORTED;
  }

  return DmaAllocateBuffer (MemoryType, Pages, HostAddress);
}


EFI_STATUS
PciIoFreeBuffer (
  IN EFI_PCI_IO_PROTOCOL           *This,
  IN  UINTN                        Pages,
  IN  VOID                         *HostAddress
  )
{
  return DmaFreeBuffer (Pages, HostAddress);
}


EFI_STATUS
PciIoFlush (
  IN EFI_PCI_IO_PROTOCOL  *This
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
PciIoGetLocation (
  IN EFI_PCI_IO_PROTOCOL          *This,
  OUT UINTN                       *SegmentNumber,
  OUT UINTN                       *BusNumber,
  OUT UINTN                       *DeviceNumber,
  OUT UINTN                       *FunctionNumber
  )
{
  EFI_PCI_IO_PRIVATE_DATA *Private = EFI_PCI_IO_PRIVATE_DATA_FROM_THIS(This);

  if (SegmentNumber != NULL) {
    *SegmentNumber = Private->Segment;
  }

  if (BusNumber != NULL) {
    *BusNumber = 0xff;
  }

  if (DeviceNumber != NULL) {
    *DeviceNumber = 0;
  }

  if (FunctionNumber != NULL) {
    *FunctionNumber = 0;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PciIoAttributes (
  IN EFI_PCI_IO_PROTOCOL                       *This,
  IN  EFI_PCI_IO_PROTOCOL_ATTRIBUTE_OPERATION  Operation,
  IN  UINT64                                   Attributes,
  OUT UINT64                                   *Result OPTIONAL
  )
{
  switch (Operation) {
  case EfiPciIoAttributeOperationGet:
  case EfiPciIoAttributeOperationSupported:
    if (Result == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    // We are not a real PCI device so just say things we kind of do
    *Result = EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER | EFI_PCI_DEVICE_ENABLE;
    break;

  case EfiPciIoAttributeOperationSet:
  case EfiPciIoAttributeOperationEnable:
  case EfiPciIoAttributeOperationDisable:
    // Since we are not a real PCI device no enable/set or disable operations exist.
  //  DEBUG((EFI_D_INFO, "--%a(Set:1, Enable:2, Disable:3)(%d):%d\n", __FUNCTION__, Operation,__LINE__));
    return EFI_SUCCESS;

  default:
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  };
  //  DEBUG((EFI_D_INFO, "--%a(Get:0, Supported:4)(%d):%d\n", __FUNCTION__, Operation,__LINE__));
  return EFI_SUCCESS;
}

EFI_STATUS
PciIoGetBarAttributes (
  IN EFI_PCI_IO_PROTOCOL             *This,
  IN  UINT8                          BarIndex,
  OUT UINT64                         *Supports, OPTIONAL
  OUT VOID                           **Resources OPTIONAL
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
PciIoSetBarAttributes (
  IN EFI_PCI_IO_PROTOCOL              *This,
  IN     UINT64                       Attributes,
  IN     UINT8                        BarIndex,
  IN OUT UINT64                       *Offset,
  IN OUT UINT64                       *Length
  )
{
  ASSERT (FALSE);
  return EFI_UNSUPPORTED;
}

EFI_PCI_IO_PROTOCOL PciIoTemplate =
{
  PciIoPollMem,
  PciIoPollIo,
  PciIoMemRead,
  PciIoMemWrite,
  PciIoIoRead,
  PciIoIoWrite,
  PciIoPciRead,
  PciIoPciWrite,
  PciIoCopyMem,
  PciIoMap,
  PciIoUnmap,
  PciIoAllocateBuffer,
  PciIoFreeBuffer,
  PciIoFlush,
  PciIoGetLocation,
  PciIoAttributes,
  PciIoGetBarAttributes,
  PciIoSetBarAttributes,
  0,
  0
};

EFI_STATUS
EFIAPI
PciEmulationEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS              Status;
  EFI_HANDLE              HandleUSB2;
  EFI_PCI_IO_PRIVATE_DATA *PrivateUSB2;

  UINT8                   CapabilityLength;
  UINT8                   PhysicalPorts;
  UINTN                   Count,Loop;
  UINTN           UsbBaseAddrArray[] = {USB_EHCI0_HCCAPBASE,USB_EHCI1_HCCAPBASE};
  
  if(SunxiGetBootWorkMode() !=WORK_MODE_BOOT){   
    return EFI_SUCCESS;   
  }

  //Configure USB host 
  ConfigureUSBHost();
  for(Loop=0;Loop<(sizeof(UsbBaseAddrArray)/sizeof(UINTN));Loop++){
    // Create a private structure for USB 2.0
    PrivateUSB2 = AllocatePool(sizeof(EFI_PCI_IO_PRIVATE_DATA));
    if (PrivateUSB2 == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      return Status;
    }
    PrivateUSB2->Signature              = EFI_PCI_IO_PRIVATE_DATA_SIGNATURE;  // Fill in signature
    PrivateUSB2->RootBridge.Signature   = PCI_ROOT_BRIDGE_SIGNATURE;          // Fake Root Bridge structure needs a signature too
    PrivateUSB2->RootBridge.MemoryStart = UsbBaseAddrArray[Loop];             // Get the USB capability register base
    PrivateUSB2->Segment                = 0;                                  // Default to segment zero

    // USB 2.0
    // Find out the capability register length and number of physical ports.
    CapabilityLength = MmioRead8(PrivateUSB2->RootBridge.MemoryStart);
    PhysicalPorts    = (MmioRead32 (PrivateUSB2->RootBridge.MemoryStart + 0x4)) & 0x0000000F;
    // Calculate the total size of the USB registers.
    PrivateUSB2->RootBridge.MemorySize = CapabilityLength + (HOST_CONTROLLER_OPERATION_REG_SIZE + ((4 * PhysicalPorts) - 1));

    // Enable Port Power bit in Port status and control registers in EHCI register space.
    // Port Power Control (PPC) bit in the HCSPARAMS register is already set which indicates
    // host controller implementation includes port power control.
    for (Count = 0; Count < PhysicalPorts; Count++) {
      MmioOr32 ((PrivateUSB2->RootBridge.MemoryStart + CapabilityLength + HOST_CONTROLLER_OPERATION_REG_SIZE + 4*Count), 0x00001000);
    }
          
    // USB 2.0
    // Create fake PCI config space.
    PrivateUSB2->ConfigSpace = AllocateZeroPool(sizeof(PCI_TYPE00));
    if (PrivateUSB2->ConfigSpace == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      FreePool(PrivateUSB2);
      return Status;
    }

    // Configure PCI config space
    PrivateUSB2->ConfigSpace->Hdr.VendorId = 0xEF3A;
    PrivateUSB2->ConfigSpace->Hdr.DeviceId = 0x1639;
    PrivateUSB2->ConfigSpace->Hdr.ClassCode[0] = 0x20;
    PrivateUSB2->ConfigSpace->Hdr.ClassCode[1] = 0x03;
    PrivateUSB2->ConfigSpace->Hdr.ClassCode[2] = 0x0C;
    PrivateUSB2->ConfigSpace->Device.Bar[0] = PrivateUSB2->RootBridge.MemoryStart;

    HandleUSB2 = NULL;
      
    // USB 2.0
    // Unique device path.
    CopyMem(&PrivateUSB2->DevicePath, &PciIoDevicePathUSBHost[Loop], sizeof(EFI_PCI_IO_DEVICE_PATH));
    //PrivateUSB2->DevicePath.AcpiDevicePath.UID = 0;
    // Copy protocol structure
    CopyMem(&PrivateUSB2->PciIoProtocol, &PciIoTemplate, sizeof(PciIoTemplate));

    Status = gBS->InstallMultipleProtocolInterfaces(&HandleUSB2,
                                                   &gEfiPciIoProtocolGuid,       &PrivateUSB2->PciIoProtocol,
                                                   &gEfiDevicePathProtocolGuid,  &PrivateUSB2->DevicePath,
                                                   NULL);
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "PciEmulationEntryPoint InstallMultipleProtocolInterfaces() failed.\n"));
    }

  }
  
  return Status;
}

