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
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

// Cached copy of the Hardware Interrupt protocol instance
EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;

void OSAL_InterruptInit(void)
{
    EFI_STATUS Status;
    Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
    ASSERT_EFI_ERROR (Status);
    
}

/*
*******************************************************************************
*                     OSAL_RegISR
*
* Description:
*    注册中断服务程序
*
* Parameters:
*    irqno          ：input.  中断号
*    flags          ：input.  中断类型，默认值为0。
*    Handler        ：input.  中断处理程序入口，或者中断事件句柄
*    pArg           ：input.  参数
*    DataSize       ：input.  参数的长度
*    prio         ：input.  中断优先级

* 
* Return value:
*     返回成功或者失败。
*
* note:
*    中断处理函数原型，typedef __s32 (*ISRCallback)( void *pArg)。
*
*******************************************************************************
*/
//int OSAL_RegISR(__u32 IrqNo, __u32 Flags,ISRCallback Handler,void *pArg,__u32 DataSize,__u32 Prio)
int OSAL_RegISR(unsigned int IrqNo,HARDWARE_INTERRUPT_HANDLER Handler)
{
    EFI_STATUS  Status;
    Status = gInterrupt->RegisterInterruptSource (gInterrupt, IrqNo, Handler);
    ASSERT_EFI_ERROR (Status);

    return 0;
}   

/*
*******************************************************************************
*                     OSAL_UnRegISR
*
* Description:
*    注销中断服务程序
*
* Parameters:
*    irqno      ：input.  中断号
*    handler    ：input.  中断处理程序入口，或者中断事件句柄
*    Argment  ：input.  参数
* 
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void OSAL_UnRegISR(__u32 IrqNo, HARDWARE_INTERRUPT_HANDLER Handler, void *pArg)
{
    /* todo */    
    //irq_free_handler(IrqNo);
}


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
void OSAL_InterruptEnable(__u32 IrqNo)
{
  EFI_STATUS  Status;
  Status = gInterrupt->EnableInterruptSource(gInterrupt, IrqNo);
  ASSERT_EFI_ERROR (Status);
    /* todo */
}

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
void OSAL_InterruptDisable(__u32 IrqNo)
{
  EFI_STATUS  Status;
  Status = gInterrupt->DisableInterruptSource(gInterrupt, IrqNo);
  ASSERT_EFI_ERROR (Status);
}

