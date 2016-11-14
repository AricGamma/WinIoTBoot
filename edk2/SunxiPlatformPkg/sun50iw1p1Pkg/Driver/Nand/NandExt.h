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
#ifndef __NAND_EXT_LIB_H__
#define __NAND_EXT_LIB_H__

#include <Sunxi_type/Sunxi_type.h>

int Nand_Get_Mbr(char* buffer, uint len);
int Nand_Uefi_Init(int boot_mode);
int Nand_Uefi_Exit(int force);
int Nand_Uefi_Probe(void);
int Nand_Download_Boot0(uint length, void *buffer);
int Nand_Download_Uefi(uint length, void *buffer);
int Nand_Force_Download_Uefi(uint length,void *buffer);
int Nand_Uefi_Erase(int user_erase);
uint Nand_Uefi_Get_Flash_Info(void *buffer, uint length);
uint Nand_Uefi_Set_Flash_Info(void *buffer, uint length);
uint Nand_Uefi_Get_Flash_Size(void);
int Nand_Uefi_flush(void);
int NAND_Uboot_Force_Erase(void);
int GetNandOpenedCnt(void);

#endif