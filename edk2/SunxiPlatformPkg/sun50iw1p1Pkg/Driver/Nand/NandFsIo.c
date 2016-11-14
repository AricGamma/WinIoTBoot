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

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/Cpu.h>
#include <Protocol/SunxiFlashIo.h>

#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Library/SunxiMbr.h>
#include <Library/SunxiPartitionLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>


#include <IndustryStandard/Mbr.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiCheckLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include "UserDef.h"




extern  EFI_GUID gSunxiFsDevicePathGuid;



static UINT32                 FileSystemBase = 0;
static UINT32                 FileSyStemSize = 0;
static EFI_HANDLE             SunxiFsBlockHandle = NULL;


EFI_BLOCK_IO_MEDIA gFsNand= {
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


NAND_DEVICE_PATH FsNandDevicePath = {
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



extern EFI_STATUS
NandReadWrite (
  IN EFI_BLOCK_IO_PROTOCOL    *This,
  IN  UINTN                   Lba,
  OUT VOID                    *Buffer,
  IN  UINTN                   BufferSize,
  IN  OPERATION_TYPE          OperationType
  );
  
extern int NAND_FlushCache(void);



EFI_STATUS
EFIAPI
NandFsReset (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN BOOLEAN                        ExtendedVerification
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
NandFsReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  OUT VOID                          *Buffer
  )
{
  EFI_STATUS Status;
  Status = NandReadWrite (This, (UINTN)Lba+FileSystemBase, Buffer, BufferSize, READ);
  return Status;

}

EFI_STATUS
EFIAPI
NandFsWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  IN VOID                           *Buffer
  )
{
  EFI_STATUS  Status;
  Status = NandReadWrite (This, (UINTN)Lba+FileSystemBase, Buffer, BufferSize, WRITE);

  return Status;

}


EFI_STATUS
EFIAPI
NandFsFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
{
  if( 0 != NAND_FlushCache())
  {
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;

}


EFI_STATUS SunxiGetFsOffset ( EFI_BLOCK_IO_PROTOCOL* BlockIo )

{
  EFI_STATUS  Status=EFI_SUCCESS;
  CHAR8 *pDataBuff=NULL;
  UINT32 BootFsOffSet = 0;

  //--------------get env part info start-------------------
  pDataBuff = AllocatePool(SUNXI_MBR_SIZE);
  ASSERT(pDataBuff != NULL);

  //read mbr
  Status = BlockIo->ReadBlocks(BlockIo,BlockIo->Media->MediaId,0,SUNXI_MBR_SIZE,pDataBuff);
  if(EFI_ERROR (Status))
  {
    DEBUG((EFI_D_INFO,"FsBlockIo:Read MBR Error\n"));
    goto __END;
  }

  //suxi mbr init
  Status = SunxiPartitionInitialize(pDataBuff, SUNXI_MBR_SIZE);
  if(EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR,"FsBlockIo:sunxi_partition_init Error\n"));
    goto __END;
  }
  //SunxiGetPartitionInfo(2,pDataBuff);
  Status = SunxipartitionGetOffset(0,&BootFsOffSet);
  if(EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR,"FsBlockIo:Get  boot-resource Partition Offset Error\n"));
    goto __END;
  }
  FileSystemBase = BootFsOffSet;

  Status = SunxiPartitionGetSize(0,&FileSyStemSize);
  if(EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR,"FsBlockIo:Get Eboot-resource Partition Size Error\n"));
    goto __END;
  }
  DEBUG((EFI_D_INFO,"FsBlockIo: FileSystem offset = 0x%x, size = 0x%x sectors\n", FileSystemBase,FileSyStemSize ));
  
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
InstallNandFsIoProtocol( EFI_BLOCK_IO_PROTOCOL* pBlockIo)
{
  EFI_BLOCK_IO_PROTOCOL* pFsBlockIo;
  NAND_DEVICE_PATH* pFsDevicePath;
  EFI_STATUS Status;

  Status = SunxiGetFsOffset(pBlockIo);
  if(EFI_ERROR(Status))
  {
    return Status;
  }

  pFsBlockIo = (EFI_BLOCK_IO_PROTOCOL*)AllocateZeroPool(sizeof(EFI_BLOCK_IO_PROTOCOL));
  pFsBlockIo->ReadBlocks = NandFsReadBlocks;
  pFsBlockIo->WriteBlocks= NandFsWriteBlocks;
  pFsBlockIo->FlushBlocks= NandFsFlushBlocks;
  pFsBlockIo->Reset    = NandFsReset;

  pFsDevicePath = (NAND_DEVICE_PATH*)AllocateZeroPool(sizeof(NAND_DEVICE_PATH));
  CopyMem(pFsDevicePath,&FsNandDevicePath,sizeof(NAND_DEVICE_PATH));

  pFsBlockIo->Media = &gFsNand; 
  CopyGuid(&pFsDevicePath->Nand.Guid,&gSunxiFsDevicePathGuid);

  pFsBlockIo->Media->MediaPresent=TRUE;
  pFsBlockIo->Media->LastBlock = FileSystemBase+FileSyStemSize-1;

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

