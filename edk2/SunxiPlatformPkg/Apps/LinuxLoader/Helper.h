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
#ifndef _LINUX_LOADER_HELPER_H_
#define _LINUX_LOADER_HELPER_H_


VOID UpdateScriptDramParam(VOID* DramParaAdd);
VOID UpdateScriptStorageParam(UINT32 StorageType);
void SetScriptForLinux();
EFI_STATUS GetBlockIoInstanceByStorage(INT32 StorageType,  EFI_BLOCK_IO_PROTOCOL  **Instance);
EFI_STATUS SunxiReadLinuxImage(EFI_BLOCK_IO_PROTOCOL* pBlockIo ,UINT32 start, EFI_PHYSICAL_ADDRESS  ImageAddr);
EFI_STATUS GetCmdLine(CHAR8 *CmdLine,CHAR8 *PartInof, UINT32 StorageType);
VOID PrintPerformance (VOID);
#endif