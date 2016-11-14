/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  wangwei <wangwei@allwinnertech.com>
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
#include <Library/SunxiPartitionLib.h>
#include <Library/SunxiMbr.h>
#include <Library/TimerLib.h>
#include <Library/UsbBaseHead.h>

#include <IndustryStandard/Usb.h>
#include <IndustryStandard/Scsi.h>
#include <Protocol/EfiUsbFnIo.h>
#include <Protocol/SunxiFlashIo.h>
#include <Sunxi_type/Sunxi_type.h>
#include <PrivateBurn.h>

#define SCSI_READ_FORMAT_CAPACITY (0x23)


STATIC UINTN PrivatePartitionStart  = 0;
STATIC UINTN PrivatePartitionLen    = 0;
STATIC UINTN FlashStart        = 0;
STATIC UINTN FlashSectors      = 0;
STATIC BOOLEAN USBExitFlag     = FALSE;
STATIC UMASS_CSW_T   gCSW      = {0};

STATIC SUNXI_PRIVATEBURN* SunxiPrivateBurn;

char SunxiPrivateBurnNormalLangID[8]     = {0x04, 0x03, 0x09, 0x04, '\0'};
char  *SunxiUsbPrivateBurnDev[SUNXI_USB_PRIVATEBURN_DEV_MAX] = {SunxiPrivateBurnNormalLangID,           \
                                  SUNXI_PRIVATEBURN_DEVICE_SERIAL_NUMBER, \
                                SUNXI_PRIVATEBURN_DEVICE_MANUFACTURER,  \
                                SUNXI_PRIVATEBURN_DEVICE_PRODUCT
                                };
CONST UINT8 PrivateBurn_RequestSense[20] = {0x07,0x00,0x02,0x00,0x00,0x00,0x00,0x0a,0x00,0x00,0x00,0x00,0x3a,0x00,0x00,0x00,0x00,0x00};

CONST UINT8 PrivateBurn_InquiryData[40]  = {0x00, 0x80, 0x02, 0x02, 0x1f,                                  \
                                            0x00, 0x00, 0x00,                                              \
                                            'U',  'S',  'B',  '2',  '.',  '0',  0x00, 0x00,                \
                                            'U' , 'S',  'B',  ' ', 'S',  't',  'o' , 'r' , 'a' , 'g' , 'e',\
                                            0x00, 0x00, 0x00, 0x00, 0x00,
                                            '0',  '1',  '0',  '0',  '\0' };

STATIC EFI_USB_DEVICE_DESCRIPTOR mDeviceDescriptor = {
  sizeof (USB_DEVICE_DESCRIPTOR),                  //Length
  USB_DESC_TYPE_DEVICE,                            //DescriptorType
  0x0200,                                          //BcdUSB
  0x0,                                             //DeviceClass
  0x0,                                             //DeviceSubClass
  0x0,                                             //DeviceProtocol
  0x40,                                            //MaxPacketSize0
  DEVICE_VENDOR_ID,                  //IdVendor
  DEVICE_PRODUCT_ID,                   //IdProduct
  DEVICE_BCD,                                      //BcdDevice
  SUNXI_PRIVATEBURN_DEVICE_STRING_MANUFACTURER_INDEX, //StrManufacturer
  SUNXI_PRIVATEBURN_DEVICE_STRING_PRODUCT_INDEX,      //StrProduct
  SUNXI_PRIVATEBURN_DEVICE_STRING_SERIAL_NUMBER_INDEX,//StrSerialNumber
  1                                                //NumConfigurations
};

/*
  We have one configuration, one interface, and two endpoints (one IN, one OUT)
*/

// Lazy (compile-time) way to concatenate descriptors to pass to the USB device
// protocol

EFI_USB_CONFIG_DESCRIPTOR     mConfigDescriptor  = {
    sizeof (USB_CONFIG_DESCRIPTOR),                   //Length;
    USB_DESC_TYPE_CONFIG,                             //DescriptorType;
    0,                              //TotalLength;
    1,                                                //NumInterfaces;
    1,                                                //ConfigurationValue;
    0,                                                //Configuration;
    0xc0,                                             //Attributes;
    0xFA                                              //MaxPower;
};

EFI_USB_INTERFACE_DESCRIPTOR  mInterfaceDescriptor = {
    sizeof (USB_INTERFACE_DESCRIPTOR), //Length;
    USB_DESC_TYPE_INTERFACE, //DescriptorType;
    0,                                                //InterfaceNumber;
    0,                                                //AlternateSetting;
    2,                                                //NumEndpoints;
    USB_CLASS_MASS_STORAGE,                           //InterfaceClass;
    // Vendor specific interface subclass and protocol codes.
    // I found these values in the PrivateBurn code
    // (in match_fastboot_with_serial in fastboot.c).
    US_SC_SCSI,                                             //InterfaceSubClass;
    US_PR_BULK,                                             //InterfaceProtocol;
    1                                                       //Interface;
};

STATIC EFI_USB_ENDPOINT_DESCRIPTOR   mEndpointDescriptor[2] = {
  //EP IN
  {
      sizeof (USB_ENDPOINT_DESCRIPTOR),                 //Length;
      USB_DESC_TYPE_ENDPOINT,                           //DescriptorType;
      1 | BIT7,                                         //EndpointAddress;
      0x2,                                              //Attributes;
      512,                                      //MaxPacketSize;
      0                                                //Interval;
    },
    //EP OUT
  {
      sizeof (USB_ENDPOINT_DESCRIPTOR),                 //Length;
      USB_DESC_TYPE_ENDPOINT,                           //DescriptorType;
      1,                                                //EndpointAddress;
      0x2,                                              //Attributes;
      512,                                      //MaxPacketSize;
      0                                                //Interval;
  }
};




STATIC EFI_STATUS 
PrivateBurnGetDescriptor(
  EFI_USBFN_IO_PROTOCOL *This,
  EFI_USB_DEVICE_REQUEST *Req
)
{
   
  EFI_STATUS Status = EFI_SUCCESS;  
  
  switch(Req->Value >> 8)
  {
    case USB_DESC_TYPE_STRING:
    {
      UINTN bLength = 0;
      UINTN StringIndex = Req->Value & 0xff;
      
      /* Language ID */
      if(StringIndex == 0)
      {
        bLength = MIN(4, Req->Length);
        This->Transfer(This,0,EfiUsbEndpointDirectionDeviceTx,&bLength,(VOID*)SunxiUsbPrivateBurnDev[0]);
      }
      else if(StringIndex < SUNXI_USB_PRIVATEBURN_DEV_MAX)
      {
        /* Size of string in chars */
        UINTN i = 0;
        UINTN StringLengh = AsciiStrLen ((CONST CHAR8*)SunxiUsbPrivateBurnDev[StringIndex]);
        bLength  = 2 + (2 * StringLengh);
        SunxiPrivateBurn->ControlSetUpBuffer[0] = bLength;        /* length */
        SunxiPrivateBurn->ControlSetUpBuffer[1] = USB_DESC_TYPE_STRING;  /* descriptor = string */

        /* Copy device string to fifo, expand to simple unicode */
        for(i = 0; i < StringLengh; i++)
        {
          SunxiPrivateBurn->ControlSetUpBuffer[2+ 2*i + 0] = SunxiUsbPrivateBurnDev[StringIndex][i];
          SunxiPrivateBurn->ControlSetUpBuffer[2+ 2*i + 1] = 0;
        }
        bLength = MIN(bLength, Req->Length);
        This->Transfer(This,PRIVATEBURN_CONTROL_ENDPOINT_INDEX,EfiUsbEndpointDirectionDeviceTx,&bLength,(VOID*)SunxiPrivateBurn->ControlSetUpBuffer);
      }
      else
      {
        PRIVATEBURN_ERROR("sunxi usb err: string line %d is not supported\n", StringIndex);
      }    
    }
    break;

    case 0X06:
    {
      PRIVATEBURN_DEBUG_INFO("get qualifier descriptor\n");
    }
    break;

    default:
      PRIVATEBURN_ERROR("err: unkown wValue(%d)\n", Req->Value);
      Status = EFI_DEVICE_ERROR;
  }

  return Status;
}


STATIC EFI_STATUS PrivateBurnUsbIoInitialize(
  IN OUT SUNXI_PRIVATEBURN*           This,
  IN UINTN                              ConnectionTimeout,
  IN UINTN                              ReadWriteTimeout
 )
{

  EFI_STATUS      Status = EFI_SUCCESS;
  CHAR8 *MbrDataBuffer =NULL;
  EFI_USB_DEVICE_INFO    *DeviceInfo = NULL;
  EFI_USB_CONFIG_INFO    *ConfigInfoTable =NULL;
  EFI_USB_INTERFACE_INFO *InterfaceInfo = NULL;
  EFI_USB_ENDPOINT_DESCRIPTOR *EndpointDescriptor =NULL;
  EFI_USBFN_IO_PROTOCOL *UsbIo;
  
  UINT16 Pid,Vid;
  UINT16 MaxPacketSize;
  
  Status = gBS->LocateProtocol (&gSunxiFlashIoProtocolGuid, NULL, (VOID **) &This->FlashIo);
  if (EFI_ERROR (Status)) {
    PRIVATEBURN_ERROR("Couldn't open Sunxi Flash Io Protocol: %r\n", Status);
    return Status;
  }
    
  Status = gBS->LocateProtocol(&gEfiUsbFnIoProtocolGuid, NULL, (VOID **)&This->UsbIo);
  ASSERT_EFI_ERROR(Status);

  UsbIo  = This->UsbIo;

  MbrDataBuffer = AllocatePool(SUNXI_MBR_SIZE);    
  if(MbrDataBuffer == NULL){   
    Status = EFI_OUT_OF_RESOURCES;  
    return Status;
  }

  //
  Status  = SunxiPrivateBurn->FlashIo->SunxiFlashIoInit(SunxiPrivateBurn->FlashIo,1);
  if(EFI_ERROR(Status))
  {
    PRIVATEBURN_ERROR("FlashIO init Fail\n");
  }
  //read mbr   
  Status = SunxiPrivateBurn->FlashIo->SunxiFlashIoRead(SunxiPrivateBurn->FlashIo,0,\
  SUNXI_MBR_SIZE/SunxiPrivateBurn->FlashIo->BlockIo->Media->BlockSize,MbrDataBuffer);
  if(Status){
    PRIVATEBURN_ERROR("Read MBR error\n");
    FreePool(MbrDataBuffer);
    return EFI_DEVICE_ERROR;
  }else
  {
    //suxi mbr init    
    if(SunxiPartitionInitialize(MbrDataBuffer, SUNXI_MBR_SIZE)){
       PRIVATEBURN_ERROR("Partition Initiliaze error\n");
       FreePool(MbrDataBuffer);
       return EFI_DEVICE_ERROR;
    }
    Status = SunxiPartitionGetOffsetByName(SUNXI_PRIVATE_PART_NAME,&PrivatePartitionStart);
    if(EFI_ERROR(Status)){
      PRIVATEBURN_ERROR("no private partition find\n");
      FreePool(MbrDataBuffer);
      return EFI_DEVICE_ERROR;
    }
    Status = SunxiPartitionGetSizeByName(SUNXI_PRIVATE_PART_NAME,&PrivatePartitionLen);
    if(EFI_ERROR(Status)){
      PRIVATEBURN_ERROR("get private partition size fail\n");
      FreePool(MbrDataBuffer);
      return EFI_DEVICE_ERROR;
    }
  }
  PRIVATEBURN_DEBUG_INFO("PrivatePart Start Addr = 0x%x\n", PrivatePartitionStart);
  PRIVATEBURN_DEBUG_INFO("PrivatePart Size = 0x%x\n", PrivatePartitionLen);
  FreePool(MbrDataBuffer);

  Status = UsbIo->AllocateTransferBuffer(UsbIo,PRIVATEBURN_CONTROL_SET_UP_BUFFER_SIZE,(VOID**)&This->ControlSetUpBuffer);
  if(Status){
    PRIVATEBURN_ERROR("Allocate ControlSetupBuffer failed\n");  
    return Status;
  }

  Status = UsbIo->AllocateTransferBuffer(UsbIo,PRIVATEBURN_DATA_SET_UP_BUFFER_SIZE,(VOID**)&This->DataSetUpBuffer);
  if(Status){
    PRIVATEBURN_ERROR("Allocate DataSetupBuffer failed\n"); 
    goto ErrorOut1;
  }
  
  This->BaseReceiveBuffer = (UINT8*)AllocatePool(PRIVATEBURN_DATA_RECIVE_BUFFER_SIZE);
    if(!This->BaseReceiveBuffer){
    PRIVATEBURN_ERROR("Allocate DataRecvieBuffer failed\n");
    Status = EFI_DEVICE_ERROR;
    goto ErrorOut2;
  }
  PRIVATEBURN_DEBUG_INFO("DataRecvieBuffer = 0x%x\n",This->BaseReceiveBuffer);
  
  This->BaseSendBuffer = (UINT8 *)AllocatePool(PRIVATEBURN_DATA_SEND_BUFFER_SIZE);
  if(!This->BaseSendBuffer){
    PRIVATEBURN_ERROR("Allocate DataSendBuffer failed\n");  
    Status = EFI_DEVICE_ERROR;
    goto ErrorOut3;
  }
  PRIVATEBURN_DEBUG_INFO("BaseSendBuffer = 0x%x\n",This->BaseSendBuffer);

  PRIVATEBURN_DEBUG_INFO("Start Controller\n");
  Status = UsbIo->StartController(UsbIo);
  if(Status){
    PRIVATEBURN_DEBUG_INFO("StartController failed\n"); 
    return Status;
  }
  DeviceInfo = AllocateZeroPool(sizeof(EFI_USB_DEVICE_INFO));
  if(!DeviceInfo){
    PRIVATEBURN_ERROR("AllocateZeroPool DeviceInfo failed\n");  
    return EFI_DEVICE_ERROR;
  }

  ConfigInfoTable = AllocateZeroPool(sizeof(EFI_USB_CONFIG_INFO));
  if(!ConfigInfoTable){
    PRIVATEBURN_ERROR("AllocateZeroPool ConfigInfoTable failed\n"); 
    return EFI_DEVICE_ERROR;
  }
  
  InterfaceInfo = AllocateZeroPool(sizeof(EFI_USB_INTERFACE_INFO));
  if(!InterfaceInfo){
    PRIVATEBURN_ERROR("AllocateZeroPool InterfaceInfo failed\n"); 
    return EFI_DEVICE_ERROR;
  }
  //get interface info:InterfaceDescriptor and EndPointDescripter.
  InterfaceInfo->InterfaceDescriptor = &mInterfaceDescriptor;
  EndpointDescriptor = &mEndpointDescriptor[0];
  InterfaceInfo->EndpointDescriptorTable = &EndpointDescriptor;

    //get config info: InterfaceInfo and ConfigDescriptor
  ConfigInfoTable->InterfaceInfoTable = &InterfaceInfo;
  ConfigInfoTable->ConfigDescriptor = &mConfigDescriptor;

  //get device info: config info and DeviceDescriptor
  DeviceInfo->ConfigInfoTable = &ConfigInfoTable;
  DeviceInfo->DeviceDescriptor = &mDeviceDescriptor;

  //fix the pid and vid in the DeviceDescriptor
  Status = UsbIo->GetVendorIdProductId(UsbIo,&Pid,&Vid);
  if(Status){
    PRIVATEBURN_ERROR("Get pid/vid failed!\n");
    goto Out;
  }
  //DeviceInfo->DeviceDescriptor->IdVendor = Vid;
  //DeviceInfo->DeviceDescriptor->IdProduct = Pid;

  //fix the MaxPacketSize in EndPointDescripter
  Status =UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,UsbBusSpeedHigh,&MaxPacketSize);
  if(Status){
    PRIVATEBURN_ERROR("Get MaxPacketSize failed!\n");
    goto Out;
  }
  mEndpointDescriptor[0].MaxPacketSize = MaxPacketSize;
  mEndpointDescriptor[1].MaxPacketSize = MaxPacketSize;

  //to do:fix  device-specific information 

  //fix the config descriptor size.
  mConfigDescriptor.TotalLength =sizeof(EFI_USB_CONFIG_DESCRIPTOR)+\
  sizeof(EFI_USB_INTERFACE_DESCRIPTOR)*mConfigDescriptor.NumInterfaces+\
  sizeof(EFI_USB_ENDPOINT_DESCRIPTOR)*mInterfaceDescriptor.NumEndpoints;
  
  //Config and enable the endpoint.
  PRIVATEBURN_DEBUG_INFO("Configure and enable endpoints\n"); 
    Status = UsbIo->ConfigureEnableEndpoints(UsbIo,DeviceInfo);
  if(Status){
    PRIVATEBURN_ERROR("enable endpoint failed!\n");
    goto Out;
  }
Out:
  if(DeviceInfo){
    FreePool(DeviceInfo);
  } 

  if(ConfigInfoTable){
    FreePool(ConfigInfoTable);
  }
  if(InterfaceInfo){
    FreePool(InterfaceInfo);
  }
  return Status;

ErrorOut1:
  UsbIo->FreeTransferBuffer(UsbIo,(VOID*)This->ControlSetUpBuffer);
ErrorOut2:
  UsbIo->FreeTransferBuffer(UsbIo,(VOID*)This->DataSetUpBuffer);
ErrorOut3:
  FreePool(This->BaseReceiveBuffer);
  return Status;
  
}

STATIC EFI_STATUS PrivateBurnUsbIoExit(
  SUNXI_PRIVATEBURN *This
 )
{

  EFI_STATUS      Status = EFI_SUCCESS;
  Status = This->UsbIo->StopController(This->UsbIo);
  if(Status){
    PRIVATEBURN_ERROR("Stop controller failed!\n");
  }

  This->UsbIo->FreeTransferBuffer(This->UsbIo,(VOID*)This->ControlSetUpBuffer);
  This->UsbIo->FreeTransferBuffer(This->UsbIo,(VOID*)This->DataSetUpBuffer);
  FreePool(This->BaseReceiveBuffer);
  FreePool(This->BaseSendBuffer);
  
  return Status;
}

/**
  The deficiency of this Write/Read interface is that asynchronous mode can't be supported.
but in case of windows mobile need it, leave it out there..
**/

EFI_STATUS
PrivateBurnUsbIoWrite (
  IN UINTN                              NumberOfBytesToWrite,
  IN OUT UINTN                          *NumberOfBytesWrite,
  OUT VOID                              *Buffer
  )
{
  PRIVATEBURN_DEBUG_INFO("%a called!\n",__FUNCTION__);
  return EFI_UNSUPPORTED;
}


EFI_STATUS
PrivateBurnUsbIoRead (
  IN UINTN                              NumberOfBytesToRead,
  IN OUT UINTN                          *NumberOfBytesRead,
  OUT VOID                              *Buffer
  )
{
  PRIVATEBURN_DEBUG_INFO("%a called!\n",__FUNCTION__);
  return EFI_UNSUPPORTED;
}



EFI_STATUS
PrivateBurnUsbIoSetUpHandle(
  SUNXI_PRIVATEBURN  *This,
  EFI_USB_DEVICE_REQUEST *Req
 )
{
  UINT16 MaxPacketSize=0;
  UINTN TxSize =0;
  EFI_USBFN_IO_PROTOCOL *UsbIo=This->UsbIo;
  
  EFI_STATUS Status = EFI_SUCCESS;
  
  if(USB_REQ_TYPE_STANDARD == (Req->RequestType & USB_REQ_TYPE_MASK))//Standard Req
  {
    switch(Req->Request)
    {
      case USB_REQ_GET_STATUS:    //   0x00
      {
        /* device-to-host */
        if(USB_DIR_IN == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_GET_STATUS\n");
        }
        break;
      }
      case USB_REQ_CLEAR_FEATURE:   //   0x01
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
            PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_CLEAR_FEATURE\n");
        }
        break;
      }
      case USB_REQ_SET_FEATURE:   //   0x03
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
           PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_SET_FEATURE\n");
        }
        break;
      }
      case USB_REQ_SET_ADDRESS:   //   0x05
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          if(USB_REQ_RECIPIENT_DEVICE == (Req->RequestType & USB_REQ_RECIPIENT_MASK))
          {
            /* receiver is device */
            PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_SET_ADDRESS\n");
          }
        }
        break;
      }
      case USB_REQ_GET_DESCRIPTOR:    //   0x06
      {
        /* device-to-host */
        if(USB_DIR_IN == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          if(USB_REQ_RECIPIENT_DEVICE == (Req->RequestType & USB_REQ_RECIPIENT_MASK))
          {
            PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_GET_DESCRIPTOR\n");
            Status  = PrivateBurnGetDescriptor(UsbIo,Req);
          }
        }
        break;
      }
      case USB_REQ_SET_DESCRIPTOR:    //   0x07
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          if(USB_REQ_RECIPIENT_DEVICE == (Req->RequestType & USB_REQ_RECIPIENT_MASK))
          {
            //there is some problem
            PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_SET_DESCRIPTOR\n");           
          }
        }
        break;
      }
      case USB_REQ_GET_CONFIG:    //   0x08
      {
        /* device-to-host */
        if(USB_DIR_IN == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          if(USB_REQ_RECIPIENT_DEVICE == (Req->RequestType & USB_REQ_RECIPIENT_MASK))
          {
            PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_GET_CONFIG\n");
          }
        }

        break;
      }
      case USB_REQ_SET_CONFIG:    //   0x09
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          if(USB_REQ_RECIPIENT_DEVICE == (Req->RequestType & USB_REQ_RECIPIENT_MASK))
          {
             PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_SET_CONFIG\n");

             //device configured!, prepare for bulk transfer.
             UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
                        UsbBusSpeedHigh,&MaxPacketSize);
             TxSize = MaxPacketSize;
             //set up Bulk receive buffer for the controller.
             UsbIo->Transfer(UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
             EfiUsbEndpointDirectionDeviceRx,&TxSize,(VOID*)This->DataSetUpBuffer);
          }
        }
        break;
      }
      case USB_REQ_GET_INTERFACE:   //   0x0a
      {
        /* device-to-host */
        if(USB_DIR_IN == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          if(USB_REQ_RECIPIENT_DEVICE == (Req->RequestType & USB_REQ_RECIPIENT_MASK))
          {
            PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_GET_INTERFACE\n");
          }
        }
        break;
      }
      case USB_REQ_SET_INTERFACE:   //   0x0b
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          if(USB_REQ_RECIPIENT_INTERFACE == (Req->RequestType & USB_REQ_RECIPIENT_MASK))
          {
            PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_SET_INTERFACE\n");
          }
        }
        break;
      }
      case USB_REQ_SYNCH_FRAME:   //   0x0b
      {
        /* device-to-host */
        if(USB_DIR_IN == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          if(USB_REQ_RECIPIENT_INTERFACE == (Req->RequestType & USB_REQ_RECIPIENT_MASK))
          {
            PRIVATEBURN_DEBUG_INFO("standard_req:USB_REQ_SYNCH_FRAME\n");
          }
        }
        break;
      }

      default:
      {
        PRIVATEBURN_DEBUG_INFO("ep0 control unsupported request:Request=0x%x\n",Req->Request);
        break;
      }
    }
  }
  else
  {
    switch(Req->RequestType)    //PBURN 特有请求
    {
      case 161:
        if(Req->Request == 0xFE)
        { 
          PRIVATEBURN_DEBUG_INFO("ask for max lun\n");
          TxSize = 1;
          SunxiPrivateBurn->ControlSetUpBuffer[0] = 0;
          This->UsbIo->Transfer(This->UsbIo,PRIVATEBURN_CONTROL_ENDPOINT_INDEX, \
                    EfiUsbEndpointDirectionDeviceTx,&TxSize,(VOID*)SunxiPrivateBurn->ControlSetUpBuffer);

          //buffer[0] = 0;
          //sunxi_udc_send_setup(1, buffer);
        }
        else
        {
          PRIVATEBURN_DEBUG_INFO("unknown ep0 req\n");
        }
        break;

      default:
        PRIVATEBURN_DEBUG_ERROR("unknown non standard ep0 req\n");
        break;
    }
    PRIVATEBURN_DEBUG_INFO("ep0 control non-standard request:Request=0x%x\n",Req->Request);
  }
  return Status;
}

STATIC EFI_STATUS 
PrivateBurnSendResponse(VOID)
{
  UINTN Length;
  Length = sizeof(UMASS_CSW_T);
  //gCSW.bCSWStatus    = Status;
  CopyMem(SunxiPrivateBurn->DataSetUpBuffer,&gCSW,Length);
  SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
  SunxiPrivateBurn->State = SunxiUsbPrivateBurnResponse;
  SunxiPrivateBurn->NextState = SunxiUsbPrivateBurnSetup;
  return EFI_SUCCESS;
}

VOID UserCmdHandler(CONST CHAR8 *CmdData)
{
  UINTN Length;
  UMASS_CBW_T *Cbw = (UMASS_CBW_T *)CmdData;

  PRIVATEBURN_INFO("usb burn private\n");

  switch(Cbw->CBWCDB[1])
  {
    case 0:       //握手
      {
        PRIVATEBURN_INFO("usb command handshake\n");
        USB_HANDSHAKE  *handshake = (USB_HANDSHAKE *)SunxiPrivateBurn->DataSetUpBuffer;
        ZeroMem(handshake, sizeof(USB_HANDSHAKE));
        AsciiStrCpy(handshake->magic, "usbhandshake");
        handshake->sizelo = PrivatePartitionLen;
        handshake->sizehi = 0;
        Length = MIN(Cbw->dCBWDataTransferLength, sizeof(USB_HANDSHAKE));

        SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
            EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
        SunxiPrivateBurn->State = SunxiUsbPrivateBurnSendData;

      }
      break;

      case 1:       //小机端接收数据
        {        
          FlashSectors  = *(int *)(Cbw->CBWCDB + 8);
          FlashStart    = *(int *)(Cbw->CBWCDB + 4);
          FlashStart += PrivatePartitionStart;

          PRIVATEBURN_INFO("usb command device receiver data,addr=0x%x size = %d\n",FlashStart,Cbw->dCBWDataTransferLength);
          Length = Cbw->dCBWDataTransferLength;
          SunxiPrivateBurn->TryToReceiveBytes = Length;
          if (SunxiPrivateBurn->TryToReceiveBytes == 0 ||
              SunxiPrivateBurn->TryToReceiveBytes > SUNXI_USB_PRIVATEBURN_BUFFER_MAX) {
              PRIVATEBURN_ERROR("ERROR: bytes to write:0x%x ,max 0x%x.\n",SunxiPrivateBurn->TryToReceiveBytes, SUNXI_USB_PRIVATEBURN_BUFFER_MAX);
              gCSW.bCSWStatus = 1;
              PrivateBurnSendResponse();
              break;
          }
          SunxiPrivateBurn->ActuallyReceivedBytes = 0;
          SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
              EfiUsbEndpointDirectionDeviceRx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
          SunxiPrivateBurn->State = SunxiUsbPrivateBurnReceiveData;
        }
        break;

      case 2:       //关闭usb
        {          
          USB_HANDSHAKE  *handshake = (USB_HANDSHAKE *)SunxiPrivateBurn->DataSetUpBuffer;
         
          PRIVATEBURN_INFO("usb command close usb\n");
          ZeroMem(handshake, sizeof(USB_HANDSHAKE));
          AsciiStrCpy(handshake->magic, "usb_dataok");

          Length = MIN(Cbw->dCBWDataTransferLength, sizeof(USB_HANDSHAKE));

          SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
              EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
          SunxiPrivateBurn->State = SunxiUsbPrivateBurnSendData;
          SunxiPrivateBurn->FlashIo->SunxiFlashIoFlush(SunxiPrivateBurn->FlashIo);
          USBExitFlag = TRUE;

        }
        break;

      case 3:             //小机端发送数据
        {
          UINT32 start, sectors;
          UINT32 offset;
          EFI_STATUS Status;
          
          start   = *(int *)(Cbw->CBWCDB + 4);    //读数据的偏移量
          sectors = *(int *)(Cbw->CBWCDB + 8);    //扇区数;

          Length     = MIN(Cbw->dCBWDataTransferLength, sectors * 512);
          PRIVATEBURN_INFO("usb command device send data,size = %d\n",Length);
         
          offset = PrivatePartitionStart;
          Status = SunxiPrivateBurn->FlashIo->SunxiFlashIoRead(SunxiPrivateBurn->FlashIo,start + offset, sectors, SunxiPrivateBurn->DataSetUpBuffer);
          if(EFI_ERROR(Status))
          {
            PRIVATEBURN_ERROR("sunxi flash read err: start,0x%x sectors 0x%x\n", start, sectors);
            gCSW.bCSWStatus = 1;
          }
          SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
              EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
          SunxiPrivateBurn->State = SunxiUsbPrivateBurnSendData;
        }
        break;

    default:
      break;
  }
}
VOID ScsICmdHandler(IN UINTN Size,IN CONST CHAR8 * Data)
{
  UMASS_CBW_T* Cbw;
  UINT8 StdCommand;
  UINTN Length;
  UINT32 sectors;
  UINT32 PartOffset;

  Cbw = (UMASS_CBW_T*)Data;
  StdCommand = Cbw->CBWCDB[0];

  if(CBWSIGNATURE != Cbw->dCBWSignature)
  {
    PRIVATEBURN_ERROR("the cbw signature 0x%x is bad, need 0x%x\n", Cbw->dCBWSignature, CBWSIGNATURE);
    return ;
  }

  gCSW.dCSWSignature = CSWSIGNATURE;
  gCSW.dCSWTag     = Cbw->dCBWTag;
  gCSW.bCSWStatus    = 0;
    
  switch(StdCommand)
  {
    case EFI_SCSI_OP_TEST_UNIT_READY:
      PRIVATEBURN_DEBUG_INFO("SCSI_TST_U_RDY\n");
      gCSW.bCSWStatus = 1;
      PrivateBurnSendResponse();
      break;

    case EFI_SCSI_OP_REQUEST_SENSE:
      PRIVATEBURN_DEBUG_INFO("SCSI_REQ_SENSE\n");
      PRIVATEBURN_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);

      Length = MIN(Cbw->dCBWDataTransferLength, 18);
      CopyMem(SunxiPrivateBurn->DataSetUpBuffer, PrivateBurn_RequestSense,Length);
      SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
          EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
      SunxiPrivateBurn->State = SunxiUsbPrivateBurnSendData;
      break;

    case EFI_SCSI_OP_VERIFY:
      PRIVATEBURN_DEBUG_INFO("SCSI_VERIFY\n");
      PRIVATEBURN_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
      PrivateBurnSendResponse();
      break;

    case EFI_SCSI_OP_INQUIRY:
      PRIVATEBURN_DEBUG_INFO("SCSI_INQUIRY\n");
      PRIVATEBURN_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);

      Length= MIN(Cbw->dCBWDataTransferLength, 36);
      CopyMem(SunxiPrivateBurn->DataSetUpBuffer, PrivateBurn_InquiryData,Length);
      SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
          EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
      SunxiPrivateBurn->State = SunxiUsbPrivateBurnSendData;

      break;

    case EFI_SCSI_OP_MODE_SEN6:
      PRIVATEBURN_DEBUG_INFO("SCSI_MODE_SEN6\n");
      PRIVATEBURN_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
      
      SunxiPrivateBurn->DataSetUpBuffer[0] = 3;
      SunxiPrivateBurn->DataSetUpBuffer[1] = 0;   //介质类型，为0
      SunxiPrivateBurn->DataSetUpBuffer[2] = 0;   //设备标识参数, 为0
      SunxiPrivateBurn->DataSetUpBuffer[3] = 0;   //块描述符长度，可以为0

      Length= MIN(Cbw->dCBWDataTransferLength, 4);
      SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
          EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
      SunxiPrivateBurn->State = SunxiUsbPrivateBurnSendData;
      break;

    case EFI_SCSI_OP_READ_CAPACITY:
      PRIVATEBURN_DEBUG_INFO("SCSI_RD_CAPAC\n");
      PRIVATEBURN_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
        
      ZeroMem(SunxiPrivateBurn->DataSetUpBuffer, 8);
      SunxiPrivateBurn->DataSetUpBuffer[2] = 0x80;
      SunxiPrivateBurn->DataSetUpBuffer[6] = 2;

      gCSW.bCSWStatus = 1;
      Length= MIN(Cbw->dCBWDataTransferLength, 8);
      SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
              EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
      SunxiPrivateBurn->State = SunxiUsbPrivateBurnSendData;

      break;

    case SCSI_READ_FORMAT_CAPACITY:
      PRIVATEBURN_DEBUG_INFO("SCSI_RD_FMT_CAPAC\n");
      PRIVATEBURN_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
    
      ZeroMem(SunxiPrivateBurn->DataSetUpBuffer, 12);

      SunxiPrivateBurn->DataSetUpBuffer[2] = 0x80;
      SunxiPrivateBurn->DataSetUpBuffer[6] = 2;
      SunxiPrivateBurn->DataSetUpBuffer[8] = 2;
      SunxiPrivateBurn->DataSetUpBuffer[10] = 2;
            
      gCSW.bCSWStatus = 1;
      Length= MIN(Cbw->dCBWDataTransferLength, 12);
            
      SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
              EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
      SunxiPrivateBurn->State = SunxiUsbPrivateBurnSendData;
      break;
            
    case EFI_SCSI_OP_MEDIA_REMOVAL:
      PRIVATEBURN_DEBUG_INFO("SCSI_MED_REMOVL\n");
      PrivateBurnSendResponse();
      break;
        
    case EFI_SCSI_OP_READ6:
    case EFI_SCSI_OP_READ10:    //HOST READ, not this device read
      PRIVATEBURN_DEBUG_INFO("SCSI_READ\n");
      PRIVATEBURN_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
          
      sectors = (Cbw->CBWCDB[7]<<8) | Cbw->CBWCDB[8];
      PRIVATEBURN_DEBUG_INFO("read sectors 0x%x\n", sectors);
      Length = MIN(Cbw->dCBWDataTransferLength, sectors * 512);

      gCSW.bCSWStatus = 1;
      SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
          EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
      SunxiPrivateBurn->State = SunxiUsbPrivateBurnSendData;
      break;

    case EFI_SCSI_OP_WRITE6:
    case EFI_SCSI_OP_WRITE10:   //HOST WRITE, not this device write
      PRIVATEBURN_DEBUG_INFO("SCSI_WRITE\n");
      PRIVATEBURN_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);

      FlashStart   = (Cbw->CBWCDB[2]<<24) | Cbw->CBWCDB[3]<<16 | Cbw->CBWCDB[4]<<8 | Cbw->CBWCDB[5]<<0;
      FlashSectors = (Cbw->CBWCDB[7]<<8) | Cbw->CBWCDB[8];

      SunxipartitionGetOffset(0,&PartOffset);
      FlashStart += PartOffset;
      
      Length = MIN(Cbw->dCBWDataTransferLength, FlashSectors * 512);
      
      PRIVATEBURN_DEBUG_INFO("try to receive data 0x%x\n", Length);
      PRIVATEBURN_DEBUG_INFO("command write start: 0x%x, sectors 0x%x\n", FlashStart, FlashSectors);

      SunxiPrivateBurn->TryToReceiveBytes = Length;
      if (SunxiPrivateBurn->TryToReceiveBytes == 0 ||
        SunxiPrivateBurn->TryToReceiveBytes > SUNXI_USB_PRIVATEBURN_BUFFER_MAX) {
        PRIVATEBURN_ERROR("ERROR: bytes to write:0x%x ,max 0x%x.\n",SunxiPrivateBurn->TryToReceiveBytes, SUNXI_USB_PRIVATEBURN_BUFFER_MAX);
        gCSW.bCSWStatus = 1;
        PrivateBurnSendResponse();
        break;
      }
          
      SunxiPrivateBurn->ActuallyReceivedBytes = 0;
      SunxiPrivateBurn->UsbIo->Transfer(SunxiPrivateBurn->UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
          EfiUsbEndpointDirectionDeviceRx,&Length,(VOID*)SunxiPrivateBurn->DataSetUpBuffer);
      SunxiPrivateBurn->State = SunxiUsbPrivateBurnReceiveData;
      break;

    case 0xf3:      //自定义命令，用于烧录用户数据      
      UserCmdHandler(Data);
      break;

    default:
      PRIVATEBURN_DEBUG_INFO("not supported command 0x%x now\n", Cbw->CBWCDB[0]);
      PRIVATEBURN_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
      gCSW.bCSWStatus = 1;
      PrivateBurnSendResponse();
      break;
  }
}

EFI_STATUS
PrivateBurnRxHandle(
  SUNXI_PRIVATEBURN  *This,
  EFI_USBFN_TRANSFER_RESULT* TransferResult
)
{
  EFI_STATUS Status;
  UINT16 MaxPacketSize=0;
  UINTN TxSize =0;
  EFI_USBFN_IO_PROTOCOL *UsbIo=This->UsbIo;
  UINTN  BytesTransferred  =TransferResult->BytesTransferred;
  EFI_USBFN_TRANSFER_STATUS TransferStatus =TransferResult->TransferStatus;
  VOID *Buffer =TransferResult->Buffer;
  
  if(TransferResult->EndpointIndex!=PRIVATEBURN_BULK_ENDPOINT_INDEX){
    return EFI_SUCCESS;
  }
  if(TransferStatus!=UsbTransferStatusComplete){
    PRIVATEBURN_DEBUG_INFO("%a:Transfer has not completed yet\n");
    return EFI_SUCCESS;
  }
  //PRIVATEBURN_DEBUG_INFO("********%a:TransferStatus: %d********\n",__FUNCTION__,TransferResult->TransferStatus);
  //PRIVATEBURN_DEBUG_INFO("********%a:EndpointIndex: %d********\n",__FUNCTION__,TransferResult->EndpointIndex);
  //PRIVATEBURN_DEBUG_INFO("********%a:Direction: %a********\n",__FUNCTION__,TransferResult->Direction==EfiUsbEndpointDirectionDeviceTx?"DeviceTx":"DeviceRx");
  //PRIVATEBURN_DEBUG_INFO("********%a:Buffer: 0x%p********\n",__FUNCTION__,TransferResult->Buffer);
  //PRIVATEBURN_DEBUG_INFO("********%a:BytesTransferred: %d********\n",__FUNCTION__,TransferResult->BytesTransferred);
  switch(This->State){
    case SunxiUsbPrivateBurnSetup:
      PRIVATEBURN_DEBUG_INFO("SUNXI_USB_PRIVATEBURN_SETUP\n");
      ScsICmdHandler(BytesTransferred,Buffer);
      break;

    case SunxiUsbPrivateBurnSendData:  
    case SunxiUsbPrivateBurnReceiveData: 
      CopyMem(This->BaseReceiveBuffer+(This->ActuallyReceivedBytes),\
      TransferResult->Buffer,TransferResult->BytesTransferred);
       
      This->ActuallyReceivedBytes += TransferResult->BytesTransferred;
      if(This->ActuallyReceivedBytes>=This->TryToReceiveBytes){
        PRIVATEBURN_INFO("PrivateBurn ReceiveData transfer finish\n");
        PRIVATEBURN_INFO("begin to write to flash: start 0x%x, size 0x%x\n", FlashStart,This->ActuallyReceivedBytes);
        Status =  SunxiPrivateBurn->FlashIo->SunxiFlashIoWrite(SunxiPrivateBurn->FlashIo,FlashStart, (This->ActuallyReceivedBytes+511)/512, (void *)This->BaseReceiveBuffer);
        if(EFI_ERROR(Status))
        {
          PRIVATEBURN_ERROR("sunxi flash write err: start,0x%x sectors 0x%x\n", FlashStart, (This->ActuallyReceivedBytes+511)/512);
          gCSW.bCSWStatus = 1;
        }
        //clear 
        This->ActuallyReceivedBytes = 0;
        This->TryToReceiveBytes = 0;
        
        PrivateBurnSendResponse();
        break;
      }
       
      UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
                UsbBusSpeedHigh,&MaxPacketSize);
      TxSize = MaxPacketSize;
      UsbIo->Transfer(UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
      EfiUsbEndpointDirectionDeviceRx,&TxSize,(VOID*)This->DataSetUpBuffer);
      break;
    default:
      PRIVATEBURN_ERROR("State Machine Error!\n");
  }
    
  return EFI_SUCCESS;

}

 EFI_STATUS
PrivateBurnTxHandle(
  SUNXI_PRIVATEBURN  *This,
  EFI_USBFN_TRANSFER_RESULT* TransferResult
 )
{ 

  UINT16 MaxPacketSize=0;
  UINTN TxSize =0;
  EFI_USBFN_IO_PROTOCOL *UsbIo=This->UsbIo;

  if(TransferResult->EndpointIndex!=PRIVATEBURN_BULK_ENDPOINT_INDEX){
    return EFI_SUCCESS;
  }
  if(TransferResult->TransferStatus!=UsbTransferStatusComplete){
    PRIVATEBURN_DEBUG_INFO("%a:Transfer has not completed yet\n");
    return EFI_SUCCESS;
  }
  //PRIVATEBURN_DEBUG_INFO("********%a:TransferStatus: %d********\n",__FUNCTION__,TransferResult->TransferStatus);
  //PRIVATEBURN_DEBUG_INFO("********%a:EndpointIndex: %d********\n",__FUNCTION__,TransferResult->EndpointIndex);
  //PRIVATEBURN_DEBUG_INFO("********%a:Direction: %a********\n",__FUNCTION__,TransferResult->Direction==EfiUsbEndpointDirectionDeviceTx?"DeviceTx":"DeviceRx");
  //PRIVATEBURN_DEBUG_INFO("********%a:Buffer: 0x%p********\n",__FUNCTION__,TransferResult->Buffer);
  //PRIVATEBURN_DEBUG_INFO("********%a:BytesTransferred: %d********\n",__FUNCTION__,TransferResult->BytesTransferred);
  switch(This->State){
    case SunxiUsbPrivateBurnSetup:
       break;
    case SunxiUsbPrivateBurnSendData:
             PrivateBurnSendResponse();
       break;
    case SunxiUsbPrivateBurnReceiveData:
       break;
    case SunxiUsbPrivateBurnResponse:
      //PRIVATEBURN_INFO("usb burn send Status finished\n");
      if(USBExitFlag)
      {
        PRIVATEBURN_INFO("Device will shutdown in 3 Secends...\n");
        MicroSecondDelay (3*1000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        USBExitFlag = FALSE;
      }
      //device configured!, prepare for bulk transfer.
      UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
                UsbBusSpeedHigh,&MaxPacketSize);
      TxSize = MaxPacketSize;
      //set up Bulk receive buffer for the controller.
      UsbIo->Transfer(UsbIo,PRIVATEBURN_BULK_ENDPOINT_INDEX,\
      EfiUsbEndpointDirectionDeviceRx,&TxSize,(VOID*)This->DataSetUpBuffer);
      if(This->NextState==SunxiUsbPrivateBurnReceiveData){
        This->ActuallyReceivedBytes = 0; 
      }
      //response done, enter the next state.
      This->State  = This->NextState;
    
      break;
    default:
      PRIVATEBURN_ERROR("State Machine Error!\n");
  }
    
  return EFI_SUCCESS;

}
 
EFI_STATUS
PrivateBurnUsbIoStateLoop (
  SUNXI_PRIVATEBURN        *This
  )
{
  
  EFI_USBFN_MESSAGE Message;
  UINTN PayloadSize=0;
  EFI_USBFN_MESSAGE_PAYLOAD Payload;
  
  This->UsbIo->EventHandler(This->UsbIo,&Message,&PayloadSize,&Payload);

  switch(Message){    
    case EfiUsbMsgSetupPacket:
      DEBUG (( EFI_D_INFO,"\n\n"));
      PRIVATEBURN_DEBUG_INFO("Message:EfiUsbMsgSetupPacket\n");
      PrivateBurnUsbIoSetUpHandle(This,&(Payload.udr));

      break;
      
    case EfiUsbMsgEndpointStatusChangedRx:
      DEBUG (( EFI_D_INFO,"\n\n"));
      PRIVATEBURN_DEBUG_INFO("Message:EfiUsbMsgEndpointStatusChangedRx\n");
      PrivateBurnRxHandle(This,&(Payload.utr));      
      break;
      
    case EfiUsbMsgEndpointStatusChangedTx:
      DEBUG (( EFI_D_INFO,"\n\n"));
      PRIVATEBURN_DEBUG_INFO("Message:EfiUsbMsgEndpointStatusChangedTx\n");
      PrivateBurnTxHandle(This,&(Payload.utr));  
      break;
      
    case EfiUsbMsgBusEventDetach:
      DEBUG (( EFI_D_INFO,"\n\n"));
      PRIVATEBURN_DEBUG_INFO("Message:EfiUsbMsgBusEventDetach\n\n");

      break;

    case EfiUsbMsgBusEventAttach:
      DEBUG (( EFI_D_INFO,"\n\n"));
      PRIVATEBURN_DEBUG_INFO("Message:EfiUsbMsgBusEventAttach\n\n");

      break;
      
    case EfiUsbMsgBusEventReset:
      /*reset the SW state machine here.*/
      This->State = SunxiUsbPrivateBurnSetup;
      DEBUG (( EFI_D_INFO,"\n\n"));
      PRIVATEBURN_DEBUG_INFO("Message:EfiUsbMsgBusEventReset\n\n");

      break;

    case EfiUsbMsgBusEventSuspend:
      DEBUG (( EFI_D_INFO,"\n\n"));
      PRIVATEBURN_DEBUG_INFO("Message:EfiUsbMsgBusEventSuspend\n\n");

      break;
      
    case EfiUsbMsgBusEventResume:
      DEBUG (( EFI_D_INFO,"\n\n"));
      PRIVATEBURN_DEBUG_INFO("Message:EfiUsbMsgBusEventResume\n\n");

      break;

    case EfiUsbMsgBusEventSpeed:
      DEBUG (( EFI_D_INFO,"\n\n"));
      PRIVATEBURN_DEBUG_INFO("Message:EfiUsbMsgBusEventSpeed\n\n");

      break;

    case EfiUsbMsgNone:

      break;
  }
  
  return EFI_SUCCESS;
}

/**

  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  
  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  
@retval EFI_SUCCESS       The entry point is executed successfully.
@retval other             Some error occurs when executing this entry point.

**/

EFI_STATUS
EFIAPI
PrivateBurnMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )

{

  EFI_STATUS Status = EFI_SUCCESS;
  
  EFI_INPUT_KEY Key;
  
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *TextIn;

  SunxiPrivateBurn = AllocatePool(sizeof(SUNXI_PRIVATEBURN));
  if(!SunxiPrivateBurn){
    Status = EFI_OUT_OF_RESOURCES;  
    goto PrivateBurnMainOut;
  }

  Status = gBS->LocateProtocol (&gEfiSimpleTextInProtocolGuid, NULL, (VOID **) &TextIn);
  if (EFI_ERROR (Status)) {
    PRIVATEBURN_ERROR("Couldn't open Text Input Protocol: %r\n", Status);
    goto PrivateBurnMainOut;
  }

  Status = PrivateBurnUsbIoInitialize(SunxiPrivateBurn,100,100);
  if(Status){
    PRIVATEBURN_ERROR("PrivateBurnUsbIoInitialize error\n");
    goto PrivateBurnMainOut;
  }
 
  // Disable watchdog
  Status = gBS->SetWatchdogTimer (0, 0x10000, 0, NULL);
  if (EFI_ERROR (Status)) {
    PRIVATEBURN_ERROR("PrivateBurn: Couldn't disable watchdog timer: %r\n", Status);
  }

  SunxiPrivateBurn->State = SunxiUsbPrivateBurnSetup;
  SunxiPrivateBurn->NextState = SunxiUsbPrivateBurnSetup;
     // Talk to the user
  PRIVATEBURN_INFO("Allwinner  PrivateBurn - version " ALLWINNER_PRIVATEBURN_VERSION ". Press Q to quit.\r\n");

  while(1){
    PrivateBurnUsbIoStateLoop(SunxiPrivateBurn);
    Status = TextIn->ReadKeyStroke(TextIn,&Key);
    if(Status==EFI_SUCCESS){
      if (Key.UnicodeChar == 113) {
        break;
      }
    }
  }

  Status = PrivateBurnUsbIoExit(SunxiPrivateBurn);
  
PrivateBurnMainOut:
  if(SunxiPrivateBurn){
    FreePool(SunxiPrivateBurn);
  }

  return Status;  
}

  

 
