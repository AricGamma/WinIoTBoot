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

#include  <Uefi.h>
#include <Interinc/sunxi_uefi.h>

typedef struct tag_CRC32_DATA
{
  UINT32 CRC;       //在Windows下编程，int的大小是32位
  UINT32 CRC_32_Tbl[256]; //用来保存码表
}CRC32_DATA_t;

unsigned int  SunxiCrc32(const void * buffer, UINT32  length)
{
  UINT32 i, j;
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


UINT32 SunxiSpriteGenerateChecksum(void *buffer, UINT32 length, UINT32 src_sum)
{
#ifndef CONFIG_USE_NEON_SIMD
  UINT32 *buf;
  UINT32 count;
  UINT32 sum;

  /* 生成校验和 */
  count = length >> 2;                       // 以 字（4bytes）为单位计数
  sum = 0;
  buf = (UINT32 *)buffer;
  do
  {
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
  }while( ( count -= 4 ) > (4-1) );

  while( count-- > 0 )
    sum += *buf++;
#else
  UINT32 sum;

  sum = add_sum_neon(buffer, length);
#endif
  sum = sum - src_sum + STAMP_VALUE;

  return sum;
}
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
int SunxiSpriteVerifyChecksum(void *buffer, UINT32 length, UINT32 src_sum)
{
#ifndef CONFIG_USE_NEON_SIMD
  UINT32 *buf;
  UINT32 count;
  UINT32 sum;

  /* 生成校验和 */
  count = length >> 2;                       // 以 字（4bytes）为单位计数
  sum = 0;
  buf = (UINT32 *)buffer;
  do
  {
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
  }while( ( count -= 4 ) > (4-1) );

  while( count-- > 0 )
    sum += *buf++;
#else
  UINT32 sum;

  sum = add_sum_neon(buffer, length);
#endif
  sum = sum - src_sum + STAMP_VALUE;

  //debug("src sum=%x, check sum=%x\n", src_sum, sum);
  if( sum == src_sum )
    return 0;               // 校验成功
  else
    return -1;              // 校验失败
}


UINT32 SunxiAddSum(void *buffer, UINT32 length)
{
#ifndef CONFIG_USE_NEON_SIMD
  UINT32 *buf;
  UINT32 count;
  UINT32 sum;

  count = length >> 2;                         // 以 字（4bytes）为单位计数
  sum = 0;
  buf = (unsigned int *)buffer;
  while(count--)
  {
    sum += *buf++;                         // 依次累加，求得校验和
  };

  switch(length & 0x03)
  {
    case 0:
      return sum;
    case 1:
      sum += (*buf & 0x000000ff);
      break;
    case 2:
      sum += (*buf & 0x0000ffff);
      break;
    case 3:
      sum += (*buf & 0x00ffffff);
      break;
  }
#else
  UINT32 sum;

  sum = add_sum_neon(buffer, length);
#endif

  return sum;
}

