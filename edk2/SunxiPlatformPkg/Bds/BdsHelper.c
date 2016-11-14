/** @file
*
*  Copyright (c) 2011 - 2014, ARM Limited. All rights reserved.
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

#include "BdsInternal.h"

EFI_STATUS
EditHIInputStr (
  IN OUT CHAR16  *CmdLine,
  IN     UINTN   MaxCmdLine
  )
{
  UINTN           CmdLineIndex;
  UINTN           WaitIndex;
  CHAR8           Char;
  EFI_INPUT_KEY   Key;
  EFI_STATUS      Status;

  // The command line must be at least one character long
  ASSERT (MaxCmdLine > 0);

  // Ensure the last character of the buffer is the NULL character
  CmdLine[MaxCmdLine - 1] = '\0';

  Print (CmdLine);

  // To prevent a buffer overflow, we only allow to enter (MaxCmdLine-1) characters
  for (CmdLineIndex = StrLen (CmdLine); CmdLineIndex < MaxCmdLine; ) {
    Status = gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &WaitIndex);
    ASSERT_EFI_ERROR (Status);

    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    ASSERT_EFI_ERROR (Status);

    // Unicode character is valid when Scancode is NUll
    if (Key.ScanCode == SCAN_NULL) {
      // Scan code is NUll, hence read Unicode character
      Char = (CHAR8)Key.UnicodeChar;
    } else {
      Char = CHAR_NULL;
    }

    if ((Char == CHAR_LINEFEED) || (Char == CHAR_CARRIAGE_RETURN) || (Char == 0x7f)) {
      CmdLine[CmdLineIndex] = '\0';
      Print (L"\r\n");

      return EFI_SUCCESS;
    } else if ((Key.UnicodeChar == L'\b') || (Key.ScanCode == SCAN_LEFT) || (Key.ScanCode == SCAN_DELETE)){
      if (CmdLineIndex != 0) {
        CmdLineIndex--;
        Print (L"\b \b");
      }
    } else if ((Key.ScanCode == SCAN_ESC) || (Char == 0x1B) || (Char == 0x0)) {
      return EFI_INVALID_PARAMETER;
    } else if (CmdLineIndex < (MaxCmdLine-1)) {
      CmdLine[CmdLineIndex++] = Key.UnicodeChar;
      Print (L"%c", Key.UnicodeChar);
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetHIInputStr (
  IN OUT CHAR16  *CmdLine,
  IN     UINTN   MaxCmdLine
  )
{
  EFI_STATUS  Status;

  // For a new input just passed an empty string
  CmdLine[0] = L'\0';

  Status = EditHIInputStr (CmdLine, MaxCmdLine);

  return Status;
}

EFI_STATUS
EditHIInputAscii (
  IN OUT CHAR8   *CmdLine,
  IN     UINTN   MaxCmdLine
  )
{
  CHAR16*     Str;
  EFI_STATUS  Status;

  Str = (CHAR16*)AllocatePool (MaxCmdLine * sizeof(CHAR16));
  AsciiStrToUnicodeStr (CmdLine, Str);

  Status = EditHIInputStr (Str, MaxCmdLine);
  if (!EFI_ERROR(Status)) {
    UnicodeStrToAsciiStr (Str, CmdLine);
  }
  FreePool (Str);

  return Status;
}

EFI_STATUS
GetHIInputAscii (
  IN OUT CHAR8   *CmdLine,
  IN     UINTN   MaxCmdLine
  )
{
  // For a new input just passed an empty string
  CmdLine[0] = '\0';

  return EditHIInputAscii (CmdLine,MaxCmdLine);
}

EFI_STATUS
GetHIInputInteger (
  OUT UINTN   *Integer
  )
{
  CHAR16      CmdLine[255];
  EFI_STATUS  Status;

  CmdLine[0] = '\0';
  Status = EditHIInputStr (CmdLine, 255);
  if (!EFI_ERROR(Status)) {
    *Integer = StrDecimalToUintn (CmdLine);
  }

  return Status;
}

EFI_STATUS
GetHIInputIP (
  OUT EFI_IP_ADDRESS   *Ip
  )
{
  CHAR16  CmdLine[255];
  CHAR16  *Str;
  EFI_STATUS  Status;

  CmdLine[0] = '\0';
  Status = EditHIInputStr (CmdLine,255);
  if (!EFI_ERROR(Status)) {
    Str = CmdLine;
    Ip->v4.Addr[0] = (UINT8)StrDecimalToUintn (Str);

    Str = StrStr (Str, L".");
    if (Str == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    Ip->v4.Addr[1] = (UINT8)StrDecimalToUintn (++Str);

    Str = StrStr (Str, L".");
    if (Str == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    Ip->v4.Addr[2] = (UINT8)StrDecimalToUintn (++Str);

    Str = StrStr (Str, L".");
    if (Str == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    Ip->v4.Addr[3] = (UINT8)StrDecimalToUintn (++Str);
  }

  return Status;
}

EFI_STATUS
GetHIInputBoolean (
  OUT BOOLEAN *Value
  )
{
  CHAR16      CmdBoolean[2];
  EFI_STATUS  Status;

  while(1) {
    Print (L"[y/n] ");
    Status = GetHIInputStr (CmdBoolean, 2);
    if (EFI_ERROR(Status)) {
      return Status;
    } else if ((CmdBoolean[0] == L'y') || (CmdBoolean[0] == L'Y')) {
      if (Value) *Value = TRUE;
      return EFI_SUCCESS;
    } else if ((CmdBoolean[0] == L'n') || (CmdBoolean[0] == L'N')) {
      if (Value) *Value = FALSE;
      return EFI_SUCCESS;
    }
  }
}

BOOLEAN
HasFilePathEfiExtension (
  IN CHAR16* FilePath
  )
{
  return (StrCmp (FilePath + (StrSize (FilePath) / sizeof (CHAR16)) - 5, L".EFI") == 0) ||
         (StrCmp (FilePath + (StrSize (FilePath) / sizeof (CHAR16)) - 5, L".efi") == 0);
}

// Return the last non end-type Device Path Node from a Device Path
EFI_DEVICE_PATH*
GetLastDevicePathNode (
  IN EFI_DEVICE_PATH*  DevicePath
  )
{
  EFI_DEVICE_PATH*     PrevDevicePathNode;

  PrevDevicePathNode = DevicePath;
  while (!IsDevicePathEndType (DevicePath)) {
    PrevDevicePathNode = DevicePath;
    DevicePath = NextDevicePathNode (DevicePath);
  }

  return PrevDevicePathNode;
}

EFI_STATUS
GenerateDeviceDescriptionName (
  IN  EFI_HANDLE  Handle,
  IN OUT CHAR16*  Description
  )
{
  EFI_STATUS                        Status;
  EFI_COMPONENT_NAME_PROTOCOL*      ComponentName2Protocol;
  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL* DevicePathToTextProtocol;
  EFI_DEVICE_PATH_PROTOCOL*         DevicePathProtocol;
  CHAR16*                           DriverName;
  CHAR16*                           DevicePathTxt;
  EFI_DEVICE_PATH*                  DevicePathNode;

  ComponentName2Protocol = NULL;
  Status = gBS->HandleProtocol (Handle, &gEfiComponentName2ProtocolGuid, (VOID **)&ComponentName2Protocol);
  if (!EFI_ERROR(Status)) {
    //TODO: Fixme. we must find the best langague
    Status = ComponentName2Protocol->GetDriverName (ComponentName2Protocol,"en",&DriverName);
    if (!EFI_ERROR(Status)) {
      StrnCpy (Description, DriverName, BOOT_DEVICE_DESCRIPTION_MAX);
    }
  }

  if (EFI_ERROR(Status)) {
    // Use the lastest non null entry of the Device path as a description
    Status = gBS->HandleProtocol (Handle, &gEfiDevicePathProtocolGuid, (VOID **)&DevicePathProtocol);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    // Convert the last non end-type Device Path Node in text for the description
    DevicePathNode = GetLastDevicePathNode (DevicePathProtocol);
    Status = gBS->LocateProtocol (&gEfiDevicePathToTextProtocolGuid, NULL, (VOID **)&DevicePathToTextProtocol);
    ASSERT_EFI_ERROR(Status);
    DevicePathTxt = DevicePathToTextProtocol->ConvertDevicePathToText (DevicePathNode, TRUE, TRUE);
    StrnCpy (Description, DevicePathTxt, BOOT_DEVICE_DESCRIPTION_MAX);
    FreePool (DevicePathTxt);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
BdsStartBootOption (
  IN CHAR16* BootOption
  )
{
  EFI_STATUS          Status;
  BDS_LOAD_OPTION     *BdsLoadOption;

  Status = BootOptionFromLoadOptionVariable (BootOption, &BdsLoadOption);
  if (!EFI_ERROR(Status)) {
    Status = BootOptionStart (BdsLoadOption);
    FreePool (BdsLoadOption);

    if (!EFI_ERROR(Status)) {
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_NOT_STARTED;
    }
  } else {
    Status = EFI_NOT_FOUND;
  }
  return Status;
}

UINTN
GetUnalignedDevicePathSize (
  IN EFI_DEVICE_PATH* DevicePath
  )
{
  UINTN Size;
  EFI_DEVICE_PATH* AlignedDevicePath;

  if ((UINTN)DevicePath & 0x1) {
    AlignedDevicePath = DuplicateDevicePath (DevicePath);
    Size = GetDevicePathSize (AlignedDevicePath);
    FreePool (AlignedDevicePath);
  } else {
    Size = GetDevicePathSize (DevicePath);
  }
  return Size;
}

EFI_DEVICE_PATH*
GetAlignedDevicePath (
  IN EFI_DEVICE_PATH* DevicePath
  )
{
  if ((UINTN)DevicePath & 0x1) {
    return DuplicateDevicePath (DevicePath);
  } else {
    return DevicePath;
  }
}

BOOLEAN
IsUnicodeString (
  IN VOID* String
  )
{
  // We do not support NULL pointer
  ASSERT (String != NULL);

  if (*(CHAR16*)String < 0x100) {
    //Note: We could get issue if the string is an empty Ascii string...
    return TRUE;
  } else {
    return FALSE;
  }
}

/*
 * Try to detect if the given string is an ASCII or Unicode string
 *
 * There are actually few limitation to this function but it is mainly to give
 * a user friendly output.
 *
 * Some limitations:
 *   - it only supports unicode string that use ASCII character (< 0x100)
 *   - single character ASCII strings are interpreted as Unicode string
 *   - string cannot be longer than 2 x BOOT_DEVICE_OPTION_MAX (600 bytes)
 *
 * @param String    Buffer that might contain a Unicode or Ascii string
 * @param IsUnicode If not NULL this boolean value returns if the string is an
 *                  ASCII or Unicode string.
 */
BOOLEAN
IsPrintableString (
  IN  VOID*    String,
  OUT BOOLEAN *IsUnicode
  )
{
  BOOLEAN UnicodeDetected;
  BOOLEAN IsPrintable;
  UINTN Index;
  CHAR16 Character;

  // We do not support NULL pointer
  ASSERT (String != NULL);

  // Test empty string
  if (*(CHAR16*)String == L'\0') {
    if (IsUnicode) {
      *IsUnicode = TRUE;
    }
    return TRUE;
  } else if (*(CHAR16*)String == '\0') {
    if (IsUnicode) {
      *IsUnicode = FALSE;
    }
    return TRUE;
  }

  // Limitation: if the string is an ASCII single character string. This comparison
  // will assume it is a Unicode string.
  if (*(CHAR16*)String < 0x100) {
    UnicodeDetected = TRUE;
  } else {
    UnicodeDetected = FALSE;
  }

  IsPrintable = FALSE;
  for (Index = 0; Index < BOOT_DEVICE_OPTION_MAX * 2; Index++) {
    if (UnicodeDetected) {
      Character = ((CHAR16*)String)[Index];
    } else {
      Character = ((CHAR8*)String)[Index];
    }

    if (Character == '\0') {
      // End of the string
      IsPrintable = TRUE;
      break;
    } else if ((Character < 0x20) || (Character > 0x7f)) {
      // We only support the range of printable ASCII character
      IsPrintable = FALSE;
      break;
    }
  }

  if (IsPrintable && IsUnicode) {
    *IsUnicode = UnicodeDetected;
  }

  return IsPrintable;
}
