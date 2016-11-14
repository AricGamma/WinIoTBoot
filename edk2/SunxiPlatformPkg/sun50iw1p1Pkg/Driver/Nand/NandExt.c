/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Jerry Wang <wangflord@allwinnertech.com>
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
#include <Library/UefiLib.h>

#include <Library/NandLib.h>
#include <Library/SunxiMbr.h>
#include <Library/SunxiCheckLib.h>
#include <Interinc/sunxi_uefi.h>

extern int NAND_UbootInit(int boot_mode);
extern int NAND_UbootToPhy(void);
extern int NAND_UbootExit(void);
extern int NAND_BurnBoot0(uint length, void *buffer);
extern int NAND_BurnUboot(uint length, void *buffer);
extern int NAND_PhyInit(void);
extern int NAND_PhyExit(void);
extern int NAND_Uboot_Erase(int erase_flag);
extern int NAND_UbootProbe(void);
extern int NAND_GetParam_store(void *buffer, uint length);
extern int NAND_FlushCache(void);
extern int  NAND_EraseChip_force(void);

extern PARTITION_MBR nand_mbr;
extern int  mbr_burned_flag;

static int  nand_open_times=0;

#define debug(fmt,args...)  //AsciiPrint(fmt ,##args)

int Nand_Get_Mbr(char* buffer, uint len)
{
  int i,j;

  sunxi_mbr_t *mbr = (sunxi_mbr_t *)buffer;

  nand_mbr.PartCount = mbr->PartCount +1;
  nand_mbr.array[0].addr = 0;
  nand_mbr.array[0].len = 32*1024;
  nand_mbr.array[0].user_type = 0x8000;
  nand_mbr.array[0].classname[0] = 'm';
  nand_mbr.array[0].classname[1] = 'b';
  nand_mbr.array[0].classname[2] = 'r';
  nand_mbr.array[0].classname[3] = '\0';

  for(i=1; i<nand_mbr.PartCount; i++)
  {
    for(j=0;j<16;j++)
      nand_mbr.array[i].classname[j] = mbr->array[i-1].name[j];

    nand_mbr.array[i].addr = nand_mbr.array[i-1].addr + nand_mbr.array[i-1].len;
    nand_mbr.array[i].len = mbr->array[i-1].lenlo;
    nand_mbr.array[i].user_type =mbr->array[i-1].user_type;
    if(i == 1)
      nand_mbr.array[0].user_type = nand_mbr.array[1].user_type;
  }

  for(j=0;j<16;j++)
    nand_mbr.array[nand_mbr.PartCount-1].classname[j] = mbr->array[nand_mbr.PartCount-2].name[j];

  nand_mbr.array[nand_mbr.PartCount-1].addr = nand_mbr.array[nand_mbr.PartCount-2].addr + nand_mbr.array[nand_mbr.PartCount-2].len;
  nand_mbr.array[nand_mbr.PartCount-1].len = 0;
  //nand_mbr.array[nand_mbr.PartCount-1].user_type = 0x0;

  //for DEBUG
  {
    AsciiPrint("total part: %d\n", nand_mbr.PartCount);
    for(i=0; i<nand_mbr.PartCount; i++)
    {
      AsciiPrint("%a %d, %x, %x\n",nand_mbr.array[i].classname,i, nand_mbr.array[i].len, nand_mbr.array[i].user_type);
    }
  }

  return 0;
}

int GetNandOpenedCnt(void)
{
  return nand_open_times;
}

int Nand_Uefi_Init(int boot_mode)
{
  if(!nand_open_times)
  {
    nand_open_times ++;
    AsciiPrint("NAND_UbootInit\n");

    return NAND_UbootInit(boot_mode);
  }
  AsciiPrint("nand already init\n");
  nand_open_times ++;

  return 0;
}

int Nand_Uefi_Exit(int force)
{
  if(!nand_open_times)
  {
    AsciiPrint("nand not opened\n");
    return 0;
  }
  if(force)
  {
    if(nand_open_times)
    {
      nand_open_times = 0;
      AsciiPrint("NAND_UbootExit\n");

      return NAND_UbootExit();
    }
  }
  AsciiPrint("nand not need closed\n");

  return 0;
}

int Nand_Uefi_Probe(void)
{
  debug("Nand_Uefi_probe\n");
  return NAND_UbootProbe();
}


int Nand_Download_Boot0(uint length, void *buffer)
{
  int ret;
  #if 0
  if(!NAND_PhyInit())
  {
    ret = NAND_BurnBoot0(length, buffer);
  }
  else
  {
    ret = -1;
  }

  NAND_PhyExit();
  #endif
  ret = NAND_BurnBoot0(length, buffer);
  return ret;
}

int Nand_Download_Uefi(uint length, void *buffer)
{
  int ret;

#if 0
  debug("nand_download_uboot\n");
  if(!NAND_PhyInit())
  {
    ret = NAND_BurnUboot(length, buffer);
    debug("nand burn uboot error ret = %d\n", ret);
  }
  else
  {
    debug("nand phyinit error\n");
    ret = -1;
  }

  NAND_PhyExit();
#endif
  ret = NAND_BurnUboot(length, buffer);
  return ret;
}

int Nand_Force_Download_Uefi(uint length,void *buffer)
{
  int ret = 0;
  if((length <= 0)||(buffer == NULL))
  {
    AsciiPrint("force download uboot error : the para is invaild \n");
    return -1;
  }
  AsciiPrint("force download uboot \n");
  ret = NAND_BurnUboot(length,buffer);
  return ret ;
}

int Nand_Uefi_Erase(int user_erase)
{
  return NAND_Uboot_Erase(user_erase);
}


uint Nand_Uefi_Get_Flash_Info(void *buffer, uint length)
{
  return NAND_GetParam_store(buffer, length);
}

uint Nand_Uefi_Set_Flash_Info(void *buffer, uint length)
{
  return 0;
}

uint Nand_Uefi_Get_Flash_Size(void)
{
  return get_nftl_cap();
}

int Nand_Uefi_flush(void)
{
  return NAND_FlushCache();
}


int NAND_Uboot_Force_Erase(void)
{
  AsciiPrint("force erase\n");
  if(NAND_PhyInit())
  {
    AsciiPrint("phy init fail\n");
    return -1;
  }

  NAND_EraseChip_force();

  NAND_PhyExit();

  return 0;
}

