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

#ifndef  __OSAL_CACHE_H__
#define  __OSAL_CACHE_H__

/* ˢ�±��λ */
#define  CACHE_FLUSH_I_CACHE_REGION       0  /* ���I-cache�д���������һ�������cache��      */
#define  CACHE_FLUSH_D_CACHE_REGION       1  /* ���D-cache�д���������һ�������cache��      */
#define  CACHE_FLUSH_CACHE_REGION       2  /* ���D-cache��I-cache�д���������һ�������cache�� */
#define  CACHE_CLEAN_D_CACHE_REGION       3  /* ����D-cache�д���������һ�������cache��      */
#define  CACHE_CLEAN_FLUSH_D_CACHE_REGION   4  /* �������D-cache�д���������һ�������cache��  */
#define  CACHE_CLEAN_FLUSH_CACHE_REGION     5  /* �������D-cache�����������I-cache        */

/*
*******************************************************************************
*                     OSAL_CacheRangeFlush
*
* Description:
*    Cache����
*
* Parameters:
*    Address    :  Ҫ��ˢ�µ�������ʼ��ַ
*    Length     :  ��ˢ�µĴ�С
*    Flags      :  ˢ�±��λ
*    
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void OSAL_CacheRangeFlush(void*Address, __u32 Length, __u32 Flags);


#endif   //__OSAL_CACHE_H__


