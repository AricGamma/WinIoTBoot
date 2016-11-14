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

#ifndef _PRIVATE_H_
#define _PRIVATE_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/SunxiBootInfoLib.h>


#define printf(FMT, ARG...)        DEBUG (( DEBUG_ERROR,FMT,##ARG))
#define sprintf(buffer,fmt,arg...) AsciiSPrint(buffer,sizeof(buffer),fmt,##arg)

extern void * memcpy(void *dest, const void *src, UINT32 count);
//#define memcpy(x,y,z)         CopyMem((x),(y),(z))
#define memcmp(x,y,z)         CompareMem((x),(y),(z))
//note : memset & SetMem para order are not the same
#define memset(x,y,z)         SetMem((x),(z),(y))

#define malloc(x)             AllocateZeroPool((x))
#define free(x)               FreePool((x))

//string 
#define strcmp(x,y)           AsciiStrCmp((x),(y))
#define strcpy(x,y)           AsciiStrCpy((x),(y))
#define strlen(x)             AsciiStrLen((x))
#define strncmp(x,y,z)        AsciiStrnCmp((x),(y),(z))

#define readl(addr) MmioRead32((u32)(addr))
#define writel(val,addr) MmioWrite32((u32)(addr),(val))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))



#endif