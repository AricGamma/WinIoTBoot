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

#include "common.h"
#include "asm/io.h"
#include "asm/arch/efuse.h"

#define SID_OP_LOCK  (0xAC)

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
uint sid_read_key(uint key_index)
{
  uint reg_val;

  reg_val = readl(SID_PRCTL);
  reg_val &= ~((0x1ff<<16)|0x3);
  reg_val |= key_index<<16;
  writel(reg_val, SID_PRCTL);

  reg_val &= ~((0xff<<8)|0x3);
  reg_val |= (SID_OP_LOCK<<8) | 0x2;
  writel(reg_val, SID_PRCTL);

  while(readl(SID_PRCTL)&0x2){};

  reg_val &= ~((0x1ff<<16)|(0xff<<8)|0x3);
  writel(reg_val, SID_PRCTL);

  reg_val = readl(SID_RDKEY);

  return reg_val;
}
/*
*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void sid_read_rotpk(void *dst)
{
  uint chipid_index = 0x64;
  uint id_length = 32;
  uint i = 0;
  for(i = 0 ; i < id_length ;i+=4 )
  {
    *(u32*)dst  = sid_read_key(chipid_index + i );
    dst += 4 ;
  }
  return ;
}

