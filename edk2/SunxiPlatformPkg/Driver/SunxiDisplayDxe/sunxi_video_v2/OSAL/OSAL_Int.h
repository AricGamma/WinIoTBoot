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
*    ע���жϷ������
*
* Parameters:
*    irqno    	    ��input.  �жϺ�
*    flags    	    ��input.  �ж����ͣ�Ĭ��ֵΪ0��
*    Handler  	    ��input.  �жϴ��������ڣ������ж��¼����
*    pArg 	        ��input.  ����
*    DataSize 	    ��input.  �����ĳ���
*    prio	        ��input.  �ж����ȼ�

* 
* Return value:
*     ���سɹ�����ʧ�ܡ�
*
* note:
*    �жϴ�����ԭ�ͣ�typedef __s32 (*ISRCallback)( void *pArg)��
*
*******************************************************************************
*/

int OSAL_RegISR(unsigned int IrqNo,HARDWARE_INTERRUPT_HANDLER Handler);


/*
*******************************************************************************
*                     OSAL_UnRegISR
*
* Description:
*    ע���жϷ������
*
* Parameters:
*    irqno    	��input.  �жϺ�
*    handler  	��input.  �жϴ��������ڣ������ж��¼����
*    Argment 	��input.  ����
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
void OSAL_InterruptEnable(__u32 IrqNo);

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
void OSAL_InterruptDisable(__u32 IrqNo);

#endif   //__OSAL_INT_H__


