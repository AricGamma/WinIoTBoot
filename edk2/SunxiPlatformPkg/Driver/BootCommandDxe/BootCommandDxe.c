/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  wangwei <wangwei@allwinnertech.com>
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
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SunxiPartitionLib.h>
#include <Library/SunxiMbr.h>
#include <Library/SunxiBootInfoLib.h>
#include <Library/SysConfigLib.h>
#include <Library/TimerLib.h>
#include <Library/IoLib.h>
#include <Library/ArmLib.h>
#include <Library/SunxiKeyLib.h>
#include <Library/SunxiCommonLib.h>

#include <Interinc/sunxi_uefi.h>

#include <Guid/SunxiVariableHob.h>

#include <Protocol/SunxiFlashIo.h>
#include <Protocol/AxpPower.h>


enum _USERBOOTMODE
{
 BOOT_NULL_MODE = 0x1000,
 ANDROID_FASTBOOT_MODE, 
 ANDROID_RECOVERY_MODE,
 USER_SELECT_MODE,
 MISC_FASTBOOT_MODE,
 MISC_BOOT_RECOVERY_MODE,
 MISC_USB_RECOVERY_MODE,
 MISC_EFEX_MODE,
 MISC_RESIGNATURE_MODE
};


#define BOOTCMDDXE_INFO(ARG...)  DEBUG (( EFI_D_INFO,"[BootCmdDxe Info]:"ARG))
#define BOOTCMDDXE_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[BootCmdDxe Error]:"ARG))


static AXP_POWER_PROTOCOL      *AxpPower = NULL ;
//static SUNXI_FLASH_IO_PROTOCOL *FlashIo = NULL;

static UINTN CheckUserInput(VOID)
{
  EFI_STATUS Status;
  EFI_INPUT_KEY Key;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *TextIn;
  UINT32 times = 20;
  UINTN UserCheck = EFI_UNSUPPORTED;
  CHAR8 BootCmd[64] = {0};

  DEBUG((EFI_D_INFO, "check user input begin\n"));
  Status = gBS->LocateProtocol (&gEfiSimpleTextInProtocolGuid, NULL, (VOID **) &TextIn);
  if (EFI_ERROR (Status)) {
      BOOTCMDDXE_ERROR("Couldn't open Text Input Protocol: %r\n", Status);
  }
 
  while(times--)
  {  
    gBS->Stall (10000);
    Status = TextIn->ReadKeyStroke(TextIn,&Key);
    if(Status == EFI_SUCCESS)
    break;
  }
  if(Status == EFI_SUCCESS)
  {
    // number  '2', jmp to fel
    if (Key.UnicodeChar == 50)
    {
      AsciiPrint("Read Fel Key('2') From SerialPort Success\n", Key.UnicodeChar );
      SunxiSetFelKey();
      AsciiPrint("WatchDog reset cpu\n");
      SunxiWatchDogReset();
    }    
    // number  '1', jmp to mass storage
    else if(Key.UnicodeChar == 49)
    {
      AsciiPrint("Read Fel Key('1') From SerialPort Success\n");
      AsciiStrCpy(BootCmd, "boot-mass");
      Status = gRT->SetVariable (
        L"SunxiBootCmd",
        &gSunxiVariableGuid,
        (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
        AsciiStrLen(BootCmd)+1, //add '\0' in the end
        BootCmd);
      if(EFI_ERROR(Status))
      {
        BOOTCMDDXE_ERROR("Set Sunxi BootCmd Variable Error\n");
        return Status;
      }
      AsciiPrint("Sunxi BootCmd = %a\n", BootCmd);
      UserCheck = EFI_SUCCESS;
    }
    // number  '3', jmp to test app
    else if(Key.UnicodeChar == 51)
    {
      AsciiPrint("Read Fel Key('3') From SerialPort Success\n");
      AsciiStrCpy(BootCmd, "test-app");
      Status = gRT->SetVariable (
        L"SunxiBootCmd",
        &gSunxiVariableGuid,
        (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
        AsciiStrLen(BootCmd)+1, //add '\0' in the end
        BootCmd);
      if(EFI_ERROR(Status))
      {
        BOOTCMDDXE_ERROR("Set Sunxi BootCmd Variable Error\n");
        return Status;
      }
      AsciiPrint("Sunxi BootCmd = %a\n", BootCmd);
      UserCheck = EFI_SUCCESS;
    }
    // number  '4', jmp to uefi shell
    else if(Key.UnicodeChar == 52)
    {
      AsciiPrint("Read Fel Key('4') From SerialPort Success\n");
      AsciiStrCpy(BootCmd, "uefi-shell");
      Status = gRT->SetVariable (
        L"SunxiBootCmd",
        &gSunxiVariableGuid,
        (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
        AsciiStrLen(BootCmd)+1, //add '\0' in the end
        BootCmd);
      if(EFI_ERROR(Status))
      {
        BOOTCMDDXE_ERROR("Set Sunxi BootCmd Variable Error\n");
        return Status;
      }
      AsciiPrint("Sunxi BootCmd = %a\n", BootCmd);
      UserCheck = EFI_SUCCESS;
    }
    // number  '5', jmp to fast boot mode
    else if(Key.UnicodeChar == 53)
    {
      AsciiPrint("Read Fel Key('5') From SerialPort Success\n");
      AsciiStrCpy(BootCmd, "fastboot");
      Status = gRT->SetVariable (
        L"SunxiBootCmd",
        &gSunxiVariableGuid,
        (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
        AsciiStrLen(BootCmd)+1, //add '\0' in the end
        BootCmd);
      if(EFI_ERROR(Status))
      {
        BOOTCMDDXE_ERROR("Set Sunxi BootCmd Variable Error\n");
        return Status;
      }
      AsciiPrint("Sunxi BootCmd = %a\n", BootCmd);
      UserCheck = EFI_SUCCESS;
    }
    }
    return UserCheck;

}

static UINTN GetPmuBootMode(VOID)
{
  UINTN PreMode = 0;
  EFI_STATUS Status;
  Status = AxpPower->ProbePreSystemMode(&PreMode);
  if(EFI_ERROR(Status))
  {
    BOOTCMDDXE_ERROR("Get PME PreSystemMode error\n");
  }

  return PreMode;
}

static UINTN GetKeyBootMode(void)
{
  INT32 ret1, ret2;
  INT32 key_high, key_low;
  INT32 keyvalue = 0;
    
  //read key 
  SunxiKeyInit();
  SunxiKeyRead();
  MicroSecondDelay(10*1000); //delay 10ms
  keyvalue = SunxiKeyRead();
  SunxiKeyExit();
    
  if(keyvalue <= 0 )
  {
    AsciiPrint("no key pressed\n", keyvalue);
    return BOOT_NULL_MODE;
  }
   
  //check fel key
  ret1 = script_parser_fetch("fel_key", "fel_key_max", &key_high, 1);
  ret2 = script_parser_fetch("fel_key", "fel_key_min", &key_low, 1);
  if(ret1 || ret2)
  {
    AsciiPrint("cant find fel_key config value\n");
  }
  else
  {
    AsciiPrint("fel key high %d, low %d\n", key_high, key_low);
    if((keyvalue <= key_high) && (keyvalue >= key_low))
    {
      AsciiPrint("fel key detected\n");
      return MISC_EFEX_MODE;
    }
  }
    
  //check recovery key
  ret1 = script_parser_fetch("recovery_key", "key_max", &key_high, 1);
  ret2 = script_parser_fetch("recovery_key", "key_min", &key_low,  1);
  if((ret1) || (ret2))
  {
    AsciiPrint("cant find recovery_key config value\n");
  }
  else
  {
    AsciiPrint("recovery key high %d, low %d\n", key_high, key_low);
    if((keyvalue >= key_low) && (keyvalue <= key_high))
    {
      AsciiPrint("key found, android recovery\n");
      return ANDROID_RECOVERY_MODE;
    }
  }

  //check fastboot key
  ret1 = script_parser_fetch("fastboot_key", "key_max", &key_high, 1);
  ret2 = script_parser_fetch("fastboot_key", "key_min", &key_low, 1);
  if((ret1) || (ret2))
  {
    AsciiPrint("cant find fastboot_key value\n");
  }
  else
  {
    AsciiPrint("fastboot key high %d, low %d\n", key_high, key_low);
    if((keyvalue >= key_low) && (keyvalue <= key_high))
    {
      AsciiPrint("key found, android fastboot\n");
      return ANDROID_FASTBOOT_MODE;
    }
  }
  
  return BOOT_NULL_MODE;
}


static EFI_STATUS BootCommandHandler(void)
{
  UINTN BootMode = BOOT_NULL_MODE;
  CHAR8 BootCmd[64] = {0};
  EFI_STATUS Status;

  AsciiStrCpy(BootCmd,"boot-normal");
  BootMode = GetPmuBootMode();
  if(BootMode == PMU_PRE_FASTBOOT_MODE || BootMode == PMU_PRE_RECOVERY_MODE)
  {

  }
  else
  {
    BootMode = GetKeyBootMode();
  }

  if(BootMode == MISC_EFEX_MODE)
  {
    AsciiPrint("find efex cmd\n");
    SunxiSetFelKey();
    AsciiPrint("WatchDog reset cpu\n");
    SunxiWatchDogReset();
  }

  switch(BootMode)
  {
    case PMU_PRE_FASTBOOT_MODE:
      AsciiPrint("PMU : ready to enter fastboot mode\n");
      AsciiStrCpy(BootCmd, "fastboot");
      break;
    case PMU_PRE_RECOVERY_MODE:
      AsciiPrint("PMU : find boot-recovery cmd,will boot recovery\n");
      AsciiStrCpy(BootCmd, "boot-recovery");
      break;
    case USER_SELECT_MODE:
      break;
    case MISC_EFEX_MODE:
      AsciiPrint("find efex cmd\n");
      break;
    case MISC_RESIGNATURE_MODE:
      AsciiPrint("find boot-resignature cmd\n");
      break;
    case ANDROID_RECOVERY_MODE:
    case MISC_BOOT_RECOVERY_MODE:
      AsciiPrint("find boot-recovery cmd, will boot recovery\n");
      AsciiStrCpy(BootCmd, "boot-recovery");
      break;
    case ANDROID_FASTBOOT_MODE:
    case MISC_FASTBOOT_MODE:
      AsciiPrint("Fastboot detected, will boot fastboot\n");
      AsciiStrCpy(BootCmd, "fastboot");
      break;
    case MISC_USB_RECOVERY_MODE:
      AsciiPrint("find usb-recovery cmd, will boot recovery\n");
      AsciiStrCpy(BootCmd, "boot-recovery");
      break;
  }
  if(AsciiStrLen(BootCmd))
  {
    Status = gRT->SetVariable (
      L"SunxiBootCmd",
      &gSunxiVariableGuid,
      (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
      AsciiStrLen(BootCmd)+1, //add '\0' in the end
      BootCmd);
    if(EFI_ERROR(Status))
    {
      BOOTCMDDXE_ERROR("Set Sunxi BootCmd Variable Error\n");
      return Status;
    }
    AsciiPrint("Sunxi BootCmd = %a\n", BootCmd);     
  }
  return EFI_SUCCESS;
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
BootCmmandDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status=EFI_SUCCESS;

  if(SunxiGetBootWorkMode() != WORK_MODE_BOOT)
  {
    DEBUG((EFI_D_INFO, "It's not work mode boot\n"));
    return EFI_SUCCESS;
  }

  Status = CheckUserInput();
  if(Status == EFI_SUCCESS)
  {
    return Status;
  }

  Status = gBS->LocateProtocol(&gAxpPowerProtocolGuid, NULL, (VOID **)&AxpPower);
  if (EFI_ERROR (Status)) 
  {
    DEBUG((EFI_D_INFO, "NandDxe:Find FlashIo Portocol , So Maybe Emmc exist\n"));
    return Status;
  }
  BOOTCMDDXE_INFO ("AxpPowerID = %d\n", AxpPower->AxpPowerId);

  Status = BootCommandHandler();

  return Status;
}

