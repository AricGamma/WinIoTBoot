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

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Protocol/FirmwareVolumeBlock.h>
#include <Protocol/BlockIo.h>
#include <Library/MemoryAllocationLib.h>

#include <Guid/VariableFormat.h>
#include <Guid/SystemNvDataGuid.h>

#include <Library/SunxiPartitionLib.h>
#include <Library/SunxiMbr.h>
#include <Interinc/sunxi_uefi.h>

#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <Library/HobLib.h>
#include <Guid/SunxiVariableHob.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiBootInfoLib.h>

EFI_FVB_ATTRIBUTES_2 gAttribute = (EFI_FVB2_READ_STATUS|EFI_FVB2_WRITE_STATUS|EFI_FVB2_ALIGNMENT_32);
EFI_BLOCK_IO_PROTOCOL *BlockIo = NULL;
EFI_EVENT mBlockIORegistration = NULL;
EFI_HANDLE mHandle  = NULL;
UINT32 TargetMediaId;
static EFI_PHYSICAL_ADDRESS FlashNvStorageVariableBase = 0;
static SUNXI_VARIABLE_HOB      *SunxiVariableHob  = NULL;
//static struct spare_boot_head_t* spare_head = NULL;


#define EMMC_BLOCK_SIZE 512

//#define EMMC_NVRAM_BLOCK_OFFSET 0x1000  //the block offset value to address the nvram variable base in RPMB hardware partition.
static EFI_LBA EMMC_NVRAM_BLOCK_OFFSET=0;


EFI_STATUS
EFIAPI
FvbBlockIoWrite (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  IN VOID                           *Buffer
  )
{
  EFI_STATUS Status=EFI_SUCCESS;
  Status = This->WriteBlocks(This, MediaId, Lba, BufferSize, Buffer);
  if(EFI_ERROR(Status))
  {
    return Status;
  }
  Status = This->FlushBlocks(This);
  return Status;
}

EFI_STATUS
EFIAPI
FvbBlockIoRead(
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  IN VOID                           *Buffer
  )  
{
  EFI_STATUS Status=EFI_SUCCESS;
  //DEBUG ((EFI_D_INFO, "FVB:FvbBlockIoRead lba = 0x%x\n",  (UINTN)Lba));
  Status = This->ReadBlocks(This, MediaId, Lba, BufferSize, Buffer);

  return Status; 
}
/**
  The GetAttributes() function retrieves the attributes and
  current settings of the block.

  @param This       Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Attributes Pointer to EFI_FVB_ATTRIBUTES_2 in which the
                    attributes and current settings are
                    returned. Type EFI_FVB_ATTRIBUTES_2 is defined
                    in EFI_FIRMWARE_VOLUME_HEADER.

  @retval EFI_SUCCESS The firmware volume attributes were
                      returned.

**/

EFI_STATUS
EFIAPI
FvbGetAttributes (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  OUT       EFI_FVB_ATTRIBUTES_2                *Attributes
  )
{
  *Attributes = gAttribute;
  DEBUG ((EFI_D_INFO, "FVB:FvbGetAttributes 0x%x\n",  gAttribute));
  return EFI_SUCCESS;
}


/**
  The SetAttributes() function sets configurable firmware volume
  attributes and returns the new settings of the firmware volume.


  @param This        Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Attributes   On input, Attributes is a pointer to
                      EFI_FVB_ATTRIBUTES_2 that contains the
                      desired firmware volume settings. On
                      successful return, it contains the new
                      settings of the firmware volume. Type
                      EFI_FVB_ATTRIBUTES_2 is defined in
                      EFI_FIRMWARE_VOLUME_HEADER.

  @retval EFI_SUCCESS           The firmware volume attributes were returned.

  @retval EFI_INVALID_PARAMETER The attributes requested are in
                                conflict with the capabilities
                                as declared in the firmware
                                volume header.

**/
EFI_STATUS
EFIAPI
FvbSetAttributes (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  IN OUT    EFI_FVB_ATTRIBUTES_2                *Attributes
  )
{
  gAttribute |= *Attributes;
  *Attributes = gAttribute;
  DEBUG ((EFI_D_INFO, "FVB:FvbSetAttributes 0x%x\n",  gAttribute));
  return EFI_SUCCESS;
}


/**
  The GetPhysicalAddress() function retrieves the base address of
  a memory-mapped firmware volume. This function should be called
  only for memory-mapped firmware volumes.

  @param This     Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Address  Pointer to a caller-allocated
                  EFI_PHYSICAL_ADDRESS that, on successful
                  return from GetPhysicalAddress(), contains the
                  base address of the firmware volume.

  @retval EFI_SUCCESS       The firmware volume base address was returned.

  @retval EFI_NOT_SUPPORTED The firmware volume is not memory mapped.

**/
EFI_STATUS
EFIAPI
FvbGetPhysicalAddress (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  OUT       EFI_PHYSICAL_ADDRESS                *Address
  )
{
  EFI_PHYSICAL_ADDRESS NVBase = FlashNvStorageVariableBase;//PcdGet32(PcdFlashNvStorageVariableBase);

  *Address = NVBase;
  //DEBUG ((EFI_D_INFO, "FVB:FvbGetPhysicalAddress Addr:0x%x\n", *Address));    
  return EFI_SUCCESS;
}

/**
  The GetBlockSize() function retrieves the size of the requested
  block. It also returns the number of additional blocks with
  the identical size. The GetBlockSize() function is used to
  retrieve the block map (see EFI_FIRMWARE_VOLUME_HEADER).


  @param This        Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Lba         Indicates the block for which to return the size.

  @param BlockSize   Pointer to a caller-allocated UINTN in which
                     the size of the block is returned.

  @param NumberOfBlocks Pointer to a caller-allocated UINTN in
                        which the number of consecutive blocks,
                        starting with Lba, is returned. All
                        blocks in this range have a size of
                        BlockSize.


  @retval EFI_SUCCESS             The firmware volume base address was returned.

  @retval EFI_INVALID_PARAMETER   The requested LBA is out of range.

**/
EFI_STATUS
EFIAPI
FvbGetBlockSize (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  IN        EFI_LBA                             Lba,
  OUT       UINTN                               *BlockSize,
  OUT       UINTN                               *NumberOfBlocks
  )
{
  EFI_STATUS status = EFI_SUCCESS;
  *BlockSize = EMMC_BLOCK_SIZE;
  *NumberOfBlocks = SunxiVariableHob->SunxiVariableSize/EMMC_BLOCK_SIZE;

  DEBUG ((EFI_D_INFO, "FVB:FvbGetBlockSize numblocks:%d\n",  *NumberOfBlocks));
  return status;
}

/**
  Reads the specified number of bytes into a buffer from the specified block.

  The Read() function reads the requested number of bytes from the
  requested block and stores them in the provided buffer.
  Implementations should be mindful that the firmware volume
  might be in the ReadDisabled state. If it is in this state,
  the Read() function must return the status code
  EFI_ACCESS_DENIED without modifying the contents of the
  buffer. The Read() function must also prevent spanning block
  boundaries. If a read is requested that would span a block
  boundary, the read must read up to the boundary but not
  beyond. The output parameter NumBytes must be set to correctly
  indicate the number of bytes actually read. The caller must be
  aware that a read may be partially completed.

  @param This     Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Lba      The starting logical block index
                  from which to read.

  @param Offset   Offset into the block at which to begin reading.

  @param NumBytes Pointer to a UINTN. At entry, *NumBytes
                  contains the total size of the buffer. At
                  exit, *NumBytes contains the total number of
                  bytes read.

  @param Buffer   Pointer to a caller-allocated buffer that will
                  be used to hold the data that is read.

  @retval EFI_SUCCESS         The firmware volume was read successfully,
                              and contents are in Buffer.

  @retval EFI_BAD_BUFFER_SIZE Read attempted across an LBA
                              boundary. On output, NumBytes
                              contains the total number of bytes
                              returned in Buffer.

  @retval EFI_ACCESS_DENIED   The firmware volume is in the
                              ReadDisabled state.

  @retval EFI_DEVICE_ERROR    The block device is not
                              functioning correctly and could
                              not be read.

**/

EFI_STATUS
EFIAPI
FvbRead (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  IN        EFI_LBA                             Lba,
  IN        UINTN                               Offset,
  IN OUT    UINTN                               *NumBytes,
  IN OUT    UINT8                               *Buffer
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32 NumBlock;
  UINT32 AllocSize = 0;
  EFI_LBA NV_Lba = Lba + EMMC_NVRAM_BLOCK_OFFSET;
  UINT8 *BlockBuffer;
  
  if (WORK_MODE_BOOT != SunxiGetBootWorkMode())
  {
    DEBUG ((EFI_D_INFO, "FVB:FvbRead workmode is not boot ,do nothing\n"));
    return EFI_SUCCESS;
  }
  //DEBUG ((EFI_D_INFO, "FVB:FvbRead O:%d ",Offset));
  //DEBUG ((EFI_D_INFO, "Lba:%d \n",Lba));    
  //DEBUG ((EFI_D_INFO, "N:%d \n",*NumBytes));    

  // We must have some bytes to write
  if (*NumBytes == 0) {
    DEBUG ((EFI_D_ERROR, "FvbWrite: ERROR - EFI_BAD_BUFFER_SIZE\n"));
    return EFI_BAD_BUFFER_SIZE;
  }
   
  //calculate block number and allocate memory
  if (0 == ((Offset + *NumBytes)%EMMC_BLOCK_SIZE)) 
  {
    NumBlock = ((Offset + *NumBytes)/EMMC_BLOCK_SIZE);
  } 
  else 
  {
    NumBlock = ((Offset + *NumBytes)/EMMC_BLOCK_SIZE)+1;
  } 

  AllocSize = NumBlock*EMMC_BLOCK_SIZE;

  // Allocate runtime memory to read in the NOR Flash data. Variable Services are runtime.
  BlockBuffer = AllocateRuntimePool (AllocSize);

  // Check we did get some memory
  if( BlockBuffer == NULL ) {
    DEBUG ((EFI_D_ERROR, "FvbWrite: ERROR - Can not allocate BlockBuffer @ 0x%08x.\n", BlockBuffer));
    return EFI_DEVICE_ERROR;
  }
  
  Status = FvbBlockIoRead(BlockIo, BlockIo->Media->MediaId, NV_Lba, AllocSize, BlockBuffer);

  // 4. Copy read buffer to dest
  CopyMem(Buffer, (UINT8 *)(BlockBuffer +Offset), *NumBytes);

  FreePool(BlockBuffer);

  if(Status!=EFI_SUCCESS)
  {
    DEBUG ((EFI_D_ERROR, "FVB:FvbRead Failed %r\n",  Status));
    Status = EFI_ACCESS_DENIED;
    goto Exit;
  }
    
Exit:
  return Status;
}


/**
  Writes the specified number of bytes from the input buffer to the block.

  The Write() function writes the specified number of bytes from
  the provided buffer to the specified block and offset. If the
  firmware volume is sticky write, the caller must ensure that
  all the bits of the specified range to write are in the
  EFI_FVB_ERASE_POLARITY state before calling the Write()
  function, or else the result will be unpredictable. This
  unpredictability arises because, for a sticky-write firmware
  volume, a write may negate a bit in the EFI_FVB_ERASE_POLARITY
  state but cannot flip it back again.  Before calling the
  Write() function,  it is recommended for the caller to first call
  the EraseBlocks() function to erase the specified block to
  write. A block erase cycle will transition bits from the
  (NOT)EFI_FVB_ERASE_POLARITY state back to the
  EFI_FVB_ERASE_POLARITY state. Implementations should be
  mindful that the firmware volume might be in the WriteDisabled
  state. If it is in this state, the Write() function must
  return the status code EFI_ACCESS_DENIED without modifying the
  contents of the firmware volume. The Write() function must
  also prevent spanning block boundaries. If a write is
  requested that spans a block boundary, the write must store up
  to the boundary but not beyond. The output parameter NumBytes
  must be set to correctly indicate the number of bytes actually
  written. The caller must be aware that a write may be
  partially completed. All writes, partial or otherwise, must be
  fully flushed to the hardware before the Write() service
  returns.

  @param This     Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Lba      The starting logical block index to write to.

  @param Offset   Offset into the block at which to begin writing.

  @param NumBytes The pointer to a UINTN. At entry, *NumBytes
                  contains the total size of the buffer. At
                  exit, *NumBytes contains the total number of
                  bytes actually written.

  @param Buffer   The pointer to a caller-allocated buffer that
                  contains the source for the write.

  @retval EFI_SUCCESS         The firmware volume was written successfully.

  @retval EFI_BAD_BUFFER_SIZE The write was attempted across an
                              LBA boundary. On output, NumBytes
                              contains the total number of bytes
                              actually written.

  @retval EFI_ACCESS_DENIED   The firmware volume is in the
                              WriteDisabled state.

  @retval EFI_DEVICE_ERROR    The block device is malfunctioning
                              and could not be written.


**/
EFI_STATUS
EFIAPI
FvbWrite (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  IN        EFI_LBA                             Lba,
  IN        UINTN                               Offset,
  IN OUT    UINTN                               *NumBytes,
  IN        UINT8                               *Buffer
  )
{  
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32 NumBlock;
  UINT32 AllocSize = 0;
  EFI_LBA NV_Lba = Lba + EMMC_NVRAM_BLOCK_OFFSET;
  UINT32 NVBase = (UINT32)FlashNvStorageVariableBase;//PcdGet32(PcdFlashNvStorageVariableBase);    
  UINT8       *BlockBuffer;

  if (WORK_MODE_BOOT != SunxiGetBootWorkMode())
  {
    DEBUG ((EFI_D_INFO, "FVB:FvbWrite workmode is not boot ,do nothing\n"));
    return EFI_SUCCESS;
  }

  //DEBUG ((EFI_D_INFO, "FvbWrite O:%d\n ", Offset));
  //DEBUG ((EFI_D_INFO, "Lba:%d \n",Lba));    
  //DEBUG ((EFI_D_INFO, "N:%d ", *NumBytes));

  // We must have some bytes to write
  if (*NumBytes == 0) {
    DEBUG ((EFI_D_ERROR, "FvbWrite: ERROR - EFI_BAD_BUFFER_SIZE\n"));
    return EFI_BAD_BUFFER_SIZE;
  }
   
  //just do a memory copy proccess  write the data to the FV in the ddr memory.
  CopyMem((VOID *)(UINT32)(NVBase+Lba*EMMC_BLOCK_SIZE + Offset), Buffer, *NumBytes);

  // 1. Calculate block count number
  if (0 == ((Offset+*NumBytes)%EMMC_BLOCK_SIZE)) 
  {
    NumBlock = ((Offset+*NumBytes)/EMMC_BLOCK_SIZE);
  } 
  else 
  {
    NumBlock = ((Offset+*NumBytes)/EMMC_BLOCK_SIZE) + 1;
  } 
  AllocSize = (NumBlock * EMMC_BLOCK_SIZE);

  // Allocate runtime memory to read in the NOR Flash data.
  // Since the intention is to use this with Variable Services and since these are runtime,
  // allocate the memory from the runtime pool.
  BlockBuffer = AllocateRuntimePool (AllocSize);

  // Check we did get some memory
  if( BlockBuffer == NULL ) {
    DEBUG ((EFI_D_ERROR, "FvbWrite: ERROR - Can not allocate BlockBuffer @ 0x%08x.\n", BlockBuffer));
    return EFI_DEVICE_ERROR;
  }

  // Read EMMC data into shadow buffer
  Status = FvbBlockIoRead(BlockIo, TargetMediaId, NV_Lba, AllocSize, (VOID *)(BlockBuffer));
  if(Status!=EFI_SUCCESS)
  {
    DEBUG ((EFI_D_ERROR, "FVB:FvbRead Failed %r\n",  Status));
    Status = EFI_ACCESS_DENIED;
    goto Exit;
  }    

  // Put the data at the appropriate location inside the buffer area
  CopyMem((BlockBuffer + Offset), Buffer, *NumBytes);
  
  // Apply the offset and WriteBlock
  Status = FvbBlockIoWrite(BlockIo, TargetMediaId, NV_Lba, AllocSize, (VOID *)(BlockBuffer));
  if(Status!=EFI_SUCCESS)
  {
    DEBUG ((EFI_D_ERROR, "FVB:FvbWrite Failed %r\n",  Status));
    Status = EFI_ACCESS_DENIED;
  }    

Exit:
  FreePool(BlockBuffer);
  return Status;
}


/**
  Erases and initializes a firmware volume block.

  The EraseBlocks() function erases one or more blocks as denoted
  by the variable argument list. The entire parameter list of
  blocks must be verified before erasing any blocks. If a block is
  requested that does not exist within the associated firmware
  volume (it has a larger index than the last block of the
  firmware volume), the EraseBlocks() function must return the
  status code EFI_INVALID_PARAMETER without modifying the contents
  of the firmware volume. Implementations should be mindful that
  the firmware volume might be in the WriteDisabled state. If it
  is in this state, the EraseBlocks() function must return the
  status code EFI_ACCESS_DENIED without modifying the contents of
  the firmware volume. All calls to EraseBlocks() must be fully
  flushed to the hardware before the EraseBlocks() service
  returns.

  @param This   Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL
                instance.

  @param ...    The variable argument list is a list of tuples.
                Each tuple describes a range of LBAs to erase
                and consists of the following:
                - An EFI_LBA that indicates the starting LBA
                - A UINTN that indicates the number of blocks to
                  erase.

                The list is terminated with an
                EFI_LBA_LIST_TERMINATOR. For example, the
                following indicates that two ranges of blocks
                (5-7 and 10-11) are to be erased: EraseBlocks
                (This, 5, 3, 10, 2, EFI_LBA_LIST_TERMINATOR);

  @retval EFI_SUCCESS The erase request successfully
                      completed.

  @retval EFI_ACCESS_DENIED   The firmware volume is in the
                              WriteDisabled state.
  @retval EFI_DEVICE_ERROR  The block device is not functioning
                            correctly and could not be written.
                            The firmware device may have been
                            partially erased.
  @retval EFI_INVALID_PARAMETER One or more of the LBAs listed
                                in the variable argument list do
                                not exist in the firmware volume.

**/
EFI_STATUS
EFIAPI
FvbEraseBlocks (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL *This,
  ...
  )
{
  DEBUG ((EFI_D_INFO, "FvbEraseBlocks\n"));
  return EFI_SUCCESS;
}


//
// Making this global saves a few bytes in image size
//
EFI_HANDLE  gFvbHandle = NULL;


///
/// The Firmware Volume Block Protocol is the low-level interface
/// to a firmware volume. File-level access to a firmware volume
/// should not be done using the Firmware Volume Block Protocol.
/// Normal access to a firmware volume must use the Firmware
/// Volume Protocol. Typically, only the file system driver that
/// produces the Firmware Volume Protocol will bind to the
/// Firmware Volume Block Protocol.
///
EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL gFvbProtocol = {
  FvbGetAttributes,
  FvbSetAttributes,
  FvbGetPhysicalAddress,
  FvbGetBlockSize,
  FvbRead,
  FvbWrite,
  FvbEraseBlocks,
  ///
  /// The handle of the parent firmware volume.
  ///
  NULL
};


EFI_STATUS
EFIAPI
ValidateFvHeader (
  IN  UINT32 FvbBaseAddress
  )
{
  UINT16                      Checksum;
  EFI_FIRMWARE_VOLUME_HEADER  *FwVolHeader;
  VARIABLE_STORE_HEADER       *VariableStoreHeader;
  UINTN                       VariableStoreLength;
  UINTN             FvLength;

  FwVolHeader = (EFI_FIRMWARE_VOLUME_HEADER*)(UINT32)FvbBaseAddress;

  FvLength = PcdGet32(PcdFlashNvStorageVariableSize);

  //
  // Verify the header revision, header signature, length
  // Length of FvBlock cannot be 2**64-1
  // HeaderLength cannot be an odd number
  //
  if (   (FwVolHeader->Revision  != EFI_FVH_REVISION)
      || (FwVolHeader->Signature != EFI_FVH_SIGNATURE)
      || (FwVolHeader->FvLength  != FvLength)
      )
  {
    DEBUG ((EFI_D_ERROR, "ValidateFvHeader: No Firmware Volume header present\n"));
    return EFI_NOT_FOUND;
  }

  // Check the Firmware Volume Guid
  if( CompareGuid (&FwVolHeader->FileSystemGuid, &gEfiSystemNvDataFvGuid) == FALSE ) {
    DEBUG ((EFI_D_ERROR, "ValidateFvHeader: Firmware Volume Guid non-compatible\n"));
    return EFI_NOT_FOUND;
  }

  // Verify the header checksum
  Checksum = CalculateSum16((UINT16*)FwVolHeader, FwVolHeader->HeaderLength);
  if (Checksum != 0) {
    DEBUG ((EFI_D_ERROR, "ValidateFvHeader: FV checksum is invalid (Checksum:0x%X)\n",Checksum));
    return EFI_NOT_FOUND;
  }

  VariableStoreHeader = (VARIABLE_STORE_HEADER*)((UINTN)FwVolHeader + FwVolHeader->HeaderLength);

  // Check the Variable Store Guid
  if( CompareGuid (&VariableStoreHeader->Signature, &gEfiVariableGuid) == FALSE ) {
    DEBUG ((EFI_D_ERROR, "ValidateFvHeader: Variable Store Guid non-compatible\n"));
    return EFI_NOT_FOUND;
  }

  VariableStoreLength = PcdGet32 (PcdFlashNvStorageVariableSize) - FwVolHeader->HeaderLength;
  if (VariableStoreHeader->Size != VariableStoreLength) {
    DEBUG ((EFI_D_ERROR, "ValidateFvHeader: Variable Store Length does not match\n"));
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InitializeFvAndVariableStoreHeaders (IN UINT32 WorkMode)
{
  EFI_STATUS                          Status = EFI_SUCCESS;
  VOID*                               Headers;
  UINTN                               HeadersLength;
  EFI_FIRMWARE_VOLUME_HEADER          *FirmwareVolumeHeader;
  VARIABLE_STORE_HEADER               *VariableStoreHeader;

  HeadersLength = sizeof(EFI_FIRMWARE_VOLUME_HEADER) + sizeof(EFI_FV_BLOCK_MAP_ENTRY) + sizeof(VARIABLE_STORE_HEADER);
  Headers = AllocateZeroPool(HeadersLength);

  //
  // EFI_FIRMWARE_VOLUME_HEADER
  //
  FirmwareVolumeHeader = (EFI_FIRMWARE_VOLUME_HEADER*)Headers;
  CopyGuid (&FirmwareVolumeHeader->FileSystemGuid, &gEfiSystemNvDataFvGuid);
  FirmwareVolumeHeader->FvLength = PcdGet32(PcdFlashNvStorageVariableSize);
      
  FirmwareVolumeHeader->Signature = EFI_FVH_SIGNATURE;
  FirmwareVolumeHeader->Attributes = (EFI_FVB_ATTRIBUTES_2) (
                                          EFI_FVB2_READ_ENABLED_CAP   | // Reads may be enabled
                                          EFI_FVB2_READ_STATUS        | // Reads are currently enabled
                                          EFI_FVB2_STICKY_WRITE       | // A block erase is required to flip bits into EFI_FVB2_ERASE_POLARITY
                                          EFI_FVB2_MEMORY_MAPPED      | // It is memory mapped
                                          EFI_FVB2_ERASE_POLARITY     | // After erasure all bits take this value (i.e. '1')
                                          EFI_FVB2_WRITE_STATUS       | // Writes are currently enabled
                                          EFI_FVB2_WRITE_ENABLED_CAP    // Writes may be enabled
                                      );
  FirmwareVolumeHeader->HeaderLength = sizeof(EFI_FIRMWARE_VOLUME_HEADER) + sizeof(EFI_FV_BLOCK_MAP_ENTRY);
  FirmwareVolumeHeader->Revision = EFI_FVH_REVISION;
  FirmwareVolumeHeader->BlockMap[0].NumBlocks = PcdGet32(PcdFlashNvStorageVariableSize)/EMMC_BLOCK_SIZE;
  FirmwareVolumeHeader->BlockMap[0].Length      = EMMC_BLOCK_SIZE;
  FirmwareVolumeHeader->BlockMap[1].NumBlocks = 0;
  FirmwareVolumeHeader->BlockMap[1].Length      = 0;
  FirmwareVolumeHeader->Checksum = CalculateCheckSum16 ((UINT16*)FirmwareVolumeHeader,FirmwareVolumeHeader->HeaderLength);

  //
  // VARIABLE_STORE_HEADER
  //
  VariableStoreHeader = (VARIABLE_STORE_HEADER*)((UINTN)Headers + FirmwareVolumeHeader->HeaderLength);
  CopyGuid (&VariableStoreHeader->Signature, &gEfiVariableGuid);
  VariableStoreHeader->Size = PcdGet32(PcdFlashNvStorageVariableSize) - FirmwareVolumeHeader->HeaderLength;
  VariableStoreHeader->Format            = VARIABLE_STORE_FORMATTED;
  VariableStoreHeader->State             = VARIABLE_STORE_HEALTHY;

  // Install the combined super-header in the NorFlash
  if(WorkMode != WORK_MODE_BOOT)
    CopyMem((VOID *)(UINTN)FlashNvStorageVariableBase, Headers,HeadersLength);
  else
    Status = FvbWrite (&gFvbProtocol, 0, 0, &HeadersLength, Headers);
  
  FreePool (Headers);
  return Status;
}

/**
  Initialize the FVB to use block IO 


  @retval EFI_SUCCESS           Protocol registered
  @retval EFI_DEVICE_ERROR      Hardware problems

**/
VOID
EFIAPI
BlockIONotificationEvent (
  IN  EFI_EVENT                           Event,
  IN  VOID                                *Context
  )
{
  EFI_HANDLE *HandleBuffer = NULL;
  EFI_STATUS Status;
  UINTN NumHandles;
  UINT32 i,j,TotalBlk,LeftBlk,ToWriteBlk;
  EFI_PHYSICAL_ADDRESS *FvbHeaderBuffer;
  CHAR8 *pDataBuff=NULL;
  EFI_LBA EnvPartAddr = 0, LogicOffset = 0;

  UINT32 StorageType, WorkMode;
  UINT32 EnvOffSet = 0;

  DEBUG((EFI_D_INFO, "FVB:BlockIONotificationEvent Start \n"));

  if(mHandle!=NULL)
  {
    DEBUG((EFI_D_INFO, "FVB:Fvb Protocol already exist \n"));
    return;
  }

  WorkMode = SunxiGetBootWorkMode();
  if(WorkMode == WORK_MODE_BOOT)
  {
    StorageType = SunxiGetBootStorage();
  }
  else 
  {
    StorageType = SunxiGetBurnStorage();
  }
    
  if(StorageType == 0)
  {
    TargetMediaId = SIGNATURE_32('n','a','n','d'); //nand not support  now,so use emmc default
    LogicOffset = 0;
  }
  else if(StorageType == 1)
  {
    TargetMediaId = SIGNATURE_32('s','d','h','c');
    LogicOffset = SUNXI_MBR_OFFSET;
  }
  else if(StorageType == 2)
  {
    TargetMediaId = SIGNATURE_32('e','m','m','c');
    LogicOffset = SUNXI_MBR_OFFSET;
  }
      
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &NumHandles, &HandleBuffer);   
  if (EFI_ERROR(Status))
  {
    DEBUG((EFI_D_INFO, "FVB: LocateHandleBuffer Fail\n"));
    return ;
  }
    
  DEBUG((EFI_D_INFO, "FVB:Device nand = 0x%x\n", (UINT32)SIGNATURE_32('n','a','n','d')));
  DEBUG((EFI_D_INFO, "FVB:Device sdhc = 0x%x\n", (UINT32)SIGNATURE_32('s','d','h','c')));
  DEBUG((EFI_D_INFO, "FVB:Device emmc = 0x%x\n", (UINT32)SIGNATURE_32('e','m','m','c')));
  DEBUG((EFI_D_INFO, "FVB:Target Device ID = 0x%x\n", TargetMediaId));
  DEBUG((EFI_D_INFO,"FVB: Total BlockIO Handles %d\n",NumHandles));
  for(i=0;i<NumHandles;i++) {
    Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiBlockIoProtocolGuid, (VOID **)&BlockIo);
    DEBUG((EFI_D_INFO, "FVB:Device %d Media ID=0x%x\n", i, BlockIo->Media->MediaId));
    if((BlockIo->Media->MediaId == TargetMediaId))
    {
      DEBUG((EFI_D_INFO, "FVB:Find BlockIo Portocol , DeviceID %x\n", BlockIo->Media->MediaId));
      break;
    }
  }
  gBS->FreePool (HandleBuffer);
  
  if(i == NumHandles) {
    Print(L"FVB: Cannot find Block IO protocol handle! \n");
    return ;
  }

  if(WorkMode == WORK_MODE_BOOT) {
    //--------------get env part info start-------------------
    pDataBuff = AllocatePool(SUNXI_MBR_SIZE);
    ASSERT(pDataBuff != NULL);

    //read mbr
    Status = FvbBlockIoRead(BlockIo,BlockIo->Media->MediaId, 
        LogicOffset+0,SUNXI_MBR_SIZE,pDataBuff);
    if(EFI_ERROR (Status))
    {
      DEBUG((EFI_D_INFO,"FVB:Read MBR Error\n"));
    }

    //suxi mbr init
    Status = SunxiPartitionInitialize(pDataBuff, SUNXI_MBR_SIZE);
    if(EFI_ERROR(Status))
    {
      DEBUG((EFI_D_ERROR,"FVB:sunxi_partition_init Error\n"));
    }
    //SunxiGetPartitionInfo(2,pDataBuff);
    Status = SunxiPartitionGetOffsetByName("env",&EnvOffSet);
    if(EFI_ERROR(Status))
    {
      DEBUG((EFI_D_ERROR,"FVB:Get Env Partition Error\n"));
    }
    EnvPartAddr =(EFI_LBA)EnvOffSet;

    //use 7<<20 ,because only erase 8M per partition when burn firmware.
    EMMC_NVRAM_BLOCK_OFFSET = EnvPartAddr + LogicOffset;//+ (7<<20)/512;

    FreePool(pDataBuff);
    DEBUG((EFI_D_INFO, "FVB:--------------env addr: 0x%x ----------------------\n", (UINTN)EMMC_NVRAM_BLOCK_OFFSET));
    //--------------get env part info end-------------------

    //read env.cfg from env partition . 
    Status = FvbBlockIoRead(BlockIo, BlockIo->Media->MediaId,  (EnvPartAddr + LogicOffset),
                 EMMC_BLOCK_SIZE*(SunxiVariableHob->SunxiEnvSize/EMMC_BLOCK_SIZE), (UINT8 *)((UINTN)SunxiVariableHob->SunxiEnvBase));
    ASSERT_EFI_ERROR (Status);

    ToWriteBlk = 128;
    FvbHeaderBuffer = AllocatePool(EMMC_BLOCK_SIZE*ToWriteBlk);

    SetMem(FvbHeaderBuffer,EMMC_BLOCK_SIZE*ToWriteBlk,0xff);

    //read one block from the emmc device to check if we have the right fvb header. 
    Status = FvbBlockIoRead(BlockIo, BlockIo->Media->MediaId, EMMC_NVRAM_BLOCK_OFFSET,EMMC_BLOCK_SIZE*1, (UINT8 *)FvbHeaderBuffer);

    ASSERT_EFI_ERROR (Status);
    //check if we already have the right nvram variable header.
    if(ValidateFvHeader((UINT32)FvbHeaderBuffer)==EFI_NOT_FOUND){
      TotalBlk = PcdGet32(PcdFlashNvStorageVariableSize)/EMMC_BLOCK_SIZE;
      LeftBlk = TotalBlk;

      //reset the RPMB partition to all one state.
      SetMem(FvbHeaderBuffer,EMMC_BLOCK_SIZE*ToWriteBlk,0xff);
      for(j=0;j<TotalBlk;){
        DEBUG((EFI_D_INFO, "FVB:init env blk to all one,left blk %x\n",LeftBlk));
        if(LeftBlk >= ToWriteBlk ) {
          Status = FvbBlockIoWrite(BlockIo, TargetMediaId, EMMC_NVRAM_BLOCK_OFFSET+j, EMMC_BLOCK_SIZE*ToWriteBlk, (VOID *)(FvbHeaderBuffer));
          ASSERT_EFI_ERROR (Status);
          LeftBlk -= ToWriteBlk;
          j += ToWriteBlk;
        }
        else
        {
          break;
        }
            
      }
      if(LeftBlk)
      {
        Status = FvbBlockIoWrite(BlockIo, TargetMediaId, EMMC_NVRAM_BLOCK_OFFSET+j, EMMC_BLOCK_SIZE*LeftBlk, (VOID *)(FvbHeaderBuffer));
        ASSERT_EFI_ERROR (Status);
      }
  
      //not find the FVB header, so install a new one onto the emmc RPMB partition.
      InitializeFvAndVariableStoreHeaders(WorkMode);
      SunxiVariableHob->SunxiEnvNeedImport = 1;
    }
    else
    {
      ZeroMem((UINT8 *)((UINTN)SunxiVariableHob->SunxiEnvBase),SunxiVariableHob->SunxiEnvSize);
      SunxiVariableHob->SunxiEnvNeedImport = 2;
    }

    FreePool(FvbHeaderBuffer);
    //read the nvram variable from the emmc device to dram FVB for sync purpose. 
    Status = FvbBlockIoRead(BlockIo, BlockIo->Media->MediaId, EMMC_NVRAM_BLOCK_OFFSET,PcdGet32(PcdFlashNvStorageVariableSize), (VOID *)(UINTN)FlashNvStorageVariableBase);

    ASSERT_EFI_ERROR (Status);
  }
  else
  {
    InitializeFvAndVariableStoreHeaders(WorkMode);
  }

  Status =  gBS->InstallMultipleProtocolInterfaces (
                &mHandle,
                &gEfiFirmwareVolumeBlockProtocolGuid,   &gFvbProtocol,
                NULL
                );

  DEBUG((EFI_D_INFO, "FVB:InstallFVBProtocol done \n"));
  if(Status!=EFI_SUCCESS)
  {
    DEBUG((EFI_D_ERROR, "FVB:BlockIO handle is not valid %r\n", Status));
  }

}



/**
  Initialize the state information for the CPU Architectural Protocol

  @param  ImageHandle   of the loaded driver
  @param  SystemTable   Pointer to the System Table

  @retval EFI_SUCCESS           Protocol registered
  @retval EFI_OUT_OF_RESOURCES  Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR      Hardware problems

**/
EFI_STATUS
EFIAPI
FvbDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status=EFI_SUCCESS;



  SunxiVariableHob = GetFirstGuidHob (&gSunxiVariableGuid);
  if (SunxiVariableHob == NULL) {
    DEBUG((DEBUG_WARN, "--%s:No Sunxi Variable Guid found, this protocol will not install. \n",__FUNCTION__));
    return EFI_NOT_FOUND;
  }

  FlashNvStorageVariableBase = SunxiVariableHob->SunxiVariableBase;

  //BlockIo = AllocatePool(sizeof(EFI_BLOCK_IO_PROTOCOL)+sizeof(EFI_BLOCK_IO_MEDIA));

  //
  // Register FvbNotificationEvent () notify function.
  // 
  EfiCreateProtocolNotifyEvent (
    &gEfiBlockIoProtocolGuid,
    TPL_CALLBACK,
    BlockIONotificationEvent,
    (VOID *)SystemTable,
    &mBlockIORegistration
  );
  
  DEBUG ((EFI_D_INFO, "\nFVB:FvbDxeInitialize\n"));

  // SetVertAddressEvent ()

  // GCD Map NAND as RT

  return Status;
}

