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

#ifndef __CAL_CRC_H__
#define __CAL_CRC_H__


typedef struct tag_CRC32_DATA
{
  unsigned int CRC;       //在Windows下编程，int的大小是32位
  unsigned int CRC_32_Tbl[256]; //用来保存码表
}CRC32_DATA_t;



extern  unsigned int calc_crc32(void * buffer, unsigned length);


#endif  /* __CAL_CRC_H__ */
