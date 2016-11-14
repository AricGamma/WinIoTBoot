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

#ifndef  __BUFFER_QUEUE_H__
#define  __BUFFER_QUEUE_H__

typedef struct {
  VOID          **Buffer;
  VOID          **TagBuffer;
  UINTN         Head;
  UINTN         Tail;
  UINTN         MaxItermCount;
  UINTN         ItemSize;
  UINTN         TagSize;
} BUFFER_QUEUE;

typedef struct _EFEX_BUFFER_QUEUE_ELEMENT
{
  UINT32 FlashStart;         //nand or emmc logic address to be write
  UINT32 FlashSectors;   //buff size :  1 sector  = 512 bytes
 //   UINT8*  Buffer;         //buff address
 //   UINT32 Reserved;
}EFEX_BUFFER_QUEUE_ELEMENT;

#define EFEX_BUFFER_QUEUE_MAX_LENGHT 30
#define EFEX_BUFFER_QUEUE_PAGE_SIZE  (64*1024)


EFI_STATUS 
EfexQueueInitialize(
  SUNXI_EFEX *This
);

EFI_STATUS
EfexQueueExit(
  SUNXI_EFEX *This
);

EFI_STATUS EfexQueueWriteOnePage( 
  SUNXI_EFEX *This
);

EFI_STATUS 
EfexQueueWriteAllPage( 
  SUNXI_EFEX *This
);

EFI_STATUS 
EfexBufferEnqueue(
IN SUNXI_EFEX *This,
IN UINT32 FlashStart,
IN UINT32 FlashSectors,
IN VOID* Buffer
);


#endif
