/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Martin.Zheng <martinzheng@allwinnertech.com>
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

#ifndef __TYPES_H_
#define __TYPES_H_  1


#define NULL            0                   /* ָ���                                                  */

#define BOOT_STORAGE_CODE_BASE  (0x7FFFFF00)

#define BOOT_STORAGE_CODE1    (0x7FFFFF00)
#define BOOT_STORAGE_CODE2    (0x7FFFFF01)
#define BOOT_STORAGE_CODE3    (0x7FFFFF02)
#define BOOT_STORAGE_CODE4    (0x7FFFFF03)


/*
**********************************************************************************************************************
*                                              DATA TYPES
**********************************************************************************************************************
*/
//**************************************************
//normal typedef
typedef unsigned long long    u64;
typedef unsigned long long  __u64;
typedef long long     __s64;

typedef unsigned int        __u32;
typedef unsigned int          u32;

typedef   signed int        __s32;
typedef   signed int          s32;

typedef unsigned short      __u16;
typedef unsigned short        u16;

typedef   signed short      __s16;
typedef   signed short        s16;

typedef unsigned char       __u8;
typedef unsigned char         u8;

typedef   signed char       __s8;
typedef   signed char         s8;


typedef signed char         __bool;
typedef signed char         __Bool;

typedef unsigned int        __stk;                  /* Each stack entry is 32-bit wide                              */
typedef unsigned int        __cpu_sr;               /* Define size of CPU status register (PSR = 32 bits)           */


typedef float               __fp32;                 /* Single precision floating point                              */
typedef double              __fp64;                 /* Double precision floating point                              */

typedef unsigned int        __hdle;

typedef unsigned int        __size;
typedef unsigned int        __size_t;

typedef unsigned int        __sector_t;

typedef unsigned char   u_char;
typedef unsigned short    u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

/* sysv */
typedef unsigned char   unchar;
typedef unsigned short    ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;

typedef   __u8    uint8_t;
typedef   __u16   uint16_t;
typedef   __u32   uint32_t;

#define eGON2_OK            (0)
#define EGON2_OK            (0)

#define eGON2_FAIL          (-1)
#define EGON2_FAIL          (-1)

#undef  _set_bit
#define _set_bit( value, bit )          ( (x) |=  ( 1U << (y) ) )

#undef  set_bit
#define set_bit( value, bit )             ( (x) |=  ( 1U << (y) ) )

#undef  _clear_bit
#define _clear_bit( value, bit )        ( (x) &= ~( 1U << (y) ) )

#undef  clear_bit
#define clear_bit( value, bit )           ( (x) &= ~( 1U << (y) ) )

#undef  _set_bit
#define _set_bit( value, bit )          ( (value) |=  ( 1U << (bit) ) )

#undef  set_bit
#define set_bit( value, bit )             ( (value) |=  ( 1U << (bit) ) )

#undef  _clear_bit
#define _clear_bit( value, bit )        ( (value) &= ~( 1U << (bit) ) )

#undef  clear_bit
#define clear_bit( value, bit )           ( (value) &= ~( 1U << (bit) ) )

#undef  _reverse_bit
#define _reverse_bit( value, bit )      ( (value) ^=  ( 1U << (bit) ) )

#undef  reverse_bit
#define reverse_bit( value, bit )         ( (value) ^=  ( 1U << (bit) ) )

#undef  _test_bit
#define _test_bit( value, bit )         ( (value)  &  ( 1U << (bit) ) )

#undef  test_bit
#define test_bit( value, bit )            ( (value)  &  ( 1U << (bit) ) )


#undef  _min
#define _min( x, y )                ( (x) < (y) ? (x) : (y) )

#undef  min
#define min( x, y )               ( (x) < (y) ? (x) : (y) )

#undef  _max
#define _max( x, y )                ( (x) > (y) ? (x) : (y) )

#undef  max
#define max( x, y )               ( (x) > (y) ? (x) : (y) )

/* ȡ����ֵ */
#undef  _absolute
#define _absolute(p)              ((p) > 0 ? (p) : -(p))

#undef  absolute
#define absolute(p)               ((p) > 0 ? (p) : -(p))


#ifndef SZ_1K

#define SZ_512       0x00000200U
#define SZ_1K        0x00000400U
#define SZ_2K        0x00000800U
#define SZ_3K    0x00000C00U
#define SZ_4K        0x00001000U
#define SZ_6K    0x00001800U
#define SZ_8K        0x00002000U
#define SZ_10K     0x00002800U
#define SZ_12K     0x00003000U
#define SZ_16K     0x00004000U
#define SZ_20K     0x00005000U
#define SZ_24K     0x00006000U


#define SZ_32K       0x00008000U
#define SZ_64K       0x00010000U
#define SZ_128K      0x00020000U
#define SZ_256K      0x00040000U
#define SZ_512K      0x00080000U
#define SZ_1M        0x00100000U
#define SZ_2M        0x00200000U
#define SZ_4M        0x00400000U
#define SZ_8M        0x00800000U
#define SZ_16M       0x01000000U
#define SZ_32M       0x02000000U
#define SZ_64M       0x04000000U
#define SZ_128M      0x08000000U
#define SZ_256M      0x10000000U
#define SZ_512M      0x20000000U
#define SZ_1G        0x40000000U
#define SZ_2G        0x80000000U
#define SZ_4G        0x0100000000ULL
#define SZ_8G        0x0200000000ULL
#define SZ_16G       0x0400000000ULL
#define SZ_32G       0x0800000000ULL
#define SZ_64G       0x1000000000ULL
#endif


#endif  /*#ifndef __TYPES_H_*/


