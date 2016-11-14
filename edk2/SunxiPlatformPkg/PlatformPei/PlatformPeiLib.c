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

#include <PiPei.h>
#include <Library/ArmPlatformLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/PrePiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SysConfigLib.h>
#include <Interinc/sunxi_uefi.h>
#include <Guid/SunxiScriptParseHob.h>
#include <Guid/SunxiVariableHob.h>
#include <Guid/SunxiBootInfoHob.h>


EFI_STATUS
EFIAPI
SunxiScriptParsePeim (
  VOID
  )
{
  UINT32 ScriptLength,UefiLength;
  EFI_PHYSICAL_ADDRESS  *SunxiScriptBase;
  struct spare_boot_head_t* spare_head; 
  
  SUNXI_SCRIPT_PARSE_HOB  *Hob;
  EFI_STATUS Status = EFI_SUCCESS;
  
  spare_head = get_spare_head(PcdGet32 (PcdFdBaseAddress));
  
  // ASSERT(!(spare_head->boot_head.length==spare_head->boot_head.uefi_length));
  if(spare_head->boot_head.length==spare_head->boot_head.uefi_length)
  {
    DEBUG((DEBUG_WARN, "++NO Sunxi Script Found in firmware.\n"));
    return EFI_SUCCESS;
  }

  ScriptLength = spare_head->boot_head.length - spare_head->boot_head.uefi_length;  
  UefiLength   = spare_head->boot_head.uefi_length;
  
  DEBUG((DEBUG_INFO, "++ScriptLength =%d,UefiLength=%d\n",ScriptLength,UefiLength));

  ASSERT(!(ScriptLength&(spare_head->boot_head.align_size-1)));

  // Allocate pages for sunxi script.
  SunxiScriptBase =AllocatePages (EFI_SIZE_TO_PAGES (ScriptLength));
  if (SunxiScriptBase == NULL) {
    return RETURN_OUT_OF_RESOURCES;
  }
  
  CopyMem((VOID *)SunxiScriptBase,(VOID *)(PcdGet32 (PcdFdBaseAddress)+UefiLength),ScriptLength);
  Hob = CreateHob (EFI_HOB_TYPE_GUID_EXTENSION, sizeof (SUNXI_SCRIPT_PARSE_HOB));
  
  ASSERT(Hob != NULL);

  CopyGuid (&(Hob->Header.Name), &gSunxiScriptParseHobGuid);
  
  Hob->SunxiScriptParseBase =(EFI_PHYSICAL_ADDRESS)((INTN)SunxiScriptBase);
  
  Hob->SunxiScriptParseSize = ScriptLength;
  
  DEBUG((DEBUG_INFO, "--%a:SunxiScriptBase=%lx\n",__func__,Hob->SunxiScriptParseBase));
  
  DEBUG((DEBUG_INFO, "--%a:SunxiScriptParseSize=%x\n",__func__,Hob->SunxiScriptParseSize));
  
  Status = script_parser_init((CHAR8*)(INTN)(Hob->SunxiScriptParseBase));
  
  ASSERT_EFI_ERROR (Status);
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BuildSunxiVariableHob (VOID)
{
  SUNXI_VARIABLE_HOB  *Hob;
  EFI_PHYSICAL_ADDRESS  *SunxiVariableBase;
  UINT32                 SunxiVariableSize;

  EFI_PHYSICAL_ADDRESS   *SunxiEnvBase;
  UINT32                 SunxiEnvSize = (128<<10);
  
  Hob = CreateHob (EFI_HOB_TYPE_GUID_EXTENSION, sizeof (SUNXI_VARIABLE_HOB));
  ASSERT(Hob != NULL);

  // Allocate pages for sunxi variable.
  SunxiVariableSize = PcdGet32 (PcdFlashNvStorageVariableSize);
  SunxiVariableBase = AllocatePages (EFI_SIZE_TO_PAGES (SunxiVariableSize));
  if (SunxiVariableBase == NULL) {
    return RETURN_OUT_OF_RESOURCES;
  }
  ZeroMem((VOID*)(UINTN*)SunxiVariableBase,SunxiVariableSize);

  // Allocate pages for sunxi env
  SunxiEnvBase = AllocatePages(EFI_SIZE_TO_PAGES (SunxiEnvSize));
  if (SunxiEnvBase == NULL) {
    return RETURN_OUT_OF_RESOURCES;
  }

  ZeroMem((VOID*)(UINTN*)SunxiEnvBase,SunxiEnvSize);

  CopyGuid (&(Hob->Header.Name), &gSunxiVariableGuid);
  Hob->SunxiVariableBase = (EFI_PHYSICAL_ADDRESS)((UINTN)SunxiVariableBase);
  Hob->SunxiVariableSize = SunxiVariableSize;
  Hob->SunxiEnvBase = (EFI_PHYSICAL_ADDRESS)((UINTN)SunxiEnvBase);
  Hob->SunxiEnvSize = SunxiEnvSize;
  Hob->SunxiEnvNeedImport = 0;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SunxiBootInfoInit (VOID)
{
  SUNXI_BOOTINFO_HOB  *Hob;
  struct spare_boot_head_t* spare_head;
  UINT64  DramSize = 0;
  
  spare_head = get_spare_head(PcdGet32 (PcdFdBaseAddress));
  DramSize = ((spare_head->boot_data.dram_para[5]>>16)&0xffff)*1024*1024;

  DEBUG((EFI_D_INIT," DramSize is 0x%x\n", DramSize));
  
  Hob = CreateHob (EFI_HOB_TYPE_GUID_EXTENSION, sizeof (SUNXI_BOOTINFO_HOB));
  ASSERT(Hob != NULL);

  CopyGuid (&(Hob->Header.Name), &gSunxiBootInfoHobGuid);
  Hob->WorkMode     = spare_head->boot_data.work_mode;
  Hob->StorageType  = spare_head->boot_data.storage_type;
  Hob->BurnStorageType = 0xff;  //unknow burn storage
  Hob->SystemMemoryBase = PcdGet64(PcdSystemMemoryBase);
  Hob->SystemMemorySize = PcdGet64(PcdSystemMemorySize);
  Hob->FrameBufferBase = PcdGet64(PcdFrameBufferBase);
  Hob->FrameBufferSize = PcdGet64(PcdFrameBufferSize);
  Hob->ParkSharedBase = PcdGet64(PcdMpParkSharedBase);
  Hob->ParkSharedSize = PcdGet64(PcdMpParkSharedSize);
  CopyMem(Hob->DramPara, spare_head->boot_data.dram_para, 32*4);
 
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
PlatformPeim (
  VOID
  )
{
  EFI_STATUS Status;

  SunxiBootInfoInit();

  BuildFvHob (PcdGet32(PcdFvBaseAddress), PcdGet32(PcdFvSize));
  Status = BuildSunxiVariableHob();
  if(EFI_ERROR(Status))
  {
    return Status;
  }

  return SunxiScriptParsePeim();

}
