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

#ifndef  __OSAL_INT_H__
#define  __OSAL_INT_H__

#include "OSAL.h"
#include <Protocol/HardwareInterrupt.h>


#ifndef __LINUX_OSAL__
typedef __s32 (*ISRCallback)( void *);
#else
typedef int (*ISRCallback)( int, void* );
#endif


void OSAL_InterruptInit(void);

/*
*******************************************************************************
*                     OSAL_RegISR
*
* Description:
*    注册中断服务程序
*
* Parameters:
*    irqno    	    ：input.  中断号
*    flags    	    ：input.  中断类型，默认值为0。
*    Handler  	    ：input.  中断处理程序入口，或者中断事件句柄
*    pArg 	        ：input.  参数
*    DataSize 	    ：input.  参数的长度
*    prio	        ：input.  中断优先级

* 
* Return value:
*     返回成功或者失败。
*
* note:
*    中断处理函数原型，typedef __s32 (*ISRCallback)( void *pArg)。
*
*******************************************************************************
*/

int OSAL_RegISR(unsigned int IrqNo,HARDWARE_INTERRUPT_HANDLER Handler);


/*
*******************************************************************************
*                     OSAL_UnRegISR
*
* Description:
*    注销中断服务程序
*
* Parameters:
*    irqno    	：input.  中断号
*    handler  	：input.  中断处理程序入口，或者中断事件句柄
*    Argment 	：input.  参数
* 
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void OSAL_UnRegISR(__u32 IrqNo, HARDWARE_INTERRUPT_HANDLER Handler, void *pArg);

//void OSAL_UnRegISR(__u32 IrqNo, ISRCallback Handler, void *pArg);

/*
*******************************************************************************
*                     OSAL_InterruptEnable
*
* Description:
*    中断使能
*
* Parameters:
*    irqno ：input.  中断号
* 
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void OSAL_InterruptEnable(__u32 IrqNo);

/*
*******************************************************************************
*                     OSAL_InterruptDisable
*
* Description:
*    中断禁止
*
* Parameters:
*     irqno ：input.  中断号
* 
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void OSAL_InterruptDisable(__u32 IrqNo);

#endif   //__OSAL_INT_H__


