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

#include <PiDxe.h>

#include <Protocol/AcpiTable.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/GraphicsOutput.h>

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/SunxiFileLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/GraphicsLib.h>

#include <IndustryStandard/Bmp.h>
#include <IndustryStandard/Acpi.h>
#include <Interinc/sunxi_uefi.h>

#include <Guid/FileInfo.h>
#include <Guid/SunxiBootInfoHob.h>

#define EFI_ACPI_TABLE_FILE_DIR     L"\\ACPI\\"

extern UINT32 SunxiLibGetBootInfo(SUNXI_BOOTINFO_HOB  **Hob);

/**
  Locate the first instance of a protocol.  If the protocol requested is an
  FV protocol, then it will return the first FV that contains the ACPI table
  storage file.

  @param  Instance      Return pointer to the first instance of the protocol

  @return EFI_SUCCESS           The function completed successfully.
  @return EFI_NOT_FOUND         The protocol could not be located.
  @return EFI_OUT_OF_RESOURCES  There are not enough resources to find the protocol.

**/
EFI_STATUS
LocateFvInstanceWithTables (
  OUT EFI_FIRMWARE_VOLUME2_PROTOCOL **Instance
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         NumberOfHandles;
  EFI_FV_FILETYPE               FileType;
  UINT32                        FvStatus;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  UINTN                         Size;
  UINTN                         Index;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *FvInstance;

  FvStatus = 0;

  //
  // Locate protocol.
  //
  Status = gBS->LocateHandleBuffer (
                   ByProtocol,
                   &gEfiFirmwareVolume2ProtocolGuid,
                   NULL,
                   &NumberOfHandles,
                   &HandleBuffer
                   );
  if (EFI_ERROR (Status)) {
    //
    // Defined errors at this time are not found and out of resources.
    //
    return Status;
  }



  //
  // Looking for FV with ACPI storage file
  //

  for (Index = 0; Index < NumberOfHandles; Index++) {
    //
    // Get the protocol on this handle
    // This should not fail because of LocateHandleBuffer
    //
    Status = gBS->HandleProtocol (
                     HandleBuffer[Index],
                     &gEfiFirmwareVolume2ProtocolGuid,
                     (VOID**) &FvInstance
                     );
    ASSERT_EFI_ERROR (Status);

    //
    // See if it has the ACPI storage file
    //
    Status = FvInstance->ReadFile (
                           FvInstance,
                           (EFI_GUID*)PcdGetPtr (PcdAcpiTableStorageFile),
                           NULL,
                           &Size,
                           &FileType,
                           &Attributes,
                           &FvStatus
                           );

    //
    // If we found it, then we are done
    //
    if (Status == EFI_SUCCESS) {
      *Instance = FvInstance;
      break;
    }
  }

  //
  // Our exit status is determined by the success of the previous operations
  // If the protocol was found, Instance already points to it.
  //

  //
  // Free any allocated buffers
  //
  gBS->FreePool (HandleBuffer);

  return Status;
}


/**
  This function calculates and updates an UINT8 checksum.

  @param  Buffer          Pointer to buffer to checksum
  @param  Size            Number of bytes to checksum

**/
VOID
AcpiPlatformChecksum (
  IN UINT8      *Buffer,
  IN UINTN      Size
  )
{
  UINTN ChecksumOffset;

  ChecksumOffset = OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum);

  //
  // Set checksum to 0 first
  //
  Buffer[ChecksumOffset] = 0;

  //
  // Update checksum value
  //
  Buffer[ChecksumOffset] = CalculateCheckSum8(Buffer, Size);
}


/**

  Routine Description:

    Update the processors information in the APIC table

  Arguments:

    Table   - The table to be set
    Version - Version to publish

  Returns:

    None

**/
VOID
ApicTableUpdate (
  IN OUT   EFI_ACPI_DESCRIPTION_HEADER  *TableHeader,
  IN OUT   EFI_ACPI_TABLE_VERSION       *Version
  )
{
  #if 1
  EFI_ACPI_5_0_GIC_STRUCTURE *pGicCpuInterFace;
  UINT32 Status;
  SUNXI_BOOTINFO_HOB *Hob = NULL;

  //struct spare_boot_head_t* spare_head;
  //UINT64  DramSize = 0;
  //UINT64  FbBase = 0;
  UINT64  MpParkBase = 0;
  UINT32  i;

  Status = SunxiLibGetBootInfo(&Hob);
  if(Status != EFI_SUCCESS){
    DEBUG((DEBUG_ERROR, "Get Boot Info Hob Failure\n"));
  }
  else
  {
    MpParkBase = Hob->ParkSharedBase;
  }

  //spare_head = get_spare_head(PcdGet32(PcdFdBaseAddress));
  //DramSize = (spare_head->boot_data.dram_para[4]&0xffff)*1024*1024;
  //FbBase    = PcdGet64(PcdSystemMemoryBase) + DramSize - PcdGet64(PcdFrameBufferSize);
  //MpParkBase = FbBase - PcdGet64(PcdMpParkSharedSize);

  pGicCpuInterFace = (EFI_ACPI_5_0_GIC_STRUCTURE*)&(((EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)(TableHeader))[1]);

  for(i=0;i<PcdGet32(PcdCoreCount);i++){

  if(pGicCpuInterFace[i].Type!=EFI_ACPI_5_0_GIC) continue;
    /*1k park mailbox per cpu.*/
    pGicCpuInterFace[i].ParkedAddress = MpParkBase + (i<<12);
  }
  #endif
}

/**

  Routine Description:

    update the acpi table data based on the platform hardware.

  Arguments:

    Table   - The table to be set
    Version - Version to publish

  Returns:

    None

**/
VOID
AcpiUpdateTable (
  IN OUT   EFI_ACPI_DESCRIPTION_HEADER  *TableHeader,
  IN OUT   EFI_ACPI_TABLE_VERSION       *Version
  )
{
  if (TableHeader != NULL && Version != NULL) {

  *Version = EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0;
  //
  // Here we use all 3.0 signature because all version use same signature if they supported
  //
  switch (TableHeader->Signature) {
    //
    // "APIC" Multiple APIC Description Table
    //
    case EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE:
      ApicTableUpdate (TableHeader, Version);
      break;
    //
    // "DSDT" Differentiated System Description Table
    //
    case EFI_ACPI_3_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
     // DsdtTableUpdate (TableHeader, Version);
      break;

    //
    // "FACP" Fixed ACPI Description Table (FADT)
    //
    case EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE:
      *Version = EFI_ACPI_TABLE_VERSION_NONE;
      if (TableHeader->Revision == EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
        *Version = EFI_ACPI_TABLE_VERSION_1_0B;

      } else if (TableHeader->Revision == EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
        *Version = EFI_ACPI_TABLE_VERSION_2_0;

      } else if (TableHeader->Revision == EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION) {
        *Version = EFI_ACPI_TABLE_VERSION_3_0;
      }
      break;
    //
    // "FACS" Firmware ACPI Control Structure
    //
    case EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE:
      break;
    //
    // "SSDT" Secondary System Description Table
    //
    case EFI_ACPI_3_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
    break;
    //
    // "HPET" IA-PC High Precision Event Timer Table
    //
    case EFI_ACPI_3_0_HIGH_PRECISION_EVENT_TIMER_TABLE_SIGNATURE:
      //
      // If HPET is disabled in setup, don't publish the table.
      //
      //  if (mGlobalNvsArea.Area->HpetEnable == 0) {
      //    *Version = EFI_ACPI_TABLE_VERSION_NONE;
      //   }
      //   ((EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER *) TableHeader)->BaseAddressLower32Bit.Address
      //    = PcdGet64 (PcdHpetBaseAddress);
      break;
    //
    // "SPCR" Serial Port Concole Redirection Table
    //
    case EFI_ACPI_3_0_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE:
      break;

    // Lakeport platform doesn't support the following table
    /*
      //
    // "ECDT" Embedded Controller Boot Resources Table
        //
    case EFI_ACPI_3_0_EMBEDDED_CONTROLLER_BOOT_RESOURCES_TABLE_SIGNATURE:
      break;
        //
    // "PSDT" Persistent System Description Table
          //
    case EFI_ACPI_3_0_PERSISTENT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
      break;
          //
    // "SBST" Smart Battery Specification Table
    //
    case EFI_ACPI_3_0_SMART_BATTERY_SPECIFICATION_TABLE_SIGNATURE:
          break;
    //
    // "SLIT" System Locality Information Table
    //
    case EFI_ACPI_3_0_SYSTEM_LOCALITY_INFORMATION_TABLE_SIGNATURE:
          break;
    //
    // "SRAT" Static Resource Affinity Table
    //
    case EFI_ACPI_3_0_STATIC_RESOURCE_AFFINITY_TABLE_SIGNATURE:
    break;
    //
    // "XSDT" Extended System Description Table
    //
    case EFI_ACPI_3_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
      break;
    //
    // "BOOT" MS Simple Boot Spec
    //
    case EFI_ACPI_3_0_SIMPLE_BOOT_FLAG_TABLE_SIGNATURE:
      break;
    //
    // "CPEP" Corrected Platform Error Polling Table
    //
    case EFI_ACPI_3_0_CORRECTED_PLATFORM_ERROR_POLLING_TABLE_SIGNATURE:
      break;
    //
    // "DBGP" MS Debug Port Spec
    //
    case EFI_ACPI_3_0_DEBUG_PORT_TABLE_SIGNATURE:
      break;
    //
    // "ETDT" Event Timer Description Table
    //
    case EFI_ACPI_3_0_EVENT_TIMER_DESCRIPTION_TABLE_SIGNATURE:
      break;
    //
    // "SPMI" Server Platform Management Interface Table
    //
    case EFI_ACPI_3_0_SERVER_PLATFORM_MANAGEMENT_INTERFACE_TABLE_SIGNATURE:
      break;
    //
    // "TCPA" Trusted Computing Platform Alliance Capabilities Table
    //
    case EFI_ACPI_3_0_TRUSTED_COMPUTING_PLATFORM_ALLIANCE_CAPABILITIES_TABLE_SIGNATURE:
      break;
    */
    default:
      break;
    }
  }


}



/**
  ConversionPixelCopy - Convert a source 24bpp image to 32bpp with a dynamically allocated buffer

  @param[in]  SourceBitDepth  Source image bit depth
  @param[in]  SourceBuffer    Source image buffer
  @param[in]  DestBitDepth    Destination image bit depth
  @param[in]  DestBuffer      Destination image buffer
  @param[in]  NumPixels       Number of pixels to process

  @retval     None

**/
void
ConversionPixelCopy(
    IN UINT32 SourceBitDepth,
    IN UINT8 *SourceBuffer,
    IN UINT32 DestBitDepth,
    IN UINT8* DestBuffer,
    IN UINT32 NumPixels)
{

  // Check for valid input parameters
  if (SourceBitDepth == DestBitDepth)
  {
    CopyMem(DestBuffer, SourceBuffer, NumPixels*(SourceBitDepth/8));
  }
  else if ((24 == SourceBitDepth) && (32 == DestBitDepth))
  {
    UINT32  Count;
    UINT32 *Buffer32BPP = (UINT32*)DestBuffer;
    UINT8  *Buffer24BPP = (UINT8*)SourceBuffer;

    for (Count=0;Count<NumPixels;Count++)
    {
      Buffer32BPP[Count] = (UINT32) (0x00000000 | (Buffer24BPP[2]<<16) | (Buffer24BPP[1]<<8) | (Buffer24BPP[0]));

      // Increment to the next pixel
      Buffer24BPP+=3;
    }
  }
  else
  {
    // All other conversion are not supported, fill with an solid color (gray)
    UINT32  Count;
    UINT32 *Buffer8BPP = (UINT32*)DestBuffer;

    for (Count=0;Count<NumPixels*(DestBitDepth/8);Count++)
    {
      Buffer8BPP[Count] = 0x5A;
    }
  }
}

/**
  RenderBgrtImage - Render an image on to the screen from the BGRT buffer

  @param[in]  BGRTImage      BGRT Source image buffer
  @param[in]  BGRTImageSize  BGRT image buffer size

  @retval EFI_SUCCESS            Image loaded successfully in to memory.
  @retval EFI_INVALID_PARAMETER  Invalid input parameters passed in
  @retval EFI_OUT_OF_RESOURCES   Not enough resources for buffer allocations

**/
EFI_STATUS
RenderBgrtImage(
    IN UINT8 *BgrtImage,
    IN UINT32 BgrtImageSize
)
{
  EFI_STATUS           Status = EFI_SUCCESS;
  UINT32             SizeOfX;
  UINT32             SizeOfY;
  INTN               DestX;
  INTN               DestY;
  UINTN              BltSize;
  UINTN              Height;
  UINTN              Width;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt = NULL;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL  bgPixel = {0};

  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;

  //
  // Try to open GOP first
  //
  //Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (VOID**)&GraphicsOutput);
  Status = gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **) &GraphicsOutput);  
  if (EFI_ERROR (Status)) {
    AsciiPrint("get GOP fail\n");
    return EFI_UNSUPPORTED;
  }

  SizeOfX = GraphicsOutput->Mode->Info->HorizontalResolution;
  SizeOfY = GraphicsOutput->Mode->Info->VerticalResolution;

  Blt = NULL;
  Status = ConvertBmpToGopBlt (
      BgrtImage,
      BgrtImageSize,
      (VOID**)&Blt,
      &BltSize,
      &Height,
      &Width
      );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  // Height can be negative, so take the modulus of height
  Height    = ((INT32)Height < 0) ? ((UINT32)- Height):Height;

  // Center of BGRT image should be at the horizontal center of the panel
  DestX = (SizeOfX - Width) / 2;
  // Center of BGRT image should be at 38.2 percent of panel height from the top
  DestY = (382*SizeOfY)/1000 - Height/2;

  // If LogoPosX and LogoPosY does not have valid values, set them to (0,0)
  if ((DestX < 0) ||
      (DestY < 0) ||
      ((UINT32)DestX > (SizeOfX-1)) ||
      ((UINT32)DestY > (SizeOfY-1)))
  {
    DestX = 0;
    DestY = 0;
  }

  if ((DestX >= 0) && (DestY >= 0)) {
    Status = GraphicsOutput->Blt(
                         GraphicsOutput,
                             (EFI_GRAPHICS_OUTPUT_BLT_PIXEL*)&bgPixel,
                             EfiBltVideoFill,
                             0, 0,
                             0, 0,
                             Width,
                             Height,
                             0
                             );
    if ( Status!= EFI_SUCCESS)
    {
      DEBUG((EFI_D_ERROR, "ChargerDXE - Blt(EfiBltVideoFill) failed.\r\n"));
      return Status;
    }
  
    Status = GraphicsOutput->Blt (
                            GraphicsOutput,
                            Blt,
                            EfiBltBufferToVideo,
                            0,
                            0,
                            (UINTN) DestX,
                            (UINTN) DestY,
                            Width,
                            Height,
                            Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                            );
  }

  return Status;
}


/**
*  Load, process and display OEM logo bitmaps including processing needed to support ACPI2.0 BGRT.
**/
EFI_STATUS
EFIAPI
ProcessBgrt(void *AcpiTable)
{
  EFI_STATUS                                Status = EFI_SUCCESS;
  EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE *BgrtTable = (EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE *)AcpiTable;

  DEBUG ((EFI_D_INFO, "%a\r\n", __FUNCTION__));

  // Validate the table
  if(NULL == BgrtTable)
  {
    DEBUG ((EFI_D_INFO, "ProcessBgrt: No BGRT table found\n"));
    Status = EFI_INVALID_PARAMETER;
  }
  else if ((EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE_SIGNATURE != BgrtTable->Header.Signature) || // "BGRT" Signature
      (EFI_ACPI_5_0_BGRT_VERSION != BgrtTable->Version) ||                       // Valid version 1.0
      (EFI_ACPI_5_0_BGRT_IMAGE_TYPE_BMP != BgrtTable->ImageType))                // Valid type BMP
  {
    DEBUG ((EFI_D_INFO, "ProcessBGRT: BGRT table header corrupt or invalid!\n"));
    Status = EFI_LOAD_ERROR;
  }
  else
  {
    //UINT32        ImageSize      = 0;
    UINT8        *BgrtAddress    = NULL;
    UINT32        BgrtBufferSize = 0;
    CHAR16      BmpFile[64]={0};

    //
    // 1. Read and display logo1.bmp (Located in the root of the GPP (WA)/User(WP) parition)
    //
    //
    AsciiStrToUnicodeStr("logo\\logo1.bmp",BmpFile);
    if (EFI_SUCCESS == (Status = ReadFileFromAwFs(BmpFile, (VOID**)&BgrtAddress, &BgrtBufferSize)))
    {
      BOOLEAN      bSecondaryLogoLoaded = FALSE;

      if (EFI_SUCCESS == (Status = RenderBgrtImage(BgrtAddress, BgrtBufferSize)))
      {
        DEBUG ((EFI_D_INFO, "ProcessBGRT: OEM Logo1 Successfully Loaded\n"));

        // As logo1.bmp is rendered to the panel, set the status field to 1.
        BgrtTable->Status = 0x1;
      }

      //
      // 2. Read logo2.bmp (Located in the root of the GPP (WA)/User(WP)  parition) and copy the raw data to the BGRT address
      //    This portion is independent of loading logo1, however this portion is optional and logo2.bmp is not required.
      //    No status is reported for logo2.bmp, status is only reported for loading of logo1 since logo2 is optional.
      //
      AsciiStrToUnicodeStr("logo\\logo2.bmp",BmpFile);
      if (EFI_SUCCESS == ReadFileFromAwFs(BmpFile, (VOID**)&BgrtAddress, &BgrtBufferSize))
      {
        DEBUG ((EFI_D_INFO, "ProcessBGRT: OEM Logo2 Successfully Loaded\n"));

        bSecondaryLogoLoaded = TRUE;

        // As logo2.bmp is loaded into the BGRT address, set the status to 0 to give an indication to OS to render this image.
        BgrtTable->Status = 0x0;
      }

      if ((EFI_SUCCESS == Status) || (TRUE == bSecondaryLogoLoaded))
      {
        EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput = NULL;

        // Open GOP(Graphics output protocol) to retrieve panel resolution.
        //Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (VOID **) &GraphicsOutput);
        Status = gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **) &GraphicsOutput);

        // Validate
        if ((EFI_SUCCESS != Status) ||
            (NULL == GraphicsOutput) ||
            (0 == GraphicsOutput->Mode->Info->HorizontalResolution) ||
            (0 == GraphicsOutput->Mode->Info->VerticalResolution))
        {
          // GOP protocol failed
        }
        else
        {
          UINT32            SizeOfX;
          UINT32            SizeOfY;
          INTN              DestX;
          INTN              DestY;
          UINTN             BltSize;
          UINTN             Height;
          UINTN             Width;
          EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt = NULL;

          SizeOfX = GraphicsOutput->Mode->Info->HorizontalResolution;
          SizeOfY = GraphicsOutput->Mode->Info->VerticalResolution;

          Blt = NULL;
          Status = ConvertBmpToGopBlt (
                                 BgrtAddress,
                                 BgrtBufferSize,
                                 (VOID**)&Blt,
                                 &BltSize,
                                 &Height,
                                 &Width
                                 );
          if (EFI_ERROR (Status)) {
            return Status;
          }
      
          // Height can be negative, so take the modulus of height
          Height    = ((INT32)Height < 0) ? ((UINT32)- Height):Height;

          // Center of BGRT image should be at the horizontal center of the panel
          DestX = (SizeOfX - Width) / 2;
          // Center of BGRT image should be at 38.2 percent of panel height from the top
          DestY = (382*SizeOfY)/1000 - Height/2;

          // If LogoPosX and LogoPosY does not have valid values, set them to (0,0)
          if ((DestX < 0) ||
              (DestY < 0) ||
              ((UINT32)DestX > (SizeOfX-1)) ||
              ((UINT32)DestY > (SizeOfY-1)))
          {
            DestX = 0;
            DestY = 0;
          }

          // Update the image offsets
          BgrtTable->ImageOffsetX = (UINT32)DestX;
          BgrtTable->ImageOffsetY = (UINT32)DestY;
        }

        // Update BGRT Table with the dynamically allocated image address.
        //BgrtTable->ImageAddress = (UINT64)BgrtAddress;
        BgrtTable->ImageAddress = (UINT32)BgrtAddress;
      }
    }
  }

  return Status;
}

/**
  Access ACPI Table from bootloader partition.

  @param  File Path

  @return EFI_SUCCESS
  @return EFI_LOAD_ERROR
  @return EFI_OUT_OF_RESOURCES

**/
EFI_STATUS
LoadAcpiFromPartition(
  CHAR16        *Path
)
{
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Volume;
  EFI_ACPI_TABLE_PROTOCOL           *AcpiTable;
  EFI_FILE_HANDLE                   RootFileHandle;
  EFI_FILE_HANDLE                   DirFileHandle;
  EFI_FILE_HANDLE                   AcpiFileHandle;
  EFI_FILE_INFO                     *DirInfo = NULL, *Info = NULL;
  VOID                              *AcpiFileBuffer = NULL;
  UINTN                             AcpiFileBufferSize = 0, BufferSize = 0;
  UINTN                             ReadSize;
  UINTN                             TableSize;
  UINTN                             TableCount = 0;
  UINTN                     NumOfAcpiTables = 0;
  UINTN                             TableHandle;

  STATIC CHAR8             FileName[1024];
  EFI_ACPI_TABLE_VERSION           Version;
  EFI_STATUS           Status;
  UINTN                Index;
  UINTN                HandleCount;
  EFI_HANDLE           *HandleBuffer;
  UINTN                ErrFlag = 0;
  UINT32               Signature;

  //
  // Find the AcpiTable protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID**)&AcpiTable);
  if (EFI_ERROR (Status)) {
    return EFI_ABORTED;
  }

  //Locate handles for SimpleFileSystem protocol
  Status = gBS->LocateHandleBuffer (
          ByProtocol,
          &gEfiSimpleFileSystemProtocolGuid,
          NULL,
          &HandleCount,
          &HandleBuffer
          );
  if (EFI_ERROR(Status))
  {
    AsciiPrint("%a,Access simple file system failure\n",__FUNCTION__);
    return Status;
  }
  for (Index = 0; Index < HandleCount; Index++) {
    // File the file system interface to the device
    Status = gBS->HandleProtocol (
                  HandleBuffer[Index],
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID *) &Volume
                  );
    if(Status != EFI_SUCCESS) {
      DEBUG ((EFI_D_WARN, "ACPI: LoadAcpiFromVolume() unable to find file system interface.\r\n" ));
      continue;
    }

  // Open the root directory of the volume
    if (!EFI_ERROR (Status)) {
    Status = Volume->OpenVolume (
                      Volume,
                      &RootFileHandle
                      );
    }
    if(Status != EFI_SUCCESS) {
      DEBUG ((EFI_D_WARN, "ACPI: LoadAcpiFromVolume() failed to open volume \r\n" ));
      continue;
    }

    //Open Directory
    Status = RootFileHandle->Open(RootFileHandle,
                   &DirFileHandle,
                   Path,
                   EFI_FILE_MODE_READ,
                   0);
    if ((Status != EFI_SUCCESS) || (DirFileHandle == NULL))
    {
      DEBUG ((EFI_D_WARN, "ACPI: LoadAcpiFromVolume() failed to open directory\r\n" ));
      continue;
    }

    Status = DirFileHandle->SetPosition(DirFileHandle, 0);
    if(Status != EFI_SUCCESS)
      continue;

    while (1) {
      // Look at each directory listing
      // First read gets the size
      ReadSize = 0;
      Status = DirFileHandle->Read (DirFileHandle, &ReadSize, DirInfo);
      if (Status == EFI_BUFFER_TOO_SMALL) {
        // Allocate the buffer for the real read
        DirInfo = AllocatePool (ReadSize);
        if (DirInfo == NULL) {
          ErrFlag = 1;
          break;
        }

        // Read the data
        Status = DirFileHandle->Read (DirFileHandle, &ReadSize, DirInfo);
        if ((EFI_ERROR (Status)) || (ReadSize == 0)) {
          ErrFlag = 1;
          break;
        }
      } else {
        ErrFlag = 1;
        break;
      }

      if (DirInfo->Attribute & EFI_FILE_DIRECTORY) {
        // Silently skip folders
        FreePool (DirInfo);
        DirInfo = NULL;
        continue;
      }
      UnicodeStrToAsciiStr (DirInfo->FileName, FileName);
      DEBUG ((DEBUG_LOAD, "ACPI: Loading %a\r\n", FileName));
      Status = DirFileHandle->Open(DirFileHandle,
                   &AcpiFileHandle,
                   DirInfo->FileName,
                   EFI_FILE_MODE_READ,
                   0);
      if(Status != EFI_SUCCESS){
        ErrFlag = 1;
        break;
      }
      //Get file size info
      BufferSize  = SIZE_OF_EFI_FILE_INFO + 200;
      do{
        Info   = NULL;
        Info = AllocatePool (BufferSize);
        if (Info == NULL){
          ErrFlag = 1;
          break;
        }
        Status = AcpiFileHandle->GetInfo (
                           AcpiFileHandle,
                           &gEfiFileInfoGuid,
                           &BufferSize,
                           Info
                        );
        if(Status == EFI_SUCCESS)
          break;
        if (Status != EFI_BUFFER_TOO_SMALL) {
          FreePool (Info);
          ErrFlag = 1;
          break;
        }
        FreePool (Info);
      } while (TRUE);

      if(ErrFlag == 1){
        continue;
      }
      AcpiFileBufferSize = Info->FileSize;
      FreePool (Info);
      AcpiFileBuffer = AllocatePool(AcpiFileBufferSize);
      // AllocatePool failed for some reason
      if (AcpiFileBuffer == NULL)
      {
        DEBUG(( EFI_D_WARN, " ACPI: AcpiFileBuffer is null\r\n"));
        continue;
      }
      // Read file content
      Status = AcpiFileHandle->Read (AcpiFileHandle, &AcpiFileBufferSize, AcpiFileBuffer);
      if ((EFI_ERROR (Status)) || (AcpiFileBufferSize == 0)) {
        UnicodeStrToAsciiStr (DirInfo->FileName, FileName);
        DEBUG ((EFI_D_WARN, "ACPI: LoadAcpiFromVolume() failed reading %a\r\n", FileName));
        continue;
      }

      // Add the table
      TableSize = ((EFI_ACPI_DESCRIPTION_HEADER *) AcpiFileBuffer)->Length;
      ASSERT (AcpiFileBufferSize >= TableSize);

      /* process bgrt table */
      Signature = ((EFI_ACPI_DESCRIPTION_HEADER *) AcpiFileBuffer)->Signature;
      if(Signature == EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE_SIGNATURE)
        ProcessBgrt(AcpiFileBuffer);

      //
      // Perform any table specific updates.
      //
      AcpiUpdateTable ((EFI_ACPI_DESCRIPTION_HEADER *) AcpiFileBuffer, &Version);

      // Checksum ACPI table
      AcpiPlatformChecksum ((UINT8*)AcpiFileBuffer, TableSize);

      //
      // Install ACPI table
      //
      Status = AcpiTable->InstallAcpiTable (
                            AcpiTable,
                            AcpiFileBuffer,
                            TableSize,
                            &TableHandle
                            );

      // increase the table count
      NumOfAcpiTables++;
      TableCount++;

      FreePool (DirInfo);
      DirInfo = NULL;
      FreePool(AcpiFileBuffer);
      AcpiFileBuffer = NULL;

    //if( DirFileHandle != NULL)
    //  DirFileHandle->Close(DirFileHandle);
    }
    if(TableCount != 0){
      break;
    }
  }

  if (0 == TableCount) {
    Status = EFI_NOT_FOUND;
  }
  else {
    DEBUG ((DEBUG_LOAD, "ACPI: Loaded %d tables\r\n", TableCount));
  }

  if (DirInfo != NULL)
    FreePool (DirInfo);

  return Status;

}


/**
  Entrypoint of Acpi Platform driver.

  @param  ImageHandle
  @param  SystemTable

  @return EFI_SUCCESS
  @return EFI_LOAD_ERROR
  @return EFI_OUT_OF_RESOURCES

**/
EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                     Status;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTable;
  EFI_FIRMWARE_VOLUME2_PROTOCOL  *FwVol;
  INTN                           Instance;
  EFI_ACPI_COMMON_HEADER         *CurrentTable;
  UINTN                          TableHandle;
  UINT32                         FvStatus;
  UINTN                          TableSize;
  UINTN                          Size;
  EFI_ACPI_TABLE_VERSION       Version;
  UINT32               Signature;

  Instance     = 0;
  CurrentTable = NULL;
  TableHandle  = 0;
  //
  // Find the AcpiTable protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID**)&AcpiTable);
  if (EFI_ERROR (Status)) {
    return EFI_ABORTED;
  }

  Status = LoadAcpiFromPartition(EFI_ACPI_TABLE_FILE_DIR);
  if (!EFI_ERROR(Status)) {
    AsciiPrint("Install ACPI table by partition file success\n");
    return EFI_SUCCESS;
  }
  else{
    AsciiPrint("Read ACPI Table From Bootloader partition fail, try to read it from FV\n");
  }

  //
  // Locate the firmware volume protocol
  //
  Status = LocateFvInstanceWithTables (&FwVol);
  if (EFI_ERROR (Status)) {
    return EFI_ABORTED;
  }
  //
  // Read tables from the storage file.
  //
  while (Status == EFI_SUCCESS) {
    Status = FwVol->ReadSection (
                      FwVol,
                      (EFI_GUID*)PcdGetPtr (PcdAcpiTableStorageFile),
                      EFI_SECTION_RAW,
                      Instance,
                      (VOID**) &CurrentTable,
                      &Size,
                      &FvStatus
                      );
    if (!EFI_ERROR(Status)) {
      //
      // Add the table
      //
      TableHandle = 0;

      TableSize = ((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable)->Length;
      ASSERT (Size >= TableSize);

      /* process bgrt table */
      Signature = ((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable)->Signature;
      if(Signature == EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE_SIGNATURE)
        ProcessBgrt(CurrentTable);

      //
      // Perform any table specific updates.
      //
      AcpiUpdateTable ((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable, &Version);

      //
      // Checksum ACPI table
      //
      AcpiPlatformChecksum ((UINT8*)CurrentTable, TableSize);

      //
      // Install ACPI table
      //
      Status = AcpiTable->InstallAcpiTable (
                            AcpiTable,
                            CurrentTable,
                            TableSize,
                            &TableHandle
                            );

      //
      // Free memory allocated by ReadSection
      //
      gBS->FreePool (CurrentTable);

      if (EFI_ERROR(Status)) {
        return EFI_ABORTED;
      }

      //
      // Increment the instance
      //
      Instance++;
      CurrentTable = NULL;
    }
  }

  return EFI_SUCCESS;
}
