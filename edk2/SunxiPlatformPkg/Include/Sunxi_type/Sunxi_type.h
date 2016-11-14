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

#ifndef  __SUNXI_TYPE_H__
#define  __SUNXI_TYPE_H__


#if !defined(bool)
#define bool unsigned char
#endif


#if !defined(__bool)
#define __bool unsigned char
#endif


typedef UINT8  u8;
typedef UINT8  __u8;
typedef UINT8 uchar;
typedef INT8  s8;

typedef UINT32 u32;
typedef UINT32 __u32;
typedef UINT32 uint;

typedef INT32  s32;
typedef INT32  __s32;

typedef INT16  s16;
typedef INT16  __s16;
typedef INT16  ushort;


typedef UINT16 u16;
typedef UINT16 __u16;


typedef INT64  s64;
typedef INT64  __s64;

typedef UINT64  u64;
typedef UINT64  __u64;
/* Type for `void *' pointers. */
typedef unsigned long int uintptr_t;
#define __iomem
#define __user

#ifdef CONFIG_SYS_64BIT_LBA
typedef uint64_t lbaint_t;
#define LBAF "%llx"
#define LBAFU "%llu"
#else
typedef UINT32 lbaint_t;
#define LBAF "%ld"
#define LBAFU "%ld"
#endif

typedef unsigned long  ulong;


#endif

