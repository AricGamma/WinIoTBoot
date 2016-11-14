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

#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/ArmGicLib.h>
#include <Library/UncachedMemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/Hash.h>
#include <Sun8iW1/platform.h>
#include <Sun8iW1/ccmu.h>

#include "HashDxe.h"
#include "Ss_hal.h"
#include "Sha256.h"

#define SS_CTL        (SS_BASE + 0x00)
#define SS_KEY        (SS_BASE + 0x04)  
#define SS_IV         (SS_BASE + 0x24)
#define SS_CNT        (SS_BASE + 0x34)  
#define SS_FCSR       (SS_BASE + 0x44)  
#define SS_ICSR       (SS_BASE + 0x48)
#define SS_MD         (SS_BASE + 0x4C)
#define SS_CTS_LEN        (SS_BASE + 0x60)
#define SS_RXFIFO     (SS_BASE + 0x200)
#define SS_TXFIFO     (SS_BASE + 0x204)


#define CLOCK_ON  1
#define CLOCK_OFF 0

#define SS_CLK_SELECT   1   //0: HOSC 24M, 1: PLL6

EFI_STATUS
EFIAPI
CryptoClockGating (
  IN UINT32 clock_status
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32 reg_val;

  reg_val = MmioRead32(CCM_SS_SCLK_CTRL);
  #if (SS_CLK_SELECT == 0)
    reg_val &= ~(0x3<<24);
    reg_val &= ~(0x3<<16);
    reg_val &= ~(0xf);
    reg_val |= 0x0<<16;     
    reg_val |= 0;       
  #elif(SS_CLK_SELECT == 1 )
    reg_val &= ~(0x3<<24);
    reg_val |= 0x1<<24;
    reg_val &= ~(0x3<<16);
    reg_val &= ~(0xf);  
    reg_val |= 0x0<<16;     // /1
    reg_val |= (4 -1);      // /4
  #else
    reg_val &= ~(0x3<<24);
    reg_val |= 0x2<<24;
    reg_val &= ~(0x3<<16);
    reg_val &= ~(0xf);  
    reg_val |= 0x0<<16;     // /1
    reg_val |= (8-1); 
  #endif  
  
  if (clock_status == CLOCK_ON){
    MmioWrite32(CCM_SS_SCLK_CTRL, reg_val | 0x1U<<31);
    MmioOr32(CCM_AHB1_GATE0_CTRL,0x1<<5); //open ss ahb gating
    MmioOr32(CCM_AHB1_RST_REG0, 0x1<<5);
  }
  else if (clock_status == CLOCK_OFF){
    MmioWrite32(CCM_SS_SCLK_CTRL, reg_val & ~0x1U<<31);
    MmioAnd32(CCM_AHB1_GATE0_CTRL,~0x1<<5);//close ss ahb gating
    MmioAnd32(CCM_AHB1_RST_REG0, ~0x1<<5);
  }
  else {
    DEBUG((EFI_D_ERROR, "[HashDxe] : Unsupported SSS clock status\n"));
    Status = EFI_INVALID_PARAMETER;
  }

  return Status;
}

/**
  Returns the size of the hash which results from a specific algorithm.

  @param[in]  This                  Points to this instance of EFI_HASH_PROTOCOL.
  @param[in]  HashAlgorithm         Points to the EFI_GUID which identifies the algorithm to use.
  @param[out] HashSize              Holds the returned size of the algorithm's hash.

  @retval EFI_SUCCESS           Hash size returned successfully.
  @retval EFI_UNSUPPORTED       The algorithm specified by HashAlgorithm is not supported 
                                by this driver.

**/
EFI_STATUS
EFIAPI
HashDxeGetHashSize (
  IN  CONST EFI_HASH_PROTOCOL     *This,
  IN  CONST EFI_GUID              *HashAlgorithm,
  OUT UINTN                       *HashSize
  )
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmSha1Guid))
    *HashSize = sizeof(EFI_SHA1_HASH);
  else if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmSha256Guid))
    *HashSize = sizeof(EFI_SHA256_HASH);
  else {
    DEBUG((EFI_D_ERROR, "[HashDxe] : Unsupported Hash Algorithm\n"));
    Status = EFI_UNSUPPORTED;
  }

  return Status;
}

/**
  Returns the size of the hash which results from a specific algorithm.

  @param[in]  This          Points to this instance of EFI_HASH_PROTOCOL.
  @param[in]  HashAlgorithm Points to the EFI_GUID which identifies the algorithm to use.
  @param[in]  Extend        Specifies whether to create a new hash (FALSE) or extend the specified
                            existing hash (TRUE).
  @param[in]  Message       Points to the start of the message.
  @param[in]  MessageSize   The size of Message, in bytes.
  @param[in,out]  Hash      On input, if Extend is TRUE, then this holds the hash to extend. On
                            output, holds the resulting hash computed from the message.

  @retval EFI_SUCCESS           Hash returned successfully.
  @retval EFI_UNSUPPORTED       The algorithm specified by HashAlgorithm is not supported by this
                                 driver. Or, Extend is TRUE, and the algorithm doesn't support extending the hash.

**/
EFI_STATUS
EFIAPI
HashDxeRunHash (
  IN CONST EFI_HASH_PROTOCOL  *This,
  IN CONST EFI_GUID   *HashAlgorithm,
  IN BOOLEAN      Extend,
  IN CONST UINT8    *Message,
  IN UINT64     MessageSize,
  IN OUT EFI_HASH_OUTPUT  *Hash
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32 IoBuf[16];
  UINT32 TotalMessageSize=0;
  UINT32 HashResult[8];
  UINT8 *TempMessage=0;

  if (Extend == 0)
    TotalMessageSize = MessageSize;
  else if (Extend == 1) {
    if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmSha1Guid)) {
      TempMessage = (UINT8 *)UncachedAllocatePool(sizeof(EFI_SHA1_HASH) + MessageSize);
      CopyMem(TempMessage, Hash, sizeof(EFI_SHA1_HASH));
      CopyMem(TempMessage + sizeof(EFI_SHA1_HASH), Message, MessageSize);
      TotalMessageSize = sizeof(EFI_SHA1_HASH) + MessageSize;
    } else if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmSha256Guid)) {
      TempMessage = (UINT8 *)UncachedAllocatePool(sizeof(EFI_SHA256_HASH) + MessageSize);
      CopyMem(TempMessage, Hash, sizeof(EFI_SHA256_HASH));
      CopyMem(TempMessage + sizeof(EFI_SHA256_HASH), Message, MessageSize);
      TotalMessageSize = sizeof(EFI_SHA256_HASH) + MessageSize;
    } else if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmMD5Guid)){
        TempMessage = (UINT8 *)UncachedAllocatePool(sizeof(EFI_SHA256_HASH) + MessageSize);
      CopyMem(TempMessage, Hash, sizeof(EFI_MD5_HASH));
      CopyMem(TempMessage + sizeof(EFI_MD5_HASH), Message, MessageSize);
      TotalMessageSize = sizeof(EFI_MD5_HASH) + MessageSize;
    }
  }
  
  if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmSha1Guid)){
    IoBuf[0] = 0; //0: SHA1, 1: MD5 
    IoBuf[1] = 0x0; //SHA1/MD5 use constants  
    ss_io_ctl(SSIO_SETUP_SHA1_MD5, IoBuf);  
    ss_io_ctl(SSIO_START, 0); //start SS engine
    if (Extend == 0)
      ss_sha1md5_text(TotalMessageSize,(UINT32*)Message);
    else if (Extend == 1)
      ss_sha1md5_text(TotalMessageSize,(UINT32*)TempMessage);
      
    ss_sha1md5_dataend();//send end text command to SHA1/MD5 engine
    ss_get_md(HashResult);//get the sha1 result
    ss_io_ctl(SSIO_STOP, 0);  //stop SS engine
  }
  else if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmSha256Guid)){
    if (Extend == 0)
      sha256(Message,TotalMessageSize,(UINT8*)HashResult);
    else if (Extend == 1)
      sha256(TempMessage,TotalMessageSize,(UINT8*)HashResult);    
  }
  else if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmMD5Guid)){
    IoBuf[0] = 1; //0: SHA1, 1: MD5 
    IoBuf[1] = 0x0; //SHA1/MD5 use constants  
    ss_io_ctl(SSIO_SETUP_SHA1_MD5, IoBuf);  
    ss_io_ctl(SSIO_START, 0); //start SS engine
    if (Extend == 0)
      ss_sha1md5_text(TotalMessageSize,(UINT32*)Message);
    else if (Extend == 1)
      ss_sha1md5_text(TotalMessageSize,(UINT32*)TempMessage);
      
    ss_sha1md5_dataend();//send end text command to SHA1/MD5 engine
    ss_get_md(HashResult);//get the sha1 result
    ss_io_ctl(SSIO_STOP, 0);  //stop SS engine
  }
  else {
    DEBUG((EFI_D_ERROR, "[HashDxe] : Unsupported Hash Algorithm\n"));
    Status = EFI_UNSUPPORTED;
    return Status;
  }

  if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmSha1Guid))
    CopyMem(Hash, HashResult, sizeof(EFI_SHA1_HASH));
  else if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmSha256Guid))
    CopyMem(Hash, HashResult, sizeof(EFI_SHA256_HASH));
  else if (CompareGuid(HashAlgorithm, &gEfiHashAlgorithmMD5Guid))
    CopyMem(Hash, HashResult, sizeof(EFI_MD5_HASH));
  else {
    DEBUG((EFI_D_ERROR, "[HashDxe] : Unsupported Hash Algorithm\n"));
    Status = EFI_UNSUPPORTED;
    return Status;
  }

  return Status;
}

EFI_HASH_PROTOCOL gHash = {
  HashDxeGetHashSize,
  HashDxeRunHash
};

/**
  Initialize the state information for the HashDxe

  @param  ImageHandle   of the loaded driver
  @param  SystemTable   Pointer to the System Table

  @retval EFI_SUCCESS           Protocol registered
  @retval EFI_OUT_OF_RESOURCES  Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR      Hardware problems

**/
EFI_STATUS
EFIAPI
HashDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
//  volatile int dbg=0x55;
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32 i;
  UINT32 IoBuf[16];
  
  DEBUG((EFI_D_INFO, "++%a:%d\n", __FUNCTION__, __LINE__));
  CryptoClockGating(CLOCK_ON);

  //set SHA1/MD5 mode 
  IoBuf[0] = 0; //0: SHA1, 1: MD5 
  IoBuf[1] = 0x0; //SHA1/MD5 use constants  
  ss_io_ctl(SSIO_SETUP_SHA1_MD5, IoBuf);  //set IV value (use orginal initial array) (A,B,C,D for MD5; H0,H1,H2,H3,H4 for SHA1) 
  IoBuf[0] = 5; 
  for(i=1; i<6; i++)    
    IoBuf[i] = 0; 
  
  ss_io_ctl(SSIO_SET_IV, IoBuf);  
  ss_io_ctl(SSIO_START, IoBuf); //start SS engine
  ss_io_ctl(SSIO_STOP, IoBuf);  //stop SS engine to clear the fifo

  Status = gBS->InstallMultipleProtocolInterfaces(
      &ImageHandle,
      &gEfiHashProtocolGuid,
      &gHash,
      NULL
      );
  DEBUG((EFI_D_INFO, "++%a:%d\n", __FUNCTION__, __LINE__));
  return Status;
}
