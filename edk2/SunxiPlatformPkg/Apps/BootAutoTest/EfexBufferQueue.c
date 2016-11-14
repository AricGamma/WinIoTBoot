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

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>

#include "BootAutoTest.h"
#include "EfexBufferQueue.h"


/**
  Create the queue.

  @param  Queue     Points to the queue.
  @param  MaxItermCount   Max iterm count can be place in this queue
  @param  ItemSize  Size of the single item.

**/
STATIC 
EFI_STATUS
InitBufferQueue (
  IN OUT  BUFFER_QUEUE   *Queue,
  IN      UINTN           MaxItermCount,
  IN      UINTN           ItemSize,
  IN      UINTN           TagSize
  )
{
  UINTN                      Index;
  
  Queue->MaxItermCount = MaxItermCount;
  Queue->ItemSize  = ItemSize;
  Queue->TagSize  = TagSize;
  Queue->Head      = 0;
  Queue->Tail      = 0;

  Queue->Buffer = AllocatePool ((MaxItermCount + 1)*sizeof(VOID*));
  ASSERT (Queue->Buffer != NULL);
  
  Queue->Buffer[0] = AllocatePool ((MaxItermCount + 1) * ItemSize);
  ASSERT (Queue->Buffer[0] != NULL);

  for (Index = 1; Index < MaxItermCount + 1; Index++) {
    Queue->Buffer[Index] = ((UINT8 *) Queue->Buffer[Index - 1]) + ItemSize;
  }
  
  Queue->TagBuffer = AllocatePool ((MaxItermCount + 1)*sizeof(VOID*));
  ASSERT (Queue->TagBuffer != NULL);
  
  Queue->TagBuffer[0] = AllocatePool ((MaxItermCount + 1) * TagSize);
  ASSERT (Queue->TagBuffer[0] != NULL);

  for (Index = 1; Index < MaxItermCount + 1; Index++) {
    Queue->TagBuffer[Index] = ((UINT8 *) Queue->TagBuffer[Index - 1]) + ItemSize;
  }
  return EFI_SUCCESS;
}

/**
  Destroy the queue

  @param Queue    Points to the queue.
**/
STATIC 
VOID
DestroyBufferQueue (
  IN OUT BUFFER_QUEUE   *Queue
  )
{
  FreePool (Queue->Buffer[0]);
  FreePool (Queue->Buffer);

  FreePool (Queue->TagBuffer[0]);
  FreePool (Queue->TagBuffer);

}


/**
  Check whether the queue is empty.

  @param  Queue     Points to the queue.

  @retval TRUE      Queue is empty.
  @retval FALSE     Queue is not empty.

**/
STATIC
BOOLEAN
IsBufferQueueEmpty (
  IN  BUFFER_QUEUE   *Queue
  )
{
  //
  // Meet FIFO empty condition
  //
  return (BOOLEAN) (Queue->Head == Queue->Tail);
}


/**
  Check whether the queue is full.

  @param  Queue     Points to the queue.

  @retval TRUE      Queue is full.
  @retval FALSE     Queue is not full.

**/
STATIC
BOOLEAN
IsBufferQueueFull (
  IN  BUFFER_QUEUE   *Queue
  )
{
  return (BOOLEAN) (((Queue->Tail + 1) % (Queue->MaxItermCount + 1)) == Queue->Head);
}


/**
  Enqueue the item to the queue.

  @param  Queue     Points to the queue.
  @param  Item      Points to the item to be enqueued.
  @param  ItemSize  Size of the item.
**/
STATIC
EFI_STATUS
BufferEnqueue (
  IN OUT  BUFFER_QUEUE *Queue,
  IN      VOID             *Item,
  IN      UINTN            ItemSize,
  IN      VOID             *Tag,
  IN      UINTN            TagSize
  )
{
  ASSERT (ItemSize == Queue->ItemSize);
  ASSERT (TagSize == Queue->TagSize);
  
  if (IsBufferQueueFull (Queue)) {
     return EFI_DEVICE_ERROR;
  }

  CopyMem (Queue->Buffer[Queue->Tail], Item, ItemSize);

  CopyMem (Queue->TagBuffer[Queue->Tail], Tag, TagSize);

  Queue->Tail = (Queue->Tail + 1) % (Queue->MaxItermCount + 1);
  return EFI_SUCCESS;
}


/**
  Dequeue a item from the queue.

  @param  Queue     Points to the queue.
  @param  Item      Receives the item.
  @param  ItemSize  Size of the item.

  @retval EFI_SUCCESS        Item was successfully dequeued.
  @retval EFI_DEVICE_ERROR   The queue is empty.

**/
STATIC
EFI_STATUS
BufferDequeue (
  IN OUT  BUFFER_QUEUE *Queue,
     OUT  VOID             **Item,
  IN      UINTN            ItemSize,
     OUT  VOID             **Tag,
  IN      UINTN            TagSize
  )
{
  ASSERT (Queue->ItemSize == ItemSize);
  ASSERT (TagSize == Queue->TagSize);
  
  if (IsBufferQueueEmpty (Queue)) {
    return EFI_DEVICE_ERROR;
  }

  *Item = Queue->Buffer[Queue->Head];
  *Tag = Queue->TagBuffer[Queue->Head];

  Queue->Head = (Queue->Head + 1) % (Queue->MaxItermCount + 1);

  return EFI_SUCCESS;
}



STATIC BUFFER_QUEUE EfexBufferQueue;

STATIC UINT32 gQueueFreePages;

EFI_STATUS 
EfexQueueInitialize(
  SUNXI_EFEX *This
)
{

  EFI_STATUS Status=EFI_SUCCESS;
  
  Status =InitBufferQueue(&EfexBufferQueue,EFEX_BUFFER_QUEUE_MAX_LENGHT,EFEX_BUFFER_QUEUE_PAGE_SIZE,sizeof(EFEX_BUFFER_QUEUE_ELEMENT));
  if(Status)
  {
    EFEX_ERROR("EfexQueueInitialize failed\n");
  }

  gQueueFreePages = EFEX_BUFFER_QUEUE_MAX_LENGHT;
  return Status;
}

EFI_STATUS
EfexQueueExit(
  SUNXI_EFEX *This
)
{
  DestroyBufferQueue(&EfexBufferQueue);
  return EFI_SUCCESS;
}

EFI_STATUS EfexQueueWriteOnePage( 
  SUNXI_EFEX *This
)
{
  EFI_STATUS Status=EFI_SUCCESS;
  EFEX_BUFFER_QUEUE_ELEMENT* BufferQueueElement = NULL;
  VOID* DataBuffer=NULL;
  
  if(IsBufferQueueEmpty(&EfexBufferQueue))
  {
    return Status;
  }
  Status = BufferDequeue(&EfexBufferQueue,&DataBuffer,EFEX_BUFFER_QUEUE_PAGE_SIZE,(VOID**)&BufferQueueElement,sizeof(EFEX_BUFFER_QUEUE_ELEMENT));
  if(Status){
    EFEX_ERROR("Efex Dequeue failed\n");
    return Status;
  }
  
  gQueueFreePages++;
  if(This->FlashIo->SunxiFlashIoWrite(This->FlashIo,BufferQueueElement->FlashStart,\
    BufferQueueElement->FlashSectors,DataBuffer))
  {
    EFEX_ERROR("EfexQueueWriteOnePage error: write flash from 0x%x, sectors 0x%x failed\n", 
      BufferQueueElement->FlashStart,BufferQueueElement->FlashSectors);
    return EFI_DEVICE_ERROR;
  }

  return Status;
}

EFI_STATUS 
EfexQueueWriteAllPage( 
  SUNXI_EFEX *This
)
{
  EFEX_BUFFER_QUEUE_ELEMENT* BufferQueueElement = NULL;
  VOID* DataBuffer =NULL;
  if(IsBufferQueueEmpty(&EfexBufferQueue))
  {
    return EFI_SUCCESS;
  }
  EFEX_DEBUG_INFO(L"%a:%d:FreePages=%d\n",__FUNCTION__,__LINE__,gQueueFreePages);
  while(!BufferDequeue(&EfexBufferQueue,&DataBuffer,EFEX_BUFFER_QUEUE_PAGE_SIZE,(VOID**)&BufferQueueElement,sizeof(EFEX_BUFFER_QUEUE_ELEMENT)))
  {
    gQueueFreePages++;
    if(This->FlashIo->SunxiFlashIoWrite(This->FlashIo,BufferQueueElement->FlashStart,\
    BufferQueueElement->FlashSectors,DataBuffer))
    {
      EFEX_ERROR("EfexQueueWriteOnePage error: write flash from 0x%x, sectors 0x%x failed\n", 
        BufferQueueElement->FlashStart,BufferQueueElement->FlashSectors);
      return EFI_DEVICE_ERROR;
    }
  }
  return EFI_SUCCESS;
}


EFI_STATUS 
EfexBufferEnqueue(
IN SUNXI_EFEX *This,
IN UINT32 FlashStart,
IN UINT32 FlashSectors,
IN VOID* Buffer
)
{
  UINT32 SectorsPerPage;     
  UINT32 Offset;
  UINT32 RequirePages ;
  UINT32 QueueFreePages = gQueueFreePages;
  VOID* DataBuffer;
  EFEX_BUFFER_QUEUE_ELEMENT BufferQueueElement;
  
  EFI_STATUS Status=EFI_SUCCESS;
  EFEX_DEBUG_INFO(L"%a:%d:FreePages=%d\n",__FUNCTION__,__LINE__,gQueueFreePages);
  //make sure queue has enough space to save buffer
  SectorsPerPage     = EFEX_BUFFER_QUEUE_PAGE_SIZE>>9;
  RequirePages = (FlashSectors+SectorsPerPage-1)/SectorsPerPage;
    
  if(RequirePages > gQueueFreePages) 
  {
    UINT32 i = 0;
    for(i = 0; i < RequirePages - QueueFreePages; i++)
    {
      if(EfexQueueWriteOnePage(This))
      {
        return EFI_DEVICE_ERROR;
      }
    }
  }

  if(RequirePages > gQueueFreePages)
  {
    EFEX_ERROR("efex queue error: free space is not enough\n");
    return EFI_DEVICE_ERROR;
  }

  //save buff to queue
  Offset = 0;
  while(FlashSectors > SectorsPerPage)
  {
    BufferQueueElement.FlashStart= FlashStart;
    BufferQueueElement.FlashSectors= SectorsPerPage;
    DataBuffer  = (VOID*)(((UINTN)Buffer)+(Offset<<9));
    Status = BufferEnqueue(&EfexBufferQueue,DataBuffer,EFEX_BUFFER_QUEUE_PAGE_SIZE,(VOID*)&BufferQueueElement,sizeof(EFEX_BUFFER_QUEUE_ELEMENT));
    if(Status){
      EFEX_ERROR("Efex Enqueue failed\n");
      return Status;
    }
    gQueueFreePages--;
    FlashSectors -= SectorsPerPage;
    Offset += SectorsPerPage;
    FlashStart += SectorsPerPage;
  }
  if(FlashSectors)
  {
    BufferQueueElement.FlashStart= FlashStart;
    BufferQueueElement.FlashSectors= FlashSectors;
    DataBuffer  = (VOID*)(((UINTN)Buffer)+(Offset<<9));
    Status = BufferEnqueue(&EfexBufferQueue,DataBuffer,EFEX_BUFFER_QUEUE_PAGE_SIZE,(VOID*)&BufferQueueElement,sizeof(EFEX_BUFFER_QUEUE_ELEMENT));
    if(Status){
      EFEX_ERROR("Efex Enqueue failed\n");
      return Status;
    }
    gQueueFreePages--;
  }

  return Status;
}

