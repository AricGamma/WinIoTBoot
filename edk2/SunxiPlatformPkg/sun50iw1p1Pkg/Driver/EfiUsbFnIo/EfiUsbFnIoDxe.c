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
#include <Library/UncachedMemoryAllocationLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include <Protocol/EfiUsbFnIo.h>
#include <Sun50iw1p1UsbFnIo.h>


#define DRIVER_VENDOR_ID      0x1F3A
#define DRIVER_PRODUCT_ID     0xEfE8

extern int sunxi_usb_init(int delaytime);
extern int sunxi_usb_exit(void);


//UsbEventQueue to be used for recieving the bus and endpoint message.
STATIC QUEUE UsbEventQueue;

EFI_STATUS
UsbQueueEventMassage(
EFI_USBFN_MESSAGE             Message,
UINT8                         EndpointIndex,
EFI_USBFN_ENDPOINT_DIRECTION  Direction,
EFI_USBFN_TRANSFER_STATUS     TransferStatus,
VOID                          *Buffer,
UINTN                         BufferLengh
)
{
  EFI_STATUS Status = EFI_SUCCESS;
  USB_EVENT_MESSAGE EpXEventMessage;
  
  if(EndpointIndex>USB_MAX_LOGIC_EP_COUNT){
    USBFN_DRIVER_DEBUG_ERROR("%a:EndpointIndex=%d excess the max support number.\n", __FUNCTION__,EndpointIndex);
    Status = EFI_INVALID_PARAMETER; 
  }

  EpXEventMessage.Message = Message;
  EpXEventMessage.PayloadSize = 0;
  
  if(Message ==EfiUsbMsgBusEventSpeed){
    EpXEventMessage.Payload.ubs = UsbBusSpeedHigh;
    EpXEventMessage.PayloadSize = sizeof(EFI_USB_BUS_SPEED);
  }else if((!EndpointIndex)&&(Direction == EfiUsbEndpointDirectionDeviceRx)&&(TransferStatus == UsbTransferStatusComplete)){
    if(Buffer != NULL)
    {
      CopyMem(&EpXEventMessage.Payload.udr,Buffer,sizeof(EFI_USB_DEVICE_REQUEST));
      EpXEventMessage.PayloadSize = sizeof(EFI_USB_DEVICE_REQUEST);
    }    
  }else{
    if(Direction == EfiUsbEndpointDirectionDeviceRx){
      ASSERT(Buffer!=NULL);
    }
#if 0
  if(Buffer == NULL)
  {
  DEBUG (( EFI_D_INFO,"--------buffer is null--msg=%d---\n",Message));
  }
#endif
    EpXEventMessage.Payload.utr.BytesTransferred = BufferLengh;
    EpXEventMessage.Payload.utr.Buffer =Buffer;
    EpXEventMessage.Payload.utr.Direction = Direction;
    EpXEventMessage.Payload.utr.EndpointIndex = EndpointIndex;
    EpXEventMessage.Payload.utr.TransferStatus = TransferStatus;
    EpXEventMessage.PayloadSize = sizeof(EFI_USBFN_TRANSFER_RESULT);
  }

  if(Enqueue(&UsbEventQueue, &EpXEventMessage, sizeof(USB_EVENT_MESSAGE))){
    USBFN_DRIVER_DEBUG_ERROR("%a:Enqueue error!\n", __FUNCTION__);
    return EFI_DEVICE_ERROR;
  }
  return Status;
}

EFI_STATUS
EFIAPI
DetectPort (
  IN EFI_USBFN_IO_PROTOCOL   *This,
  OUT EFI_USBFN_PORT_TYPE    *PortType
  )
{
  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);

  *PortType = EfiUsbChargingDownstreamPort;

  return EFI_SUCCESS;
}
VOID QueueTest(VOID)
{
  QUEUE TestQueue;
  UINTN i,LoopCounter,Result,TestData[10];
  gBS->SetWatchdogTimer (0, 0x10000, 0, NULL);
  while(1){
    if(InitQueue(&TestQueue,10,sizeof(int))){
      USBFN_DRIVER_DEBUG_ERROR("%a:InitQueue error!\n", __FUNCTION__);
    }
  
    for(i=0;i<10;i++){
      TestData[i] = i;

      if(Enqueue(&TestQueue,&TestData[i],sizeof(UINTN))){
        USBFN_DRIVER_DEBUG_ERROR("%a:Enqueue error!\n", __FUNCTION__);
      }
    }

    for(i=0;i<10;i++){
      TestData[i] = i;
      if(Dequeue(&TestQueue,&Result,sizeof(UINTN))){
        USBFN_DRIVER_DEBUG_ERROR("%a:Dequeue error!\n", __FUNCTION__);
      }
    }
    
    DestroyQueue(&TestQueue);
    LoopCounter++;
    if(!(LoopCounter%100000)){
      USBFN_DRIVER_DEBUG_INFO("Loop=%d\n",LoopCounter);
    }
  }
}

EFI_STATUS
EFIAPI 
ConfigureEnableEndpoints (
  IN EFI_USBFN_IO_PROTOCOL         *This,
  IN EFI_USB_DEVICE_INFO           *DeviceInfo
  )
{

  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);
  if(InitQueue(&UsbEventQueue,USB_EVENT_QUEUE_MAX_EVENT_COUNT,sizeof(USB_EVENT_MESSAGE))){
    USBFN_DRIVER_DEBUG_ERROR("%a:Initialiaze UsbEventQueue error!\n", __FUNCTION__);
    return EFI_DEVICE_ERROR;
  }

  SunxiConfigureEnableEndpoints(This,DeviceInfo);

  return EFI_SUCCESS;   
}

EFI_STATUS
EFIAPI
GetDeviceInfo(
  IN EFI_USBFN_IO_PROTOCOL      *This,
  IN EFI_USBFN_DEVICE_INFO_ID   Id,
  IN OUT UINTN                  *BufferSize,
  OUT VOID                      *Buffer OPTIONAL
  )
{
  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);

  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI  
GetVendorIdProductId(
  IN EFI_USBFN_IO_PROTOCOL      *This,
  OUT UINT16                    *Vid,
  OUT UINT16                    *Pid
  )
{
  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);

  *Vid = DRIVER_VENDOR_ID;
  *Pid = DRIVER_PRODUCT_ID;

  return EFI_SUCCESS;  
}


EFI_STATUS
EFIAPI 
AbortTransfer(
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction
  )
{
  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);
  return SunxiUsbFnIoAbortTransfer(This,EndpointIndex,Direction);  
}


EFI_STATUS
EFIAPI 
GetEndpointStallState (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction,
  IN OUT BOOLEAN                  *State
  )
{
  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI 
SetEndpointStallState (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction,
  IN BOOLEAN                      State
  )
{

  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);
  return SunxiSetEndpointStallState(This,EndpointIndex,Direction,State);
}


EFI_STATUS
EFIAPI
EventHandler(
  IN EFI_USBFN_IO_PROTOCOL        *This,
  OUT EFI_USBFN_MESSAGE           *Message,
  IN OUT UINTN                    *PayloadSize,
  OUT EFI_USBFN_MESSAGE_PAYLOAD   *Payload
  )
{
  USB_EVENT_MESSAGE UsbEventMassage;

  if(IsQueueEmpty(&UsbEventQueue)){
    *Message = EfiUsbMsgNone;
    return EFI_SUCCESS;
  }
 
  if(Dequeue(&UsbEventQueue, &UsbEventMassage, sizeof(USB_EVENT_MESSAGE))){
    USBFN_DRIVER_DEBUG_ERROR("%a:Dequeue error!\n", __FUNCTION__);
    return EFI_DEVICE_ERROR;
  }
  
  *Message = UsbEventMassage.Message;
  *PayloadSize = UsbEventMassage.PayloadSize;
  CopyMem(Payload,&UsbEventMassage.Payload,*PayloadSize);
  
  return EFI_SUCCESS;
  
}

/*
data transter flow:
Transfer(EfiUsbEndpointDirectionDeviceTx/Rx)
        |
        |
         \|/
       EventHandler
                |
                |check the
                |
   EFI_USBFN_TRANSFER_RESULT  
               /\ 
              /  \
             /    \
         success  fail
*/
EFI_STATUS
EFIAPI 
Transfer (
  IN EFI_USBFN_IO_PROTOCOL         *This,
  IN UINT8                         EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION  Direction,
  IN OUT UINTN                     *BufferSize,
  IN OUT VOID                      *Buffer
  )
{
  EFI_STATUS      Status = EFI_UNSUPPORTED;

  USBFN_DRIVER_DEBUG_INFO("%a Transfer:Devices %a:%dBytes\n",EndpointIndex==0? "Control":"Bulk",Direction == EfiUsbEndpointDirectionDeviceTx ? "out":"in",*BufferSize);
  switch(EndpointIndex){
    case 0:
      if(Direction == EfiUsbEndpointDirectionDeviceTx){   
        Status = SunxiEp0SendData(*BufferSize,Buffer);
      }
      else
      {
        //for the EP0 ,aka the control ep, we will not invoke this function to
        //receive control transfer request,instead,we use the EventHandler to poll the data.
      }
      break;
    case 1:
      if(Direction == EfiUsbEndpointDirectionDeviceTx){
        Status = SunxiEpXSendData(This,EndpointIndex,BufferSize,Buffer);
      }
      else
      {
        Status = SunxiEpXReceiveData(This,EndpointIndex,BufferSize,Buffer);
      }
      break;
    default:
      USBFN_DRIVER_DEBUG_INFO("Unsupported EndpointIndex = %d\n",EndpointIndex); 
      break;

  }

  return Status;  
}


EFI_STATUS
EFIAPI  
GetMaxTransferSize (
  IN EFI_USBFN_IO_PROTOCOL     *This,
  OUT UINTN                    *MaxTransferSize
  )
{
  *MaxTransferSize = 64*1024;
  //USBFN_DRIVER_DEBUG_INFO("%a:MaxTransferSize = %d\n", __FUNCTION__,*MaxTransferSize);
  return EFI_SUCCESS;
}

typedef struct {
  VOID        *Allocation;
  UINTN       Pages;
  LIST_ENTRY  Link;
} FREE_PAGE_NODE;

static LIST_ENTRY  mPageList = INITIALIZE_LIST_HEAD_VARIABLE (mPageList);

VOID
AddUsbBufferPagesToList (
  IN VOID   *Allocation,
  UINTN     Pages
  )
{
  FREE_PAGE_NODE  *NewNode;
  
  NewNode = AllocatePool (sizeof (FREE_PAGE_NODE));
  if (NewNode == NULL) {
    ASSERT (FALSE);
    return;
  }
  
  NewNode->Allocation = Allocation;
  NewNode->Pages      = Pages;
  
  InsertTailList (&mPageList, &NewNode->Link);
}


VOID
RemoveUsbBufferPagesFromList (
  IN VOID  *Allocation,
  OUT UINTN *Pages
  )
{
  LIST_ENTRY      *Link;
  FREE_PAGE_NODE  *OldNode;

  *Pages = 0;
  
  for (Link = mPageList.ForwardLink; Link != &mPageList; Link = Link->ForwardLink) {
    OldNode = BASE_CR (Link, FREE_PAGE_NODE, Link);
    if (OldNode->Allocation == Allocation) {
      *Pages = OldNode->Pages;
      
      RemoveEntryList (&OldNode->Link);
      FreePool (OldNode);
      return;
    }
  }

  return;
}


EFI_STATUS
EFIAPI 
AllocateTransferBuffer (
  IN EFI_USBFN_IO_PROTOCOL    *This,
  IN UINTN                    Size,
  OUT VOID                    **Buffer
  )
{

  if(!Size)
    return EFI_INVALID_PARAMETER;

  *Buffer =UncachedAllocatePages(EFI_SIZE_TO_PAGES(Size));
  AddUsbBufferPagesToList ((VOID *)(*Buffer), EFI_SIZE_TO_PAGES (Size));
  USBFN_DRIVER_DEBUG_INFO("%a:Size=%d,Buffer=%p\n", __FUNCTION__,Size,*Buffer);
  return *Buffer==NULL ? EFI_OUT_OF_RESOURCES : EFI_SUCCESS; 
}


EFI_STATUS
EFIAPI 
FreeTransferBuffer (
  IN EFI_USBFN_IO_PROTOCOL    *This,
  IN VOID                     *Buffer
  )
{
  UINTN Pages;
  if(!Buffer)
    return EFI_INVALID_PARAMETER;
  
  RemoveUsbBufferPagesFromList (Buffer, &Pages);
  USBFN_DRIVER_DEBUG_INFO("%a:Size=%d,Buffer=%p\n", __FUNCTION__,Pages*(1<<12),Buffer);
  UncachedFreePages(Buffer,Pages);

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
GetEndpointMaxPacketSize (
  IN EFI_USBFN_IO_PROTOCOL      *This,
  IN EFI_USB_ENDPOINT_TYPE      EndpointType,
  IN EFI_USB_BUS_SPEED          BusSpeed,
  OUT UINT16                    *MaxPacketSize
  )
{
  switch(EndpointType){
    case UsbEndpointControl:
      *MaxPacketSize = 64;
      break;
     
    case UsbEndpointBulk:
      *MaxPacketSize = 512;
      break;
     
    case UsbEndpointIsochronous:
    case UsbEndpointInterrupt:  
      *MaxPacketSize = 512;
      break;  
  }
  //USBFN_DRIVER_DEBUG_INFO("%a:MaxPacketSize = %d\n", __FUNCTION__,*MaxPacketSize);
  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
StartController (
  IN EFI_USBFN_IO_PROTOCOL        *This
  )
{
  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);
  if(SunxiUsbInit(This,0))
  {
    USBFN_DRIVER_DEBUG_ERROR("usb init fail\n");
    SunxiUsbExit(This);

    return EFI_DEVICE_ERROR;
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI  
StopController(
  IN EFI_USBFN_IO_PROTOCOL        *This
  )
{
  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);
  SunxiUsbExit(This);
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI 
SetEndpointPolicy (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction,
  IN EFI_USBFN_POLICY_TYPE        PolicyType,
  IN UINTN                        BufferSize,
  IN VOID                         *Buffer
  )
{
  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);
  
  return EFI_UNSUPPORTED;
}
 

EFI_STATUS
EFIAPI  
GetEndpointPolicy (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction,
  IN EFI_USBFN_POLICY_TYPE        PolicyType,
  IN OUT UINTN                    BufferSize,
  IN OUT VOID                     *Buffer
  )
{
  USBFN_DRIVER_DEBUG_INFO("%a:\n", __FUNCTION__);
  
  return EFI_UNSUPPORTED;
}



STATIC EFI_USBFN_IO_PROTOCOL SunxiUsbFnIoProtocol = 
{
 .Revision                 = EFI_USBFN_IO_PROTOCOL_REVISION,                
 .DetectPort               = DetectPort,              
 .ConfigureEnableEndpoints = ConfigureEnableEndpoints,
 .GetEndpointMaxPacketSize = GetEndpointMaxPacketSize,
 .GetDeviceInfo            = GetDeviceInfo,           
 .GetVendorIdProductId     = GetVendorIdProductId,    
 .AbortTransfer            = AbortTransfer,           
 .GetEndpointStallState    = GetEndpointStallState,  
 .SetEndpointStallState    = SetEndpointStallState,  
 .EventHandler             = EventHandler,           
 .Transfer                 = Transfer,                
 .GetMaxTransferSize       = GetMaxTransferSize,      
 .AllocateTransferBuffer   = AllocateTransferBuffer,  
 .FreeTransferBuffer       = FreeTransferBuffer,      
 .StartController          = StartController,        
 .StopController           = StopController,          
 .SetEndpointPolicy        = SetEndpointPolicy,       
 .GetEndpointPolicy        = GetEndpointPolicy,      
};

EFI_STATUS
EfiUsbFnIoDxeInitialize (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
  EFI_HANDLE      Handle = NULL;
  EFI_STATUS      Status;
  
  // Install the EfiUsbFnIo interface
  Status = gBS->InstallMultipleProtocolInterfaces(&Handle, &gEfiUsbFnIoProtocolGuid, &SunxiUsbFnIoProtocol, NULL);
  ASSERT_EFI_ERROR(Status);
  
  return Status;
}

