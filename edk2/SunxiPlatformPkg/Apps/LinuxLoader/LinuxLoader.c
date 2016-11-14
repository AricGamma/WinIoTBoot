/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  WangWei <wangwei@allwinnertech.com>
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
#include <Library/BaseLib.h> 
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>

#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/LoadedImage.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SunxiPartitionLib.h>
#include <Library/SunxiMbr.h>
#include <Library/SunxiBootInfoLib.h>
#include <Library/PerformanceLib.h>

#include <Guid/SunxiVariableHob.h>

#include "SunxiLinuxHead.h"
#include "Helper.h"


#define  CMD_LINE_STR_Linux             ("console=ttyS0,115200 root=/dev/mmcblk0p7 init=/init vmalloc=384M loglevel=8 ")
#define  CMD_LINE_STR_Linux_NAND        ("console=ttyS0,115200 root=/dev/nandd init=/init vmalloc=384M loglevel=8 ")
#define  CMD_LINE_STR_Andriod           ("console=ttyS0,115200 root=/dev/mmcblk0p7 init=/init vmalloc=384M coherent_pool=8M cma_reserve=384M loglevel=8  config_size=56284 ")

static int Make_Argv(char *s, int argvsz, char *argv[]);

EFI_RUNTIME_SERVICES              *RuntimeServices = NULL;
EFI_BOOT_SERVICES                 *BootServices = NULL;


/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
BootArm (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  EFI_BLOCK_IO_PROTOCOL* pBlockIo = NULL;
  INT32 StorageType=0;
  CHAR8 *CmdLine = NULL;
  CHAR8 *DataBuff = NULL;
  
  const UINT32 DataBuffSize = 1024;
  CHAR8 *Argv[64];
  INT32 Argc = 0, i;
  EFI_LOADED_IMAGE_PROTOCOL*   LoadedImage;

  UINT32 BootOffSet;
  EFI_LBA BootImageAddr;
  EFI_PHYSICAL_ADDRESS  LinuxImageAdd ;

  CHAR8 *pDataBuff=NULL;
 
  CHAR8          SunxiBootCmd[64] = {0};
  UINTN          SunxiBootCmdSize=0;
  CHAR8          BootPartName[64] = {0};

  PERF_START  (NULL, "BTARM", NULL, 0);
  
  RuntimeServices = SystemTable->RuntimeServices;
  BootServices    = SystemTable->BootServices;
    
  //get sunxi bootcmd variable
  SunxiBootCmdSize = sizeof(SunxiBootCmd);
  Status = RuntimeServices->GetVariable(L"SunxiBootCmd", &gSunxiVariableGuid,NULL, &SunxiBootCmdSize, (VOID*)SunxiBootCmd);
  if(!AsciiStrCmp(SunxiBootCmd,"boot-recovery"))
  {
    AsciiStrCpy(BootPartName,"recovery");
  }
  else if(!AsciiStrCmp(SunxiBootCmd,"boot-normal"))
  {
    AsciiStrCpy(BootPartName,"boot");
  }
  else
  {
    AsciiPrint("BootCmd: %a is not support\n", SunxiBootCmd);
    return EFI_NOT_FOUND;
  }

  DataBuff = AllocatePool(1024);
  if(DataBuff == NULL)
  {
    Status = EFI_OUT_OF_RESOURCES;
    goto __BOOT_END;
  }
  
  Status = BootServices->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if(LoadedImage->LoadOptionsSize != 0){
    //get storage type from args
    ZeroMem(DataBuff,DataBuffSize);
    UnicodeStrToAsciiStr((CHAR16*)LoadedImage->LoadOptions,DataBuff);
    Argc = Make_Argv(DataBuff,sizeof(Argv)/sizeof(Argv[0]),Argv);
    for( i = 0; i < Argc; i ++)
    {
      DEBUG((DEBUG_INFO,"arg %d : %a\n",i, Argv[i]));
    }
    if(0 == AsciiStrLen(Argv[1]) || 0 == AsciiStrCmp(Argv[1],"nand")) {StorageType = 0;}
    else if(0 == AsciiStrCmp(Argv[1],"card0")) {StorageType = 1;}
    else if(0 == AsciiStrCmp(Argv[1],"card2")) {StorageType = 2;}
    else {return EFI_DEVICE_ERROR;}
  }
  else
  {
    StorageType = SunxiGetBootStorage();
  }

  AsciiPrint ("Boot Storage Type: %d\n", StorageType);

  LinuxImageAdd = PcdGet64(PcdSystemMemoryBase) + PcdGet64(PcdKernelImageOffset);

  //update script 
  UpdateScriptStorageParam(StorageType);
  UpdateScriptDramParam((void*)SunxiGetBootDramPram());
    
  //get BlockIO Instance
  Status = GetBlockIoInstanceByStorage(StorageType,&pBlockIo);
  if(EFI_ERROR (Status))
  {
    AsciiPrint("can't get BlockIO Instance\n");
    return Status;
  }
  
  pDataBuff = AllocatePool(SUNXI_MBR_SIZE);
  if(pDataBuff == NULL)
  {
    Status = EFI_OUT_OF_RESOURCES;
    goto __BOOT_END;
  }

  CmdLine = AllocateZeroPool(1024);
  if(CmdLine == NULL)
  {
    Status = EFI_OUT_OF_RESOURCES;
    goto __BOOT_END;
  }

  //read gpt
  Status = pBlockIo->ReadBlocks(pBlockIo,pBlockIo->Media->MediaId,
  SUNXI_GPT_PRIMARY_HEADER_LBA,pBlockIo->Media->BlockSize,pDataBuff);
  if(EFI_ERROR (Status))
  {
    DEBUG((EFI_D_INFO,"FsBlockIo:Read GPT Error\n"));
    goto __BOOT_END;
  }

  //suxi gpt init
  Status = SunxiGptPartitionInitialize(pBlockIo, pDataBuff, pBlockIo->Media->BlockSize);
  if(EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR,"FsBlockIo:sunxi_partition_init Error\n"));
    goto __BOOT_END;
  }

  //read linux image
  Status = SunxiGptPartitionGetOffsetByName(BootPartName,&BootOffSet);
  if(EFI_ERROR(Status))
  {
    AsciiPrint ("Can't Find  Partition:%a\n", BootPartName);
    goto __BOOT_END;
  }
   
  BootImageAddr =(EFI_LBA)BootOffSet;
  Status = SunxiReadLinuxImage(pBlockIo,(StorageType==0?BootImageAddr:GET_BLOCK_ADDR(BootImageAddr)),LinuxImageAdd);
  if(EFI_ERROR(Status))
  {
    goto __BOOT_END;
  }
  
  //set cmdline for linux
  SunxiGptGetPartitionInfo(StorageType,DataBuff);
  //SunxiGetPartitionInfo(StorageType,DataBuff);
  GetCmdLine(CmdLine,DataBuff,StorageType);//AsciiStrCpy(CmdLine,CMD_LINE_STR_Andriod);
  DEBUG((DEBUG_INFO,"cmdline: %a\n",CmdLine));
  //set script linux
  SetScriptForLinux();
  
  PERF_END(NULL, "BTARM", NULL, 0);
  if (PerformanceMeasurementEnabled ()) {
    PrintPerformance ();
  }
    
  //boot linux
  Status = SunxiBdsBootLinuxAtag(LinuxImageAdd,CmdLine);
    
__BOOT_END:
  if(pDataBuff)
    FreePool(pDataBuff);

  if(CmdLine)
    FreePool(CmdLine);

  return Status;  
}


static int Make_Argv(CHAR8 *s, int argvsz, CHAR8 *argv[])
{
  int argc = 0;

  /* split into argv */
  while (argc < argvsz - 1) {
    /* skip any white space */
    while ((*s == ' ') || (*s == '\t'))
      ++s;

    if (*s == '\0') /* end of s, no more args */
      break;

    argv[argc++] = s; /* begin of argument string */
       

    /* find end of string */
    while (*s && (*s != ' ') && (*s != '\t'))
      ++s;

    if (*s == '\0')   /* end of s, no more args */
      break;

    *s++ = '\0';    /* terminate current arg   */
  }
  argv[argc] = NULL;

  return argc;
}

