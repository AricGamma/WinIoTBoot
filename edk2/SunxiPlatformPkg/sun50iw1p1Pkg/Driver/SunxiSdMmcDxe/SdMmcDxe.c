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
#include "mmc.h"

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/Cpu.h>
#include <Protocol/SunxiFlashIo.h>

#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Library/SysConfigLib.h>
#include <Library/SunxiBootInfoLib.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiBootInfoLib.h>

#include <mmc_op.h>
#include "UserDef.h"



// {76B35BB5-02EB-47CF-BA85-79EFAC1B610F}
extern  EFI_GUID gSdMmc0DevicePathGuid;

//{B94253B2-B71E-4E8F-93C7-CAC212021C53}
extern  EFI_GUID gSdMmc2DevicePathGuid;


EFI_BLOCK_IO_MEDIA gSdMmc0 = {
  SIGNATURE_32('s','d','h','c'),            // MediaId
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

EFI_BLOCK_IO_MEDIA gSdMmc2 = {
  SIGNATURE_32('e','m','m','c'),            // MediaId
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


SDHC_DEVICE_PATH SdMmcDevicePath = {
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

EFI_HANDLE gSdMmcHandleArray[MAX_MMC_NUM];

static EFI_EVENT                  gCard0DetectTimerEvent;
static UINT32             gCard0DetectGpioHandle = 0;
static EFI_LOCK           gCard0DetectLock;
extern UINT32                     gFileSyStemSize;
EFI_STATUS
SdReadWrite (
  IN EFI_BLOCK_IO_PROTOCOL    *This,
  IN  UINTN                   Lba,
  OUT VOID                    *Buffer,
  IN  UINTN                   BufferSize,
  IN  OPERATION_TYPE          OperationType
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINTN      BlockCount;
  UINTN      CardNumber;
  EFI_TPL    OldTpl;

  if (Buffer == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if(This->Media->MediaId!=SIGNATURE_32('a','w','f','s')){
    
    if (Lba > This->Media->LastBlock) {
      AsciiPrint("Lba(0x%x) is bigger than LastBlock(0x%x).\n",Lba,This->Media->LastBlock);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    } 
  }
  else
  {
    if (Lba > This->Media->LastBlock+gFileSyStemSize) {
      AsciiPrint("Lba(0x%x) is bigger than LastBlock(0x%x).\n",Lba,This->Media->LastBlock);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
    } 
  }
    
  if ((BufferSize % This->Media->BlockSize) != 0) {
    Status = EFI_BAD_BUFFER_SIZE;
    goto Done;
  }

  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);
  BlockCount = BufferSize/This->Media->BlockSize;

  if(This->Media->MediaId==SIGNATURE_32('s','d','h','c')){
    CardNumber = 0;
  }
  else if(This->Media->MediaId==SIGNATURE_32('e','m','m','c') || 
        This->Media->MediaId==SIGNATURE_32('a','w','f','s')){
    CardNumber = 2;
  }else
  {
    AsciiPrint("The required media doesn't exist\n");
    Status = EFI_NO_MEDIA;
    goto DoneRestoreTPL;
  }
    
  if(OperationType ==READ){
    if(0>SDMMC_LogicalRead((UINT32)Lba,BlockCount, Buffer, CardNumber)){
      Status = EFI_DEVICE_ERROR;
    }
  }
  
  if(OperationType ==WRITE){  
    if(0>SDMMC_LogicalWrite ((UINT32)Lba,BlockCount, Buffer, CardNumber)){
      Status = EFI_DEVICE_ERROR;
    }
  }

  if (EFI_ERROR(Status)) {
    AsciiPrint("TransferBlockData fails. %x\n", Status);
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
SdMmcReset (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN BOOLEAN                        ExtendedVerification
  )
{
  DEBUG ((EFI_D_INFO, " SDHC::SDHCReset is called\n"));
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
SdMmcReadBlocks (
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

  Status = SdReadWrite (This, (UINTN)Lba, Buffer, BufferSize, READ);

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
SdMmcWriteBlocks (
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
  Status = SdReadWrite (This, (UINTN)Lba, Buffer, BufferSize, WRITE);

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
SdMmcFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
{
 // DEBUG ((EFI_D_INFO, "SDHC::SDHCFlushBlocks is called\n"));
  return EFI_SUCCESS;
}




STATIC EFI_CPU_ARCH_PROTOCOL  *Cpu;

void OSAL_CacheRangeFlush(void*Address, UINT32 Length, UINT32 Flags)
{
  if(Address == NULL || Length == 0)
  {
    return;
  }
    
  switch(Flags)
  {
    case CACHE_FLUSH_I_CACHE_REGION:
      break;
        
    case CACHE_FLUSH_D_CACHE_REGION:
      Cpu->FlushDataCache(Cpu,(EFI_PHYSICAL_ADDRESS)(UINT32)(Address),(UINT64)Length,EfiCpuFlushTypeWriteBack);
      break;
        
    case CACHE_FLUSH_CACHE_REGION:
      break;
          
    case CACHE_CLEAN_D_CACHE_REGION:
      Cpu->FlushDataCache(Cpu,(EFI_PHYSICAL_ADDRESS)(UINT32)(Address),(UINT64)Length,EfiCpuFlushTypeInvalidate);
      break;
          
    case CACHE_CLEAN_FLUSH_D_CACHE_REGION:
      Cpu->FlushDataCache(Cpu,(EFI_PHYSICAL_ADDRESS)(UINT32)(Address),(UINT64)Length,EfiCpuFlushTypeWriteBackInvalidate);
      break;        
        
    case CACHE_CLEAN_FLUSH_CACHE_REGION:  
      break;
        
    default:
      break;
  }
      
  return;
}


BOOLEAN
Card0Present (
  VOID
  )
{
  INT32     GpioVaule;

  GpioVaule = gpio_read_one_pin_value(gCard0DetectGpioHandle,NULL);
  if(GpioVaule<0) ASSERT(FALSE);

  return GpioVaule == 1 ? FALSE:TRUE;
}

EFI_STATUS
DetectCard0 (
  VOID
  )
{
  INT32 Lba;
  EFI_STATUS Status = EFI_SUCCESS;
  static UINT32 DetectCount=0;
  
  if (!Card0Present ()) {
    DetectCount = 0;
    return EFI_NO_MEDIA;
  }
  DetectCount++;
  if(DetectCount>=3){
    Lba=SDMMC_LogicalInit(0,0);
    if(Lba<0)
    {
      gSdMmc0.MediaPresent = FALSE;
      gSdMmc0.LastBlock    = 0;
      Status =EFI_DEVICE_ERROR;
    }else
    {
      gSdMmc0.LastBlock    = (Lba - 1);
      gSdMmc0.MediaPresent = TRUE; 
      Status =EFI_SUCCESS;
    }  
    gSdMmc0.BlockSize    = 512;
    gSdMmc0.ReadOnly     = FALSE;
    gSdMmc0.MediaId    =SIGNATURE_32('s','d','h','c'); 
    DetectCount  = 0;
  }else
  {
    Status =EFI_NO_MEDIA;
  }
  return Status;
}


extern EFI_STATUS
InstallMMCFlashIoProtocol(
  EFI_HANDLE* eMMCBlockIOHandle,
  EFI_BLOCK_IO_PROTOCOL* BlockIo
);

extern EFI_STATUS
InstallMMCFsIoProtocol( EFI_BLOCK_IO_PROTOCOL* pSdMmcBlockIo);

VOID
EFIAPI
Card0DetectTimerCallback (
  IN  EFI_EVENT   Event,
  IN  VOID        *Context
  )
{
  BOOLEAN Present;
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_BLOCK_IO_PROTOCOL* pSdMmcBlockIo;
  SDHC_DEVICE_PATH* pSdMmcDevicePath;
  SUNXI_FLASH_IO_PROTOCOL *FlashIo = NULL;

  Status = EfiAcquireLockOrFail(&gCard0DetectLock);
  if(Status==EFI_ACCESS_DENIED)
  {
    AsciiPrint("Card0 state detect Access denied\n");
    return;
  }
  
  Present = Card0Present ();
  if (gSdMmc0.MediaPresent) {
    if (!Present) { 
      Status = gBS->HandleProtocol(gSdMmcHandleArray[0],&gEfiBlockIoProtocolGuid,(VOID **)&pSdMmcBlockIo);
      if(EFI_ERROR (Status)) {
        AsciiPrint("HandleProtocol for SdMmc0 error\n");
      }

      Status = gBS->HandleProtocol(gSdMmcHandleArray[0],&gEfiDevicePathProtocolGuid,(VOID **)&pSdMmcDevicePath);
      if(EFI_ERROR (Status)) {
        AsciiPrint("HandleProtocol for SdMmc0 error\n");
      }

      Status = gBS->HandleProtocol(gSdMmcHandleArray[0],&gSunxiFlashIoProtocolGuid,(VOID **)&FlashIo);
      if(EFI_ERROR (Status)) {
        AsciiPrint("HandleProtocol for SdMmc0 error\n");
      }
      // Uninstall Protocol Interfaces
      Status = gBS->UninstallMultipleProtocolInterfaces (
        gSdMmcHandleArray[0],
        &gEfiBlockIoProtocolGuid,pSdMmcBlockIo,
        &gEfiDevicePathProtocolGuid,pSdMmcDevicePath,
        &gSunxiFlashIoProtocolGuid,FlashIo,
        NULL
        );
      FreePool(pSdMmcBlockIo);
      FreePool(pSdMmcDevicePath);
      FreePool(FlashIo);
      gSdMmc0.MediaPresent = FALSE;
      SDMMC_LogicalExit(0);
      AsciiPrint("\nCard0 removed!\n");
    }
  } else {
    if (Present) {   
      Status = DetectCard0();
      if(EFI_ERROR (Status)){
        EfiReleaseLock(&gCard0DetectLock);
        return;
      }
     
      pSdMmcBlockIo = (EFI_BLOCK_IO_PROTOCOL*)AllocateZeroPool(sizeof(EFI_BLOCK_IO_PROTOCOL));
      pSdMmcBlockIo->ReadBlocks = SdMmcReadBlocks;
      pSdMmcBlockIo->WriteBlocks= SdMmcWriteBlocks;
      pSdMmcBlockIo->FlushBlocks= SdMmcFlushBlocks;
      pSdMmcBlockIo->Reset    = SdMmcReset;
      pSdMmcBlockIo->Media = &gSdMmc0;

      pSdMmcDevicePath = (SDHC_DEVICE_PATH*)AllocateZeroPool(sizeof(SDHC_DEVICE_PATH));
      CopyMem(pSdMmcDevicePath,&SdMmcDevicePath,sizeof(SDHC_DEVICE_PATH));
      CopyGuid(&pSdMmcDevicePath->Mmc.Guid,&gSdMmc0DevicePathGuid);
      gSdMmcHandleArray[0] = 0;

      Status = gBS->LocateProtocol(&gSunxiFlashIoProtocolGuid, NULL, (VOID **)&FlashIo);
      if (!EFI_ERROR (Status)) 
      {
        DEBUG((EFI_D_INFO, "SdMMCDxe:Find FlashIo Portocol , So eMMC or Nand Flash Exist\n"));
      }
      else
      {
        AsciiPrint("\nInstall flash protocol for sd card\n");
        SunxiSetBurnStorage(SunxiBurnStorageSDHC);
        InstallMMCFlashIoProtocol(&gSdMmcHandleArray[0],pSdMmcBlockIo);
      }
      //Publish BlockIO.
      Status = gBS->InstallMultipleProtocolInterfaces (
                  &gSdMmcHandleArray[0],
                  &gEfiBlockIoProtocolGuid,    pSdMmcBlockIo,
                  &gEfiDevicePathProtocolGuid, pSdMmcDevicePath,
                  NULL
                  );
      ASSERT_EFI_ERROR (Status);
      AsciiPrint("\nCard0 insert!\n");
    }
  }
  EfiReleaseLock(&gCard0DetectLock);
}


EFI_STATUS
SunxiSdMmcDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS    Status;
  UINTN i;
  UINT64 Lba;
  EFI_BLOCK_IO_PROTOCOL* pSdMmcBlockIo;
  SDHC_DEVICE_PATH* pSdMmcDevicePath;
  SUNXI_FLASH_IO_PROTOCOL *FlashIo = NULL;
  BOOLEAN   NandExistFlag = FALSE;
  INT32   SdMmc0Used = 0;
  
  user_gpio_set_t Card0DetectGpio;
  //if we are going into the card burn mode ,the create the sdmmc host 2 instance.
  INTN HostNumberUseMap[MAX_MMC_NUM] ={2,0,-1,-1};
  //INTN HostNumberUseMap[MAX_MMC_NUM] ={0,2,-1,-1};
  
  if(SunxiGetBootWorkMode() == WORK_MODE_BOOT)
  {

  }
  else 
  {
    //get FlashIO protocol
    Status = gBS->LocateProtocol(&gSunxiFlashIoProtocolGuid, NULL, (VOID **)&FlashIo);
    if (!EFI_ERROR (Status)) 
    {
      DEBUG((EFI_D_INFO, "SdMMCDxe:Find FlashIo Portocol , So Nand Flash Exist\n"));
      NandExistFlag = TRUE;
    }
  }

  SetMem(gSdMmcHandleArray,sizeof(gSdMmcHandleArray),0);

  // Ensure the Cpu architectural protocol is already installed
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&Cpu);
  ASSERT_EFI_ERROR(Status);
  
  script_parser_fetch("sdc0", "sdc0_used",(INT32*)&SdMmc0Used,1);

  if(!SdMmc0Used) {   
    HostNumberUseMap[1] = -1;
    goto InitController;
  }
  script_parser_fetch("sdc0", "cd-gpios",(INT32*)&Card0DetectGpio,sizeof(user_gpio_set_t)/sizeof(INT32));
  Card0DetectGpio.mul_sel  =0;
  gCard0DetectGpioHandle = gpio_request(&Card0DetectGpio,1);
  if(!gCard0DetectGpioHandle)
  {
    AsciiPrint("request card0 detect Gpio failed! please check the sysconfig file.\n");
    ASSERT(FALSE);
  }
  //initialize a mutex lock to detect the card0 insert state.
  EfiInitializeLock(&gCard0DetectLock,TPL_CALLBACK);
  
  Status = gBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK, Card0DetectTimerCallback, NULL, &gCard0DetectTimerEvent);
  ASSERT_EFI_ERROR (Status);
  //set the card0 detect timer callback freq as 100MS.
  Status = gBS->SetTimer (gCard0DetectTimerEvent, TimerPeriodic, 10*1000*100); 
  ASSERT_EFI_ERROR (Status);
  
InitController:
  for(i=0;i<sizeof(HostNumberUseMap)&&(HostNumberUseMap[i]>=0);i++){

    if((HostNumberUseMap[i]==0)&&!Card0Present()) continue;
    if((HostNumberUseMap[i]==2)&& NandExistFlag)  continue;
    
    if(SDMMC_LogicalInit(HostNumberUseMap[i],0)) 
    {
      AsciiPrint("Init the SDMMC%d failed!\n",HostNumberUseMap[i]);
      continue;
    }
    else
    {
      Lba = SDMMC_LogicalLBA(HostNumberUseMap[i]);
      //a little bit tricky. we set the offset to 0 so that we can access the physical size.
      DEBUG((EFI_D_INFO,"SDMMC total psysical Sectors:0x%lx\n",Lba));
    }
    pSdMmcBlockIo = (EFI_BLOCK_IO_PROTOCOL*)AllocateZeroPool(sizeof(EFI_BLOCK_IO_PROTOCOL));
    pSdMmcBlockIo->ReadBlocks = SdMmcReadBlocks;
    pSdMmcBlockIo->WriteBlocks= SdMmcWriteBlocks;
    pSdMmcBlockIo->FlushBlocks= SdMmcFlushBlocks;
    pSdMmcBlockIo->Reset    = SdMmcReset;

    pSdMmcDevicePath = (SDHC_DEVICE_PATH*)AllocateZeroPool(sizeof(SDHC_DEVICE_PATH));
    CopyMem(pSdMmcDevicePath,&SdMmcDevicePath,sizeof(SDHC_DEVICE_PATH));
  
    if(HostNumberUseMap[i]==0){
      pSdMmcBlockIo->Media = &gSdMmc0;
      CopyGuid(&pSdMmcDevicePath->Mmc.Guid,&gSdMmc0DevicePathGuid);
    }
    else if(HostNumberUseMap[i]==2){
      pSdMmcBlockIo->Media = &gSdMmc2;  
      CopyGuid(&pSdMmcDevicePath->Mmc.Guid,&gSdMmc2DevicePathGuid);
    }
    pSdMmcBlockIo->Media->MediaPresent=TRUE;
    pSdMmcBlockIo->Media->LastBlock = Lba-1;

    if(HostNumberUseMap[i]==2){  
      Status = gBS->LocateProtocol(&gSunxiFlashIoProtocolGuid, NULL, (VOID **)&FlashIo);
      if (!EFI_ERROR (Status)) 
      {
        DEBUG((EFI_D_INFO, "SdMMCDxe:Find FlashIo Portocol , So eMMC or NAND Flash Exist\n"));
      }
      else
      {
        SunxiSetBurnStorage(SunxiBurnStorageEMMC);
        //call the InstalleMMCFlashIoProtocol to publish the SunxiFlashIO protocol befor BlockIo
        InstallMMCFlashIoProtocol(&gSdMmcHandleArray[HostNumberUseMap[i]],pSdMmcBlockIo);
      }
    }
    else if(HostNumberUseMap[i]==0)
    {
      Status = gBS->LocateProtocol(&gSunxiFlashIoProtocolGuid, NULL, (VOID **)&FlashIo);
      if (!EFI_ERROR (Status)) 
      {
        DEBUG((EFI_D_INFO, "SdMMCDxe:Find FlashIo Portocol , So eMMC or NAND Flash Exist\n"));
      }
      else
      {
        SunxiSetBurnStorage(SunxiBurnStorageSDHC);
        //call the InstalleMMCFlashIoProtocol to publish the SunxiFlashIO protocol befor BlockIo
        InstallMMCFlashIoProtocol(&gSdMmcHandleArray[HostNumberUseMap[i]],pSdMmcBlockIo);
      }
    }
    //Publish BlockIO.
    Status = gBS->InstallMultipleProtocolInterfaces (
        &gSdMmcHandleArray[HostNumberUseMap[i]],
        &gEfiBlockIoProtocolGuid,    pSdMmcBlockIo,
        &gEfiDevicePathProtocolGuid, pSdMmcDevicePath,
        NULL
        );
    ASSERT_EFI_ERROR (Status);
 
    if(SunxiGetBootWorkMode() == WORK_MODE_BOOT  && (HostNumberUseMap[i]==2 || HostNumberUseMap[i]==0))
    {  
      //Status = InstallMMCFsIoProtocol(pSdMmcBlockIo);
      //ASSERT_EFI_ERROR (Status);
    } 
  }

  do {
    struct mmc *mmc_dev = NULL;

    if(SDMMC_PhyInit(1)) // physical init sdc1 
    {
      AsciiPrint("Physical init the SDMMC1 failed!\n");
      break;
    }
    mmc_dev = find_mmc_device(1);
    if (!mmc_dev)
    {
      AsciiPrint("find_mmc_device SDMMC1 failed!\n");
      break;
    }
    mmc_set_clock(mmc_dev, 50000000);
    AsciiPrint("mmc_set_clock SDMMC1\n");
  } while (0);

  return EFI_SUCCESS;
}

