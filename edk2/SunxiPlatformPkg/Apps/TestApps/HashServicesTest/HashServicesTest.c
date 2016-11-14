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

#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UncachedMemoryAllocationLib.h>

#include <Protocol/Hash.h>
#include <Protocol/PciIo.h>

#define NOT_EXTEND  0
#define EXTEND    1

#define TEST_HASH 0x56565656

EFI_STATUS
EFIAPI
HashTestMain (
  IN EFI_HANDLE   ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_HASH_PROTOCOL *HashProtocol;
  UINTN HashSize;
  UINT8 *Message;
  UINT32 *Phash;
  UINT32 Count, Test[5];
  EFI_HASH_OUTPUT *Hash;

  DEBUG((EFI_D_ERROR, "[HashTestApps] : HashTestApps\n"));

  for (Count = 0 ; Count < 5; Count++)
    Test[Count] = TEST_HASH;

  Status = gBS->LocateProtocol(&gEfiHashProtocolGuid, NULL, (VOID **)&HashProtocol);
  ASSERT_EFI_ERROR(Status);

  HashProtocol->GetHashSize(HashProtocol, &gEfiHashAlgorithmSha1Guid, &HashSize);

  Message = (UINT8 *)UncachedAllocatePool(HashSize);
  Hash = (EFI_HASH_OUTPUT *)UncachedAllocatePool(sizeof(EFI_HASH_OUTPUT));
  gBS->CopyMem(Hash, Test, sizeof(EFI_SHA1_HASH));
  gBS->CopyMem(Message, "abc", sizeof("abc"));

  HashProtocol->Hash(HashProtocol, &gEfiHashAlgorithmSha1Guid, NOT_EXTEND, Message, (UINT64)(sizeof("abc")-1), Hash);
  Phash = (UINT32*)Hash;
  
  DEBUG((EFI_D_ERROR, "[Hash SHA1] : Result = ", (EFI_SHA1_HASH *)Hash->Sha1Hash));
  for(Count=0;Count<5;Count++)
  DEBUG((EFI_D_ERROR, "%x", *Phash++));
  DEBUG((EFI_D_ERROR, "\n"));

  HashProtocol->Hash(HashProtocol, &gEfiHashAlgorithmSha256Guid, NOT_EXTEND, Message, (UINT64)(sizeof("abc")-1), Hash);
  Phash = (UINT32*)Hash;
  DEBUG((EFI_D_ERROR, "[Hash SHA256] : Result = ", (EFI_SHA256_HASH *)Hash->Sha1Hash));
  for(Count=0;Count<sizeof(EFI_SHA256_HASH)/4;Count++)
  DEBUG((EFI_D_ERROR, "%x", *Phash++));
  DEBUG((EFI_D_ERROR, "\n"));

  HashProtocol->Hash(HashProtocol, &gEfiHashAlgorithmMD5Guid, NOT_EXTEND, Message, (UINT64)(sizeof("abc")-1), Hash);
  Phash = (UINT32*)Hash;
  DEBUG((EFI_D_ERROR, "[Hash MD5] : Result = ", (EFI_MD5_HASH *)Hash->Sha1Hash));
  for(Count=0;Count<sizeof(EFI_MD5_HASH)/4;Count++)
  DEBUG((EFI_D_ERROR, "%x", *Phash++));
  DEBUG((EFI_D_ERROR, "\n"));

  return Status;
}

