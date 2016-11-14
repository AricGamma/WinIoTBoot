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

#ifndef  __OSAL_H__
#define  __OSAL_H__


typedef struct
{
    char  gpio_name[32];
    int port;
    int port_num;
    int mul_sel;
    int pull;
    int drv_level;
    int data;
    int gpio;
} disp_gpio_set_t;


#include "../de_bsp/de/bsp_display.h"
#include <Library/DebugLib.h>
#include  "OSAL_Clock.h"
#include  "OSAL_Pin.h"
#include  "OSAL_Lib_C.h"
#include  "OSAL_Int.h"
#include  "OSAL_Pin.h"
#include  "OSAL_Parser.h"
#include  "OSAL_IrqLock.h"
#include  "OSAL_Cache.h"
#include  "OSAL_Power.h"

#define sys_get_value(n)    (*((volatile u8 *)(n)))          /* byte input */
#define sys_put_value(n,c)  (*((volatile u8 *)(n))  = (c))   /* byte output */
//#define sys_get_hvalue(n)   (*((volatile u16 *)(n)))         /* half word input */
//#define sys_put_hvalue(n,c) (*((volatile u16 *)(n)) = (c))   /* half word output */
#define sys_get_wvalue(n)   (*((volatile u32 *)(n)))          /* word input */
#define sys_put_wvalue(n,c) (*((volatile u32 *)(n))  = (c))   /* word output */
#define sys_set_bit(n,c)    (*((volatile u8 *)(n)) |= (c))   /* byte bit set */
#define sys_clr_bit(n,c)    (*((volatile u8 *)(n)) &=~(c))   /* byte bit clear */
#define sys_set_hbit(n,c)   (*((volatile u16 *)(n))|= (c))   /* half word bit set */
#define sys_clr_hbit(n,c)   (*((volatile u16 *)(n))&=~(c))   /* half word bit clear */
#define sys_set_wbit(n,c)   (*((volatile u32 *)(n))|= (c))    /* word bit set */
#define sys_cmp_wvalue(n,c) (c == (*((volatile __u32 *) (n))))
#define sys_clr_wbit(n,c)   (*((volatile u32 *)(n))&=~(c))  


#endif   //__OSAL_H__


