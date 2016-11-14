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
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

#include <Library/SunxiQueueLib.h>


/**
  Create the queue.

  @param  Queue     Points to the queue.
  @param  MaxItermCount   Max iterm count can be place in this queue
  @param  ItemSize  Size of the single item.

**/
EFI_STATUS
InitQueue (
  IN OUT  QUEUE   *Queue,
  IN      UINTN       MaxItermCount,
  IN      UINTN       ItemSize
  )
{
  UINTN                      Index;
  
  Queue->MaxItermCount = MaxItermCount;
  Queue->ItemSize  = ItemSize;
  Queue->Head      = 0;
  Queue->Tail      = 0;

  Queue->Buffer = AllocatePool ((MaxItermCount + 1)*sizeof(VOID*));
  ASSERT (Queue->Buffer != NULL);
  
  Queue->Buffer[0] = AllocatePool ((MaxItermCount + 1) * ItemSize);
  ASSERT (Queue->Buffer[0] != NULL);

  for (Index = 1; Index < MaxItermCount + 1; Index++) {
    Queue->Buffer[Index] = ((UINT8 *) Queue->Buffer[Index - 1]) + ItemSize;
  }
  return EFI_SUCCESS;
}

/**
  Destroy the queue

  @param Queue    Points to the queue.
**/
VOID
DestroyQueue (
  IN OUT QUEUE   *Queue
  )
{
  FreePool (Queue->Buffer[0]);
  FreePool (Queue->Buffer);
}


/**
  Check whether the queue is empty.

  @param  Queue     Points to the queue.

  @retval TRUE      Queue is empty.
  @retval FALSE     Queue is not empty.

**/
BOOLEAN
IsQueueEmpty (
  IN  QUEUE   *Queue
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
BOOLEAN
IsQueueFull (
  IN  QUEUE   *Queue
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
EFI_STATUS
Enqueue (
  IN OUT  QUEUE *Queue,
  IN      VOID             *Item,
  IN      UINTN            ItemSize
  )
{
  ASSERT (ItemSize == Queue->ItemSize);

  if (IsQueueFull (Queue)) {
    return EFI_DEVICE_ERROR;
  }

  CopyMem (Queue->Buffer[Queue->Tail], Item, ItemSize);

  //
  // Adjust the tail pointer of the FIFO keyboard buffer.
  //
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
EFI_STATUS
Dequeue (
  IN OUT  QUEUE *Queue,
     OUT  VOID             *Item,
  IN      UINTN            ItemSize
  )
{
  ASSERT (Queue->ItemSize == ItemSize);

  if (IsQueueEmpty (Queue)) {
    return EFI_DEVICE_ERROR;
  }

  CopyMem (Item, Queue->Buffer[Queue->Head], ItemSize);

  //
  // Adjust the head pointer of the FIFO keyboard buffer.
  //
  Queue->Head = (Queue->Head + 1) % (Queue->MaxItermCount + 1);

  return EFI_SUCCESS;
}

