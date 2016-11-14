/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  tangmanliang <tangmanliang@allwinnertech.com>
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

#include <Library/BaseLib.h> 
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>

#include <Protocol/BlockIo.h>

//--sysconfig
#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <Library/HobLib.h>
#include <Guid/SunxiScriptParseHob.h>
#include <Guid/SunxiVariableHob.h>

#include <Library/SysConfigLib.h>
#include <Library/SerialPortLib.h>
#include <Library/TimerLib.h>
#include <Library/PerformanceLib.h>

#include "SunxiLinuxHead.h"

#define  LINUX_IMAGE_FIRST_SIZE   (32 * 1024)
#define  ALIGN(x, a)              (((x) + ((a) - 1)) & ~((a) - 1))

extern EFI_RUNTIME_SERVICES              *RuntimeServices;
extern EFI_BOOT_SERVICES                 *BootServices;

VOID UpdateScriptDramParam(VOID* DramParaAdd)
{
  UINT32 *addr;

  addr = (UINT32 *)DramParaAdd;

  //puts("dram_para_set start\n");
  script_parser_patch("dram_para", "dram_clk", &addr[0], 1);
  script_parser_patch("dram_para", "dram_type", &addr[1], 1);
  script_parser_patch("dram_para", "dram_zq", &addr[2], 1);
  script_parser_patch("dram_para", "dram_odt_en", &addr[3], 1);

  script_parser_patch("dram_para", "dram_para1", &addr[4], 1);
  script_parser_patch("dram_para", "dram_para2", &addr[5], 1);

  script_parser_patch("dram_para", "dram_mr0", &addr[6], 1);
  script_parser_patch("dram_para", "dram_mr1", &addr[7], 1);
  script_parser_patch("dram_para", "dram_mr2", &addr[8], 1);
  script_parser_patch("dram_para", "dram_mr3", &addr[9], 1);

  script_parser_patch("dram_para", "dram_tpr0", &addr[10], 1);
  script_parser_patch("dram_para", "dram_tpr1", &addr[11], 1);
  script_parser_patch("dram_para", "dram_tpr2", &addr[12], 1);
  script_parser_patch("dram_para", "dram_tpr3", &addr[13], 1);
  script_parser_patch("dram_para", "dram_tpr4", &addr[14], 1);
  script_parser_patch("dram_para", "dram_tpr5", &addr[15], 1);
  script_parser_patch("dram_para", "dram_tpr6", &addr[16], 1);
  script_parser_patch("dram_para", "dram_tpr7", &addr[17], 1);
  script_parser_patch("dram_para", "dram_tpr8", &addr[18], 1);
  script_parser_patch("dram_para", "dram_tpr9", &addr[19], 1);
  script_parser_patch("dram_para", "dram_tpr10", &addr[20], 1);
  script_parser_patch("dram_para", "dram_tpr11", &addr[21], 1);
  script_parser_patch("dram_para", "dram_tpr12", &addr[22], 1);
  script_parser_patch("dram_para", "dram_tpr13", &addr[23], 1);
  //puts("dram_para_set end\n");

}

VOID UpdateScriptStorageParam(UINT32 StorageType)
{
  UINT32 nand_used, sdc0_used, sdc2_used, sdc_detmode=3;

  switch(StorageType)
  {
    case 0:
      {
        nand_used = 1;
        sdc2_used  = 0;
        script_parser_patch("nand0_para", "nand0_used", &nand_used, 1);
        script_parser_patch("nand1_para", "nand1_used", &nand_used, 1);
        script_parser_patch("mmc2_para",  "sdc_used",   &sdc2_used, 1);
      }
      break;
    case 1:
      {
        sdc0_used  = 1;
        script_parser_patch("mmc0_para",  "sdc_used",   &sdc0_used, 1);
        script_parser_patch("mmc0_para",  "sdc_detmode", &sdc_detmode, 1); 
      }
      break;
    case 2:
      {
        nand_used = 0;
        sdc2_used = 1;
        script_parser_patch("nand0_para", "nand0_used", &nand_used, 1);
        script_parser_patch("nand1_para", "nand1_used", &nand_used, 1);
        script_parser_patch("mmc2_para",  "sdc_used",   &sdc2_used, 1);
        script_parser_patch("mmc2_para",  "sdc_detmode", &sdc_detmode, 1);
      }
      break;
    default:
      AsciiPrint("Error:StroageType is %d\n",StorageType);
      break;
  }   
}

void SetScriptForLinux(void)
{    
  SUNXI_SCRIPT_PARSE_HOB      *GuidHob;
  EFI_PHYSICAL_ADDRESS  LinuxScriptAdd;
  LinuxScriptAdd = PcdGet64(PcdSystemMemoryBase) + PcdGet64(PcdKernelScriptOffset);

  //EFI_STATUS Status = EFI_SUCCESS;
  GuidHob = GetFirstGuidHob (&gSunxiScriptParseHobGuid);
  ASSERT(GuidHob != NULL);
  DEBUG((DEBUG_INFO, "--%a:SunxiScriptParseBase=%lx\n",__func__,GuidHob->SunxiScriptParseBase));
  DEBUG((DEBUG_INFO, "--%a:SunxiScriptParseSize=%x\n",__func__,GuidHob->SunxiScriptParseSize));
  CopyMem((void*)(UINTN)LinuxScriptAdd,(void*)(INTN)GuidHob->SunxiScriptParseBase,GuidHob->SunxiScriptParseSize);
}


EFI_STATUS
GetBlockIoInstanceByStorage(INT32 StorageType,  EFI_BLOCK_IO_PROTOCOL  **Instance)
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         NumberOfHandles;
  UINTN                         Index;
  EFI_BLOCK_IO_MEDIA            *DeviceID;
  UINT32                        MediaId;
  EFI_BLOCK_IO_PROTOCOL         *DeviceInstance = NULL;
  UINT32                        FindFlag = 0;
  
  if(StorageType == 0)
  {
    MediaId =  SIGNATURE_32('n','a','n','d');
  }
  else if(StorageType == 1)
  {
    MediaId =  SIGNATURE_32('s','d','h','c');
  }
  else if(StorageType == 2)
  {
    MediaId = SIGNATURE_32('e','m','m','c');
  }
  else
  {
    return EFI_DEVICE_ERROR;
  }

  //
  // Locate protocol.
  //
  Status = BootServices->LocateHandleBuffer (
                   ByProtocol,
                   &gEfiBlockIoProtocolGuid,
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

  for (Index = 0; Index < NumberOfHandles; Index++) {
    //
    // Get the protocol on this handle
    // This should not fail because of LocateHandleBuffer
    //
    Status = BootServices->HandleProtocol (
                     HandleBuffer[Index],
                     &gEfiBlockIoProtocolGuid,
                     (VOID**) &DeviceInstance
                     );
    ASSERT_EFI_ERROR (Status);

 
    DeviceID = DeviceInstance->Media;
    if(DeviceID->MediaId == MediaId)
    {
      *Instance = DeviceInstance;
      FindFlag = 1;
      break;
    }

  }

  //
  // Free any allocated buffers
  //
  BootServices->FreePool (HandleBuffer);

  return FindFlag ? Status : EFI_DEVICE_ERROR;
}



EFI_STATUS SunxiReadLinuxImage(EFI_BLOCK_IO_PROTOCOL* pBlockIo ,UINT32 start, EFI_PHYSICAL_ADDRESS  ImageAddr)
{
  EFI_STATUS Status;
  UINTN rbytes, rblock;
  UINT32 start_block = start;
  void *addr;
  struct fastboot_boot_img_hdr *fb_hdr;
  EFI_PHYSICAL_ADDRESS kaddr,raddr;

  addr = (void *)(UINTN)ImageAddr;
  //ret = sunxi_flash_read(start_block, LINUX_IMAGE_FIRST_SIZE/512, addr);
  Status = pBlockIo->ReadBlocks(pBlockIo,pBlockIo->Media->MediaId, 
  (start_block),LINUX_IMAGE_FIRST_SIZE, addr);
  if(EFI_ERROR (Status))
  {
    AsciiPrint("Read LinuxImage error: start=%x, addr=%x\n", start_block, (UINT32)addr);
    return EFI_DEVICE_ERROR;
  }

  fb_hdr = (struct fastboot_boot_img_hdr *)addr;
  if (CompareMem(fb_hdr->magic, FASTBOOT_BOOT_MAGIC, 8))
  {
    AsciiPrint("Error: bad boot image magic, maybe not a boot.img?\n");
    return EFI_DEVICE_ERROR;
  }
  else
  {
    rbytes = fb_hdr->kernel_size + fb_hdr->ramdisk_size + fb_hdr->second_size + 1024 * 1024 + 511;
  }
  rblock = rbytes/512 - LINUX_IMAGE_FIRST_SIZE/512;
  start_block += LINUX_IMAGE_FIRST_SIZE/512;
  addr = (void *)(UINTN)(ImageAddr+ LINUX_IMAGE_FIRST_SIZE);
  //ret = sunxi_flash_read(start_block, rblock, addr);
  Status = pBlockIo->ReadBlocks(pBlockIo,pBlockIo->Media->MediaId, 
  (start_block),rblock*512, addr);
  if(EFI_ERROR (Status))
  {
    AsciiPrint("Read LinuxImage error: start=%x, addr=%x\n", start_block, (UINT32)addr);
    return EFI_DEVICE_ERROR;
  }
  AsciiPrint("Sunxi  read :offset %x, %d bytes OK\n", start<<9, rbytes);

  kaddr = ImageAddr + fb_hdr->page_size;
  raddr = kaddr + ALIGN(fb_hdr->kernel_size, fb_hdr->page_size);
    
  if(fb_hdr->kernel_addr != (UINTN)kaddr)
  {
    AsciiPrint("moving kernel from %x to: %x, size 0x%x\n", (UINTN)kaddr, fb_hdr->kernel_addr, fb_hdr->kernel_size);
    CopyMem((void*) fb_hdr->kernel_addr, (const void *)(UINTN)kaddr, fb_hdr->kernel_size);
  }
  if(fb_hdr->ramdisk_addr != (UINTN)raddr)
  {
    AsciiPrint("moving ramdisk from %x to: %x, size 0x%x\n", (UINTN)raddr, fb_hdr->ramdisk_addr, fb_hdr->ramdisk_size);
    CopyMem((void*) fb_hdr->ramdisk_addr, (const void *)(UINTN)raddr, fb_hdr->ramdisk_size);
  }

  return  EFI_SUCCESS;
}

BOOLEAN BootEnvSplit(CHAR8* src, CHAR8 flag, CHAR8* key, CHAR8* value, BOOLEAN *expand)
{
  UINT32 i = 0;

  for(i = 0; i < AsciiStrLen(src); i++)
  {
    if(src[i] == flag)
    {
      break;
    }
  }
  if(i < AsciiStrLen(src))
  {
    CopyMem(key, src, i);
    key[i] = 0;

    //need expand
    if(*(src+i+1) == '$')
    {
      ////delete    "${...}"
      AsciiStrCpy(value, src+i+1+2);
      value[AsciiStrLen(value)-1] = 0; 
      *expand = TRUE;
    }
    else
    {
      AsciiStrCpy(value, src+i+1);
      *expand = FALSE;
    }

    return TRUE;
  }
  return FALSE;   
}


#define  VARIABLE_MAX_LEN (1024)

EFI_STATUS
GetCmdLineFormEnvVariable(CHAR8 *CmdLine, UINT32 StorageType)
{
  EFI_STATUS Status;
  UINT32  BootCmdSize=0;

  CHAR8  *BootCmdVariable = NULL;
  CHAR16 VariableName[64];
  UINT32 VariableSize;
  CHAR8  *VariableValue = NULL;

  CHAR8  StrKeyValue[256];
  CHAR8  Key[64], Value[128];
  INT32  index,j;
  BOOLEAN Expand;

  VariableValue = AllocateZeroPool(VARIABLE_MAX_LEN);
  ASSERT(VariableValue != NULL);
  BootCmdVariable = AllocateZeroPool(VARIABLE_MAX_LEN);
  ASSERT(BootCmdVariable != NULL);

  //get boot_args variable value
  ZeroMem(VariableName,sizeof(VariableName));
  StrCpy(VariableName, StorageType ? L"setargs_mmc":L"setargs_nand");

  BootCmdSize = VARIABLE_MAX_LEN;
  Status = RuntimeServices->GetVariable(VariableName, 
           &gSunxiVariableGuid,NULL, &BootCmdSize, (VOID*)BootCmdVariable);
  if(EFI_ERROR(Status))
  {
    Print(L"variable:get %s error\n", VariableName);
    goto __FUN_END;
  }
  for(index = 0,j = 0; index < BootCmdSize; index++)
  {   
    if(j == 0 && BootCmdVariable[index] == 0x20 ) continue;

    if(BootCmdVariable[index] == 0x20 || BootCmdVariable[index] == 0)
    {
      StrKeyValue[j] = 0;
      j = 0;
      ZeroMem(Key,sizeof(Key));
      ZeroMem(Value,sizeof(Value));
      //AsciiPrint("string = %a\n", StrKeyValue);
      if(!BootEnvSplit(StrKeyValue,'=',Key,Value,&Expand)) 
        continue;
      if(Expand)
      {
        AsciiStrToUnicodeStr(Value,VariableName);
        VariableSize = VARIABLE_MAX_LEN;//set a big enouth value
        Status = RuntimeServices->GetVariable (VariableName, &gSunxiVariableGuid, 
            NULL, &VariableSize, (VOID*)VariableValue);
        if(EFI_ERROR(Status))
        {
          Print(L"variable:get %s error\n", VariableName);
          continue;
          //goto __FUN_END;
        }
        AsciiStrCat(CmdLine,Key); 
        AsciiStrCat(CmdLine,"=");
        AsciiStrCat(CmdLine,VariableValue);

      }
      else
      {
        AsciiStrCat(CmdLine,StrKeyValue);
      }
      AsciiStrCat(CmdLine," ");
            
    }
    StrKeyValue[j++] = BootCmdVariable[index];
        
  }

  //get android charge mode
  Status = RuntimeServices->GetVariable (L"SunxiChargeMode", &gSunxiVariableGuid, 
                  NULL, &VariableSize, (VOID*)VariableValue);
  if(!EFI_ERROR(Status))
  {
    if(AsciiStrCmp(VariableValue,"Enable") == 0)
    {
      AsciiStrCat(CmdLine,"androidboot.mode=charger ");
    }
  }
    
__FUN_END: 
  if(BootCmdVariable)
    FreePool(BootCmdVariable);
  if(VariableValue)
    FreePool(VariableValue);
  
  return Status;
}

EFI_STATUS
GetCmdLine(CHAR8 *CmdLine, CHAR8*PartInfo, UINT32 StorageType)
{
  CHAR8 TmpStr[512];
  INT32 ChargeMode = 0;
  SUNXI_SCRIPT_PARSE_HOB      *GuidHob = NULL;

  //EFI_STATUS Status = EFI_SUCCESS;
  GuidHob = GetFirstGuidHob (&gSunxiScriptParseHobGuid);
  ASSERT(GuidHob != NULL);

  GetCmdLineFormEnvVariable(CmdLine,StorageType);
  if(0 == script_parser_fetch("charging_type", "charging_type", &ChargeMode, 1))
  {
    if(ChargeMode)
    {
      //AsciiStrCat(CmdLine,"androidboot.mode=charger ");
    }
  }
  ZeroMem(TmpStr,sizeof(TmpStr));
  AsciiSPrint(TmpStr,sizeof(TmpStr),"config_size=%d ",GuidHob->SunxiScriptParseSize);
  AsciiStrCat(CmdLine,TmpStr);
  AsciiPrint("cmd=%a\n",TmpStr );
    
  ZeroMem(TmpStr,sizeof(TmpStr));
  AsciiSPrint(TmpStr,sizeof(TmpStr),"partitions=%a ",PartInfo);
  AsciiStrCat(CmdLine,TmpStr);
  AsciiPrint("cmd=%a\n",TmpStr );

  ZeroMem(TmpStr,sizeof(TmpStr));
  AsciiSPrint(TmpStr,sizeof(TmpStr),"boot_type=%d ",StorageType);
  AsciiStrCat(CmdLine,TmpStr);
  AsciiPrint("cmd=%a\n",TmpStr );

  return EFI_SUCCESS;
}

STATIC CHAR8 *mTokenList[] = {
  /*"SEC",*/
  "PEI",
  "DXE",
  "BDS",
  "BTARM",
  NULL
};


VOID
PrintPerformance (
  VOID
  )
{
  UINTN       Key;
  CONST VOID  *Handle;
  CONST CHAR8 *Token, *Module;
  UINT64      Start, Stop, TimeStamp;
  UINT64      Delta, TicksPerSecond, Milliseconds;
  UINTN       Index;
  CHAR8       Buffer[100];
  UINTN       CharCount;
  BOOLEAN     CountUp;

  TicksPerSecond = GetPerformanceCounterProperties (&Start, &Stop);
  if (Start < Stop) {
    CountUp = TRUE;
  } else {
    CountUp = FALSE;
  }

  TimeStamp = 0;
  Key       = 0;
  do {
    Key = GetPerformanceMeasurement (Key, (CONST VOID **)&Handle, &Token, &Module, &Start, &Stop);
    if (Key != 0) {
      for (Index = 0; mTokenList[Index] != NULL; Index++) {
        if (AsciiStriCmp (mTokenList[Index], Token) == 0) {
          Delta = CountUp?(Stop - Start):(Start - Stop);
          TimeStamp += Delta;
          Milliseconds = DivU64x64Remainder (MultU64x32 (Delta, 1000), TicksPerSecond, NULL);
          CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"%6a %6ld ms\n", Token, Milliseconds);
          SerialPortWrite ((UINT8 *) Buffer, CharCount);
          break;
        }
      }
    }
  } while (Key != 0);

  CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"Total Time = %ld ms\n\n", DivU64x64Remainder (MultU64x32 (TimeStamp, 1000), TicksPerSecond, NULL));
  SerialPortWrite ((UINT8 *) Buffer, CharCount);
}

