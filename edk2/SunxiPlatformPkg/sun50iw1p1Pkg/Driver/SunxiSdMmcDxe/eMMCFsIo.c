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
#include <mmc_def.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/Cpu.h>
#include <Protocol/SunxiFlashIo.h>

#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Library/SunxiMbr.h>
#include <Library/SunxiPartitionLib.h>

#include <IndustryStandard/Mbr.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiCheckLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include "UserDef.h"




extern  EFI_GUID gSunxiFsDevicePathGuid;



static UINT32                 FileSystemBase = 0;
UINT32                        gFileSyStemSize = 0;
EFI_HANDLE                    SunxiFsBlockHandle = NULL;


EFI_BLOCK_IO_MEDIA gFsSdMmc2 = {
  SIGNATURE_32('a','w','f','s'),            // MediaId
  FALSE,                                     // RemovableMedia
  FALSE,                                    // MediaPresent
  FALSE,                                    // LogicalPartition
  FALSE,                                    // ReadOnly
  FALSE,                                    // WriteCaching
  512,                                      // BlockSize
  4,                                        // IoAlign
  0,                                        // Pad
  0                                         // LastBlock
};


SDHC_DEVICE_PATH FsSdMmcDevicePath = {
  {
    HARDWARE_DEVICE_PATH,
    HW_VENDOR_DP,
    (UINT8)(sizeof(VENDOR_DEVICE_PATH)),
    (UINT8)((sizeof(VENDOR_DEVICE_PATH)) >> 8),
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    sizeof (EFI_DEVICE_PATH_PROTOCOL),
    0
  }
};



EFI_STATUS
SdReadWrite (
  IN EFI_BLOCK_IO_PROTOCOL    *This,
  IN  UINTN                   Lba,
  OUT VOID                    *Buffer,
  IN  UINTN                   BufferSize,
  IN  OPERATION_TYPE          OperationType
  );


EFI_STATUS
EFIAPI
SdMmcFsReset (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN BOOLEAN                        ExtendedVerification
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SdMmcFsReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  OUT VOID                          *Buffer
  )
{
  EFI_STATUS Status;
  Status = SdReadWrite (This, (UINTN)Lba+FileSystemBase, Buffer, BufferSize, READ);
  return Status;
}

EFI_STATUS
EFIAPI
SdMmcFsWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  IN VOID                           *Buffer
  )
{
  EFI_STATUS  Status;
  Status = SdReadWrite (This, (UINTN)Lba+FileSystemBase, Buffer, BufferSize, WRITE);

  return Status;
}

EFI_STATUS
EFIAPI
SdMmcFsFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS SunxiGetFsOffset ( EFI_BLOCK_IO_PROTOCOL* BlockIo )

{
  EFI_STATUS  Status=EFI_SUCCESS;
  CHAR8 *pDataBuff=NULL;
  UINT32 BootFsOffSet = 0;
  UINT32 BlockSize;

  BlockSize = BlockIo->Media->BlockSize;
  //--------------get env part info start-------------------
  pDataBuff = AllocatePool(BlockSize);
  ASSERT(pDataBuff != NULL);

   //read gpt
  Status = BlockIo->ReadBlocks(BlockIo,BlockIo->Media->MediaId,
  SUNXI_GPT_PRIMARY_HEADER_LBA,BlockSize,pDataBuff);
  if(EFI_ERROR (Status))
  {
    DEBUG((EFI_D_INFO,"FsBlockIo:Read MBR Error\n"));
    goto __END;
  }

  //suxi gpt init
  Status = SunxiGptPartitionInitialize(BlockIo, pDataBuff, BlockSize);
  if(EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR,"FsBlockIo:sunxi_partition_init Error\n"));
    goto __END;
  }
  //SunxiGetPartitionInfo(2,pDataBuff);
  Status = SunxiGptPartitionGetOffset(SUNXI_USB_MASS_GPT_PART_NUM,&BootFsOffSet);
  if(EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR,"FsBlockIo:Get Env Partition Error\n"));
    goto __END;
  }
  FileSystemBase = BootFsOffSet+MMC_LOGICAL_OFFSET;

  Status = SunxiGptPartitionGetSize(SUNXI_USB_MASS_GPT_PART_NUM,&gFileSyStemSize);
  if(EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR,"FsBlockIo:Get Env Partition Error\n"));
    goto __END;
  }
  if(gFileSyStemSize == 0)
  {
    //the size of udisk partition is 0, so need set this value. 
    gFileSyStemSize =  BlockIo->Media->LastBlock - FileSystemBase-MMC_LOGICAL_OFFSET;
  }
  DEBUG((EFI_D_INFO,"FsBlockIo: FileSystem offset = 0x%x, size = 0x%x sectors\n", FileSystemBase,gFileSyStemSize ));
  
__END:
  if(pDataBuff)
  {
    FreePool(pDataBuff);
  }
  return Status;
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
InstallMMCFsIoProtocol( EFI_BLOCK_IO_PROTOCOL* pSdMmcBlockIo)
{
  EFI_BLOCK_IO_PROTOCOL* pFsBlockIo;
  SDHC_DEVICE_PATH* pFsDevicePath;
  EFI_STATUS Status;

  Status = SunxiGetFsOffset(pSdMmcBlockIo);
  if(EFI_ERROR(Status))
  {
    return Status;
  }

  pFsBlockIo = (EFI_BLOCK_IO_PROTOCOL*)AllocateZeroPool(sizeof(EFI_BLOCK_IO_PROTOCOL));
  pFsBlockIo->ReadBlocks = SdMmcFsReadBlocks;
  pFsBlockIo->WriteBlocks= SdMmcFsWriteBlocks;
  pFsBlockIo->FlushBlocks= SdMmcFsFlushBlocks;
  pFsBlockIo->Reset   = SdMmcFsReset;

  pFsDevicePath = (SDHC_DEVICE_PATH*)AllocateZeroPool(sizeof(SDHC_DEVICE_PATH));
  CopyMem(pFsDevicePath,&FsSdMmcDevicePath,sizeof(SDHC_DEVICE_PATH));

  pFsBlockIo->Media = &gFsSdMmc2; 
  CopyGuid(&pFsDevicePath->Mmc.Guid,&gSunxiFsDevicePathGuid);

  pFsBlockIo->Media->MediaPresent=TRUE;
  pFsBlockIo->Media->LastBlock = gFileSyStemSize-1;

  //Publish BlockIO.
  Status = gBS->InstallMultipleProtocolInterfaces (
    &SunxiFsBlockHandle,
    &gEfiBlockIoProtocolGuid,    pFsBlockIo,
    &gEfiDevicePathProtocolGuid, pFsDevicePath,
    NULL
    );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

