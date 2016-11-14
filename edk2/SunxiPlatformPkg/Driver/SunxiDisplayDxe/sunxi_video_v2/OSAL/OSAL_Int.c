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
*    ע���жϷ������
*
* Parameters:
*    irqno          ��input.  �жϺ�
*    flags          ��input.  �ж����ͣ�Ĭ��ֵΪ0��
*    Handler        ��input.  �жϴ��������ڣ������ж��¼����
*    pArg           ��input.  ����
*    DataSize       ��input.  �����ĳ���
*    prio         ��input.  �ж����ȼ�

* 
* Return value:
*     ���سɹ�����ʧ�ܡ�
*
* note:
*    �жϴ�����ԭ�ͣ�typedef __s32 (*ISRCallback)( void *pArg)��
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
*    ע���жϷ������
*
* Parameters:
*    irqno      ��input.  �жϺ�
*    handler    ��input.  �жϴ��������ڣ������ж��¼����
*    Argment  ��input.  ����
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
*    �ж�ʹ��
*
* Parameters:
*    irqno ��input.  �жϺ�
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
*    �жϽ�ֹ
*
* Parameters:
*     irqno ��input.  �жϺ�
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

