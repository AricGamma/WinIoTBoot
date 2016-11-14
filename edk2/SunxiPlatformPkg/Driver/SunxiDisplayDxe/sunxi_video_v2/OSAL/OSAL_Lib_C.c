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


#include "OSAL.h"
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>   
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>


extern int kdb_trap_printk;
/* 普通内存分配 */
void * OSAL_malloc(__u32 Size)
{
    void * addr;

    addr = AllocatePool(Size);
    ZeroMem(addr,Size);
  return addr;
}

void OSAL_free(void *pAddr)
{
    FreePool(pAddr);

    return;
}

/* 连续的物理内存分配 */
void * OSAL_PhyAlloc(__u32 Size)
{
    void * addr;

    addr = AllocatePool(Size);
    ZeroMem(addr,Size);
  return addr;
}

void OSAL_PhyFree(void *pAddr, __u32 Size)
{
   FreePool(pAddr);

   return;
}


/* 虚拟内存和物理内存之间的转化 */
unsigned int OSAL_VAtoPA(void *va)
{
  //if((unsigned int)(va) > 0x40000000)
        //return (unsigned int)(va) - 0x40000000;

  return (unsigned int)va;
    //return virt_to_phys(va);
}

void *OSAL_PAtoVA(unsigned int pa)
{
  return (void *)pa;
    //return phys_to_virt(pa);
}





int OSAL_putchar(int value)
{
  return 0;
}
int OSAL_puts(const char * value)
{
  return 0;
}
int OSAL_getchar(void)
{
  return 0;
}
char * OSAL_gets(char *value)
{
  return NULL;
}

//----------------------------------------------------------------
//  实用函数
//----------------------------------------------------------------
/* 字符串转长整形 */
long OSAL_strtol (const char *str, const char **err, int base)
{
  return 0;
}

/* 有符号十进制整形转字符串*/
void OSAL_int2str_dec(int input, char * str)
{
}

/* 十六进制整形转字符串*/
void OSAL_int2str_hex(int input, char * str, int hex_flag)
{
}

/* 无符号十进制整形转字符串*/
void OSAL_uint2str_dec(unsigned int input, char * str)
{
}



