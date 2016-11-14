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

#ifndef  __QUEUE_H__
#define  __QUEUE_H__


typedef struct {
  VOID          **Buffer;
  UINTN         Head;
  UINTN         Tail;
  UINTN         MaxItermCount;
  UINTN         ItemSize;
} QUEUE;


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
  );
  
/**
  Destroy the queue

  @param Queue    Points to the queue.
**/
VOID
DestroyQueue (
  IN OUT QUEUE   *Queue
  );
  
/**
  Check whether the queue is empty.

  @param  Queue     Points to the queue.

  @retval TRUE      Queue is empty.
  @retval FALSE     Queue is not empty.

**/
BOOLEAN
IsQueueEmpty (
  IN  QUEUE   *Queue
  );
  
/**
  Check whether the queue is full.

  @param  Queue     Points to the queue.

  @retval TRUE      Queue is full.
  @retval FALSE     Queue is not full.

**/
BOOLEAN
IsQueueFull (
  IN  QUEUE   *Queue
  );
  
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
  );
  
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
  );
  
#endif
