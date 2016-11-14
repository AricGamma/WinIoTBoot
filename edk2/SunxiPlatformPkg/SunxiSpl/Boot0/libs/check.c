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

#include <Interinc/private_uefi.h>
#include <Interinc/private_toc.h>
#include <boot0_include/boot0_helper.h>

//#include <Sunxi_type/PrivateDef.h>

//#pragma arm section  code="check_magic"
/********************************************************************************
*函数名称: check_magic
*函数原型: __s32 check_magic( __u32 *mem_base, const char *magic )
*函数功能: 使用“算术和”来校验内存中的一段数据
*入口参数: mem_base       Boot文件在内存中的起始地址
*          magic          Boot的magic
*返 回 值: CHECK_IS_CORRECT      校验正确
*          CHECK_IS_WRONG        校验错误
*备    注:
********************************************************************************/
INT32 check_magic( UINT32 *mem_base, const char *magic )
{
  struct spare_boot_head_t *bfh;

  bfh = (struct spare_boot_head_t *)mem_base;
  if(!(strncmp((const char *)bfh->boot_head.magic, magic, 8)))
  {
    return 0;
  }

  return -1;
}

//#pragma arm section




//#pragma arm section  code="check_sum"
/********************************************************************************
*函数名称: check_sum
*函数原型: __s32 check_sum( __u32 *mem_base, __u32 size, const char *magic )
*函数功能: 使用“算术和”来校验内存中的一段数据
*入口参数: mem_base           待校验的数据在内存中的起始地址（必须是4字节对齐的）
*          size               待校验的数据的个数（以字节为单位，必须是4字节对齐的）
*返 回 值: CHECK_IS_CORRECT   校验正确
*          CHECK_IS_WRONG     校验错误
*备    注:
********************************************************************************/
INT32 check_sum( UINT32 *mem_base, UINT32 size )
{
  UINT32 *buf;
  UINT32 count;
  UINT32 src_sum;
  UINT32 sum;
  struct spare_boot_head_t  *bfh;


  bfh = (struct spare_boot_head_t *)mem_base;

  /* 生成校验和 */
  src_sum = bfh->boot_head.check_sum;                  // 从Boot_file_head中的“check_sum”字段取出校验和
  bfh->boot_head.check_sum = STAMP_VALUE;              // 将STAMP_VALUE写入Boot_file_head中的“check_sum”字段

  count = size >> 2;                         // 以 字（4bytes）为单位计数
  sum = 0;
  buf = (UINT32 *)mem_base;
  do
  {
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
  }while( ( count -= 4 ) > (4-1) );

  while( count-- > 0 )
    sum += *buf++;

  bfh->boot_head.check_sum = src_sum;                  // 恢复Boot_file_head中的“check_sum”字段的值

  printf("sum=%x\n", sum);
  printf("src_sum=%x\n", src_sum);

  if( sum == src_sum )
    return 0;               // 校验成功
  else
    return -1;                 // 校验失败
}

//#pragma arm section



//#pragma arm section  code="check_file"
/********************************************************************************
*函数名称: check_file
*函数原型: __s32 check_file( __u32 *mem_base, __u32 size, const char *magic )
*函数功能: 使用“算术和”来校验内存中的一段数据
*入口参数: mem_base       待校验的数据在内存中的起始地址（必须是4字节对齐的）
*          size           待校验的数据的个数（以字节为单位，必须是4字节对齐的）
*          magic          magic number, 待校验文件的标识码
*返 回 值: CHECK_IS_CORRECT       校验正确
*          CHECK_IS_WRONG         校验错误
*备    注:
********************************************************************************/
INT32 check_file( UINT32 *mem_base, UINT32 size, const char *magic )
{
  if( check_magic( mem_base, magic ) == 0
        &&check_sum( mem_base, size  ) == 0 )
    return 0;
  else
    return -1;
}

int verify_addsum( void *mem_base, UINT32 size )
{
  UINT32 *buf;
  UINT32 count;
  UINT32 src_sum;
  UINT32 sum;
  sbrom_toc1_head_info_t *bfh;

  bfh = (sbrom_toc1_head_info_t *)mem_base;

  /* 生成校验和 */
  src_sum = bfh->add_sum;                  // 从Boot_file_head中的“check_sum”字段取出校验和
  bfh->add_sum = STAMP_VALUE;              // 将STAMP_VALUE写入Boot_file_head中的“check_sum”字段

  count = size >> 2;                         // 以 字（4bytes）为单位计数
  sum = 0;
  buf = (UINT32 *)mem_base;
  do
  {
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
    sum += *buf++;                         // 依次累加，求得校验和
  }while( ( count -= 4 ) > (4-1) );

  while( count-- > 0 )
    sum += *buf++;

  bfh->add_sum = src_sum;                  // 恢复Boot_file_head中的“check_sum”字段的值

  printf("sum=%x\n", sum);
  printf("src_sum=%x\n", src_sum);

  if( sum == src_sum )
    return 0;               // 校验成功
  else
    return -1;                 // 校验失败
}
