/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  sunny <sunny@allwinnertech.com>
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

#include  "Crc32.h"

unsigned int  calc_crc32(void * buffer, unsigned int  length)
{
  unsigned int i, j;
  CRC32_DATA_t crc32;   //
  unsigned int  CRC32 = 0xffffffff; //设置初始值

  crc32.CRC = 0;

  for( i = 0; i < 256; ++i)//用++i以提高效率
  {
    crc32.CRC = i;
    for( j = 0; j < 8 ; ++j)
    {
      //这个循环实际上就是用"计算法"来求取CRC的校验码
      if(crc32.CRC & 1)
        crc32.CRC = (crc32.CRC >> 1) ^ 0xEDB88320;
      else //0xEDB88320就是CRC-32多项表达式的值
        crc32.CRC >>= 1;
    }
    crc32.CRC_32_Tbl[i] = crc32.CRC;
  }

  CRC32 = 0xffffffff; //设置初始值
  for( i = 0; i < length; ++i)
  {
    CRC32 = crc32.CRC_32_Tbl[(CRC32^((unsigned char*)buffer)[i]) & 0xff] ^ (CRC32>>8);
  }

  //return CRC32;
  return CRC32^0xffffffff;
}

