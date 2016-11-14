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
//#include <mmc_def.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SunxiFlashIo.h>


#include <Library/SysConfigLib.h>
#include <Library/NandLib.h>

#include <Library/SunxiPartitionLib.h>
#include <Library/SunxiMbr.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiBootInfoLib.h>

#include "NandExt.h"
#include "UserDef.h"

extern int NAND_LogicInit(int boot_mode);
extern int NAND_LogicRead(uint nSectNum, uint nSectorCnt, void * pBuf);
extern int NAND_LogicWrite(uint nSectNum, uint nSectorCnt, void * pBuf);
extern int NAND_UbootProbe(void);
extern int NAND_UbootInit(int boot_mode);
extern int NAND_FlushCache(void);


extern EFI_STATUS
InstallNandFlashIoProtocol(
  EFI_HANDLE* NandBlockIOHandle,
  EFI_BLOCK_IO_PROTOCOL* BlockIo
  );
  


// {3786D853-22E4-DFD9-9C20-494EA8FFDEB2}
extern  EFI_GUID gNandDevicePathGuid;



EFI_BLOCK_IO_MEDIA gNandDev = {
  SIGNATURE_32('n','a','n','d'),            // MediaId
  TRUE,                                     // RemovableMedia
  FALSE,                                    // MediaPresent
  FALSE,                                    // LogicalPartition
  FALSE,                                    // ReadOnly
  FALSE,                                    // WriteCaching
  512,                                      // BlockSize
  4,                                        // IoAlign
  0,                                        // Pad
  0                                         // LastBlock
};


NAND_DEVICE_PATH NandDevicePath = {
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
NandReadWrite (
  IN EFI_BLOCK_IO_PROTOCOL    *This,
  IN  UINTN                   Lba,
  OUT VOID                    *Buffer,
  IN  UINTN                   BufferSize,
  IN  OPERATION_TYPE          OperationType
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINTN      BlockCount;
  EFI_TPL    OldTpl;

  if(GetNandOpenedCnt() == 0)
  {
    Print(L"Nand not Opened,Do nothing\n");
    return EFI_SUCCESS;
  }

  if (Buffer == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (Lba > This->Media->LastBlock) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if ((BufferSize % This->Media->BlockSize) != 0) {
    Status = EFI_BAD_BUFFER_SIZE;
    goto Done;
  }

  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  BlockCount = BufferSize/This->Media->BlockSize;
  if(OperationType ==READ){
    if(This->Media->MediaId==SIGNATURE_32('n','a','n','d') || 
            This->Media->MediaId==SIGNATURE_32('a','w','f','s') )
    {
      if(0 != NAND_LogicRead((UINT32)Lba,BlockCount, Buffer)){
        Status = EFI_DEVICE_ERROR;
      }
    }
  }
  
  if(OperationType ==WRITE){
    if(This->Media->MediaId==SIGNATURE_32('n','a','n','d') ||
             This->Media->MediaId==SIGNATURE_32('a','w','f','s') )
    {
      if(0 != NAND_LogicWrite((UINT32)Lba,BlockCount, Buffer)){
        Status = EFI_DEVICE_ERROR;
      }
    }
    else
    {
      Print(L"The required media doesn't exist\n");
      Status = EFI_NO_MEDIA;
      goto DoneRestoreTPL;
    }

  }
    
  if (EFI_ERROR(Status)) {
    Print(L"TransferBlockData fails. %x\n", Status);
    goto DoneRestoreTPL;
  }

DoneRestoreTPL:

  gBS->RestoreTPL (OldTpl);

Done:

  return Status;

}


/**

  Reset the Block Device.



  @param  This                 Indicates a pointer to the calling context.

  @param  ExtendedVerification Driver may perform diagnostics on reset.



  @retval EFI_SUCCESS          The device was reset.

  @retval EFI_DEVICE_ERROR     The device is not functioning properly and could

                               not be reset.



**/
EFI_STATUS
EFIAPI
NandReset (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN BOOLEAN                        ExtendedVerification
  )
{
  DEBUG ((EFI_D_INFO, " Nand::NandReset is called\n"));
  return EFI_SUCCESS;
}


/**

  Read BufferSize bytes from Lba into Buffer.



  @param  This       Indicates a pointer to the calling context.

  @param  MediaId    Id of the media, changes every time the media is replaced.

  @param  Lba        The starting Logical Block Address to read from

  @param  BufferSize Size of Buffer, must be a multiple of device block size.

  @param  Buffer     A pointer to the destination buffer for the data. The caller is

                     responsible for either having implicit or explicit ownership of the buffer.



  @retval EFI_SUCCESS           The data was read correctly from the device.

  @retval EFI_DEVICE_ERROR      The device reported an error while performing the read.

  @retval EFI_NO_MEDIA          There is no media in the device.

  @retval EFI_MEDIA_CHANGED     The MediaId does not matched the current device.

  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.

  @retval EFI_INVALID_PARAMETER The read request contains LBAs that are not valid,

                                or the buffer is not on proper alignment.

EFI_STATUS

**/
EFI_STATUS
EFIAPI
NandReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  OUT VOID                          *Buffer
  )
{
  EFI_STATUS Status;

//  DEBUG ((EFI_D_INFO, "SDHC::SDHCReadBlocks : MediaId = %d, Lba = %d, BufferSize = %d, Buffer = 0x%x\n",
//          MediaId, (UINTN)Lba, BufferSize, Buffer));

  Status = NandReadWrite (This, (UINTN)Lba, Buffer, BufferSize, READ);

  return Status;

}


/**

  Write BufferSize bytes from Lba into Buffer.



  @param  This       Indicates a pointer to the calling context.

  @param  MediaId    The media ID that the write request is for.

  @param  Lba        The starting logical block address to be written. The caller is

                     responsible for writing to only legitimate locations.

  @param  BufferSize Size of Buffer, must be a multiple of device block size.

  @param  Buffer     A pointer to the source buffer for the data.



  @retval EFI_SUCCESS           The data was written correctly to the device.

  @retval EFI_WRITE_PROTECTED   The device can not be written to.

  @retval EFI_DEVICE_ERROR      The device reported an error while performing the write.

  @retval EFI_NO_MEDIA          There is no media in the device.

  @retval EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.

  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.

  @retval EFI_INVALID_PARAMETER The write request contains LBAs that are not valid,

                                or the buffer is not on proper alignment.



**/
EFI_STATUS
EFIAPI
NandWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  IN VOID                           *Buffer
  )
{
  EFI_STATUS  Status;

//  DEBUG ((EFI_D_INFO, "SDHC::SDHCWriteBlocks : MediaId = %d, Lba = %d, BufferSize = %d, Buffer = 0x%x\n",
//          MediaId, (UINTN)Lba, BufferSize, Buffer));
  //Perform write operation.
  Status = NandReadWrite (This, (UINTN)Lba, Buffer, BufferSize, WRITE);

  return Status;

}


/**

  Flush the Block Device.



  @param  This              Indicates a pointer to the calling context.



  @retval EFI_SUCCESS       All outstanding data was written to the device

  @retval EFI_DEVICE_ERROR  The device reported an error while writting back the data

  @retval EFI_NO_MEDIA      There is no media in the device.



**/
EFI_STATUS
EFIAPI
NandFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
{
 // DEBUG ((EFI_D_INFO, "SDHC::SDHCFlushBlocks is called\n"));
  if( 0 != NAND_FlushCache())
  {
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}



EFI_STATUS
InstallNandFsIoProtocol( EFI_BLOCK_IO_PROTOCOL* pBlockIo);

extern int NAND_Print(const CHAR8 * str, ...);

EFI_HANDLE gNandHandle = NULL;

EFI_STATUS
SunxiNandDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{


  EFI_STATUS    Status;
  INT32 Lba;
  EFI_BLOCK_IO_PROTOCOL* pNandBlockIo;
  NAND_DEVICE_PATH* pNandDevicePath;

  SUNXI_FLASH_IO_PROTOCOL *FlashIo =NULL;

  //detect nand flash by eMMC BlockIO , Because NandProbe function will change io which will cause emmc error.
  if(WORK_MODE_BOOT == SunxiGetBootWorkMode())
  {
    if(0 != SunxiGetBootStorage())
    {
      return EFI_SUCCESS;
    }

    if(Nand_Uefi_Init(1) != 0 )
    {
      Print(L"Init the Nand Flash failed!\n");
      return EFI_DEVICE_ERROR;
    }
    Lba = (INT32)get_nftl_cap();
  }
  else 
  {
    //get FlashIO protocol
    Status = gBS->LocateProtocol(&gSunxiFlashIoProtocolGuid, NULL, (VOID **)&FlashIo);
    if (!EFI_ERROR (Status)) 
    {
      DEBUG((EFI_D_INFO, "NandDxe:Find FlashIo Portocol , So Maybe Emmc exist\n"));
      return EFI_SUCCESS;
    }
    //probe nand
    if(0 != NAND_UbootProbe())
    {
      DEBUG((EFI_D_INFO, "NandDxe:probe nand fail, SSo Maybe Emmc exist\n"));
      return EFI_SUCCESS;
    }
    Lba = 1;

  }
  
  pNandBlockIo = (EFI_BLOCK_IO_PROTOCOL*)AllocateZeroPool(sizeof(EFI_BLOCK_IO_PROTOCOL));
  pNandBlockIo->ReadBlocks = NandReadBlocks;
  pNandBlockIo->WriteBlocks= NandWriteBlocks;
  pNandBlockIo->FlushBlocks= NandFlushBlocks;
  pNandBlockIo->Reset   = NandReset;

  pNandDevicePath = (NAND_DEVICE_PATH*)AllocateZeroPool(sizeof(NAND_DEVICE_PATH));
  CopyMem(pNandDevicePath,&NandDevicePath,sizeof(NAND_DEVICE_PATH));
  
  pNandBlockIo->Media = &gNandDev;
  CopyGuid(&pNandDevicePath->Nand.Guid,&gNandDevicePathGuid);
  
  pNandBlockIo->Media->MediaPresent=TRUE;
  pNandBlockIo->Media->LastBlock = Lba-1;

  //Publish FlashIO. because Fvb will Run after BlockIO Publish,so FlashIO should be Install first.
  SunxiSetBurnStorage(SunxiBurnStorageNand);
  Status = InstallNandFlashIoProtocol(&gNandHandle, pNandBlockIo);
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_INFO, "Install FlashIo Protocol For Nand Flash Success!\n"));
    
  //Publish BlockIO.
  Status = gBS->InstallMultipleProtocolInterfaces (
            &gNandHandle,
            &gEfiBlockIoProtocolGuid,    pNandBlockIo,
            &gEfiDevicePathProtocolGuid, pNandDevicePath,
            NULL
            );
  
  ASSERT_EFI_ERROR (Status);

  if(WORK_MODE_BOOT == SunxiGetBootWorkMode())
  {
    DEBUG ((EFI_D_INFO, "Install NandFs Io Protocol For Nand Flash start!\n"));
    Status = InstallNandFsIoProtocol(pNandBlockIo);
    if(!EFI_ERROR (Status))
    {
      DEBUG ((EFI_D_INFO, "Install NandFs Io Protocol For Nand Flash Success!\n"));
    }
  }
  return EFI_SUCCESS;
}

