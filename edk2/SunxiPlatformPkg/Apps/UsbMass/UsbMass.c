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
#include <Library/IoLib.h>

#include <IndustryStandard/Usb.h>
#include <IndustryStandard/Scsi.h>
#include <Protocol/EfiUsbFnIo.h>
#include <Protocol/SunxiFlashIo.h>
#include <Sunxi_type/Sunxi_type.h>
#include <Interinc/sunxi_uefi.h>

#include <UsbMass.h>

#define SCSI_READ_FORMAT_CAPACITY (0x23)


STATIC UINTN MassPartitionStart  = 0;
STATIC UINTN MassPartitionLen    = 0;
STATIC UINTN FlashStart        = 0;
STATIC UINTN FlashSectors      = 0;
//STATIC BOOLEAN USBExitFlag     = FALSE;
STATIC UMASS_CSW_T   gCSW      = {0};

STATIC SUNXI_USBMASS* SunxiUsbMass;

char SunxiUsbMassNormalLangID[8]     = {0x04, 0x03, 0x09, 0x04, '\0'};
char  *SunxiUsbMassDev[SUNXI_USBMASS_DEV_MAX] = {SunxiUsbMassNormalLangID,           \
                                  SUNXI_USBMASS_DEVICE_SERIAL_NUMBER, \
                                SUNXI_USBMASS_DEVICE_MANUFACTURER,  \
                                SUNXI_USBMASS_DEVICE_PRODUCT
                                };
CONST UINT8 UsbMass_RequestSense[20] = {0x07,0x00,0x02,0x00,0x00,0x00,0x00,0x0a,0x00,0x00,0x00,0x00,0x3a,0x00,0x00,0x00,0x00,0x00};

CONST UINT8 UsbMass_InquiryData[40]  = {0x00, 0x80, 0x02, 0x02, 0x1f,                                  \
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
  SUNXI_USBMASS_DEVICE_STRING_MANUFACTURER_INDEX, //StrManufacturer
  SUNXI_USBMASS_DEVICE_STRING_PRODUCT_INDEX,      //StrProduct
  SUNXI_USBMASS_DEVICE_STRING_SERIAL_NUMBER_INDEX,//StrSerialNumber
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
    // I found these values in the UsbMass code
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
      2,                                                //EndpointAddress;
      0x2,                                              //Attributes;
      512,                                      //MaxPacketSize;
      0                                                //Interval;
  }
};




STATIC EFI_STATUS 
UsbMassGetDescriptor(
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
      USBMASS_DEBUG_INFO("get string descriptor\n");
      /* Language ID */
      if(StringIndex == 0)
      {
        bLength = MIN(4, Req->Length);
        This->Transfer(This,0,EfiUsbEndpointDirectionDeviceTx,&bLength,(VOID*)SunxiUsbMassDev[0]);
      }
      else if(StringIndex < SUNXI_USBMASS_DEV_MAX)
      {
        /* Size of string in chars */
        UINTN i = 0;
        UINTN StringLengh = AsciiStrLen ((CONST CHAR8*)SunxiUsbMassDev[StringIndex]);
        bLength  = 2 + (2 * StringLengh);
        SunxiUsbMass->ControlSetUpBuffer[0] = bLength;        /* length */
        SunxiUsbMass->ControlSetUpBuffer[1] = USB_DESC_TYPE_STRING;  /* descriptor = string */

        /* Copy device string to fifo, expand to simple unicode */
        for(i = 0; i < StringLengh; i++)
        {
          SunxiUsbMass->ControlSetUpBuffer[2+ 2*i + 0] = SunxiUsbMassDev[StringIndex][i];
          SunxiUsbMass->ControlSetUpBuffer[2+ 2*i + 1] = 0;
        }
        bLength = MIN(bLength, Req->Length);
        This->Transfer(This,USBMASS_CONTROL_ENDPOINT_INDEX,EfiUsbEndpointDirectionDeviceTx,&bLength,(VOID*)SunxiUsbMass->ControlSetUpBuffer);
      }
      else
      {
        USBMASS_ERROR("sunxi usb err: string line %d is not supported\n", StringIndex);
      }
    
    }
    break;

    case 0X06:
    {
      USBMASS_DEBUG_INFO("get qualifier descriptor\n");
    }
    break;

    default:
      USBMASS_ERROR("err: unkown wValue(%d)\n", Req->Value);
      Status = EFI_DEVICE_ERROR;
  }

  return Status;
}


STATIC EFI_STATUS UsbMassUsbIoInitialize(
  IN OUT SUNXI_USBMASS*           This,
  IN UINTN                              ConnectionTimeout,
  IN UINTN                              ReadWriteTimeout
 )
{

  EFI_STATUS      Status = EFI_SUCCESS;
  CHAR8 *GptDataBuffer =NULL;
  EFI_USB_DEVICE_INFO    *DeviceInfo = NULL;
  EFI_USB_CONFIG_INFO    *ConfigInfoTable =NULL;
  EFI_USB_INTERFACE_INFO *InterfaceInfo = NULL;
  EFI_USB_ENDPOINT_DESCRIPTOR *EndpointDescriptor =NULL;
  EFI_USBFN_IO_PROTOCOL *UsbIo;
  
  UINT16 Pid,Vid;
  UINT16 MaxPacketSize;
    //EFI_LBA LastBlock;
  //CHAR8   *PartName = "Data";
  
  Status = gBS->LocateProtocol (&gSunxiFlashIoProtocolGuid, NULL, (VOID **) &This->FlashIo);
  if (EFI_ERROR (Status)) {
    USBMASS_ERROR("Couldn't open Sunxi Flash Io Protocol: %r\n", Status);
    return Status;
  }
    
  Status = gBS->LocateProtocol(&gEfiUsbFnIoProtocolGuid, NULL, (VOID **)&This->UsbIo);
  ASSERT_EFI_ERROR(Status);

  UsbIo  = This->UsbIo;

  GptDataBuffer = AllocatePool(SUNXI_MBR_SIZE);    
  if(GptDataBuffer == NULL){   
    Status = EFI_OUT_OF_RESOURCES;  
    return Status;
  }

  //
  Status  = SunxiUsbMass->FlashIo->SunxiFlashIoInit(SunxiUsbMass->FlashIo,1);
  if(EFI_ERROR(Status))
  {
    USBMASS_ERROR("FlashIO init Fail\n");
  }

  /* Export whole device */
  MassPartitionStart = 0;
  MassPartitionLen = SunxiUsbMass->FlashIo->BlockIo->Media->LastBlock - MassPartitionStart;

  USBMASS_INFO("SunxiUsbMass->FlashIo->LogicOffSetBlock = 0x%x\n", SunxiUsbMass->FlashIo->LogicOffSetBlock);
  USBMASS_INFO("MassPart Start Addr = 0x%x\n", MassPartitionStart);
  USBMASS_INFO("MassPart Size = %dM\n", MassPartitionLen/2048);
  FreePool(GptDataBuffer);

  Status = UsbIo->AllocateTransferBuffer(UsbIo,USBMASS_CONTROL_SET_UP_BUFFER_SIZE,(VOID**)&This->ControlSetUpBuffer);
  if(Status){
    USBMASS_ERROR("Allocate ControlSetupBuffer failed\n");  
    return Status;
  }

  Status = UsbIo->AllocateTransferBuffer(UsbIo,USBMASS_DATA_SET_UP_BUFFER_SIZE,(VOID**)&This->DataSetUpBuffer);
  if(Status){
    USBMASS_ERROR("Allocate DataSetupBuffer failed\n"); 
    goto ErrorOut1;
  }
  
  This->BaseReceiveBuffer = (UINT8*)AllocatePool(USBMASS_DATA_RECIVE_BUFFER_SIZE);
    if(!This->BaseReceiveBuffer){
    USBMASS_ERROR("Allocate DataRecvieBuffer failed\n");
    Status = EFI_DEVICE_ERROR;
    goto ErrorOut2;
  }
  USBMASS_DEBUG_INFO("DataRecvieBuffer = 0x%x\n",This->BaseReceiveBuffer);
  
  This->BaseSendBuffer = (UINT8 *)AllocatePool(USBMASS_DATA_SEND_BUFFER_SIZE);
  if(!This->BaseSendBuffer){
    USBMASS_ERROR("Allocate DataSendBuffer failed\n");  
    Status = EFI_DEVICE_ERROR;
    goto ErrorOut3;
  }
  USBMASS_DEBUG_INFO("BaseSendBuffer = 0x%x\n",This->BaseSendBuffer);

  USBMASS_DEBUG_INFO("Start Controller\n");
  Status = UsbIo->StartController(UsbIo);
  if(Status){
    USBMASS_DEBUG_INFO("StartController failed\n"); 
    return Status;
  }
  DeviceInfo = AllocateZeroPool(sizeof(EFI_USB_DEVICE_INFO));
  if(!DeviceInfo){
    USBMASS_ERROR("AllocateZeroPool DeviceInfo failed\n");  
    return EFI_DEVICE_ERROR;
  }

  ConfigInfoTable = AllocateZeroPool(sizeof(EFI_USB_CONFIG_INFO));
  if(!ConfigInfoTable){
    USBMASS_ERROR("AllocateZeroPool ConfigInfoTable failed\n"); 
    return EFI_DEVICE_ERROR;
  }
  
  InterfaceInfo = AllocateZeroPool(sizeof(EFI_USB_INTERFACE_INFO));
  if(!InterfaceInfo){
    USBMASS_ERROR("AllocateZeroPool InterfaceInfo failed\n"); 
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
    USBMASS_ERROR("Get pid/vid failed!\n");
    goto Out;
  }
  //DeviceInfo->DeviceDescriptor->IdVendor = Vid;
  //DeviceInfo->DeviceDescriptor->IdProduct = Pid;

  //fix the MaxPacketSize in EndPointDescripter
  Status =UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,UsbBusSpeedHigh,&MaxPacketSize);
  if(Status){
    USBMASS_ERROR("Get MaxPacketSize failed!\n");
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
  USBMASS_DEBUG_INFO("Configure and enable endpoints\n"); 
  Status = UsbIo->ConfigureEnableEndpoints(UsbIo,DeviceInfo);
  if(Status){
    USBMASS_ERROR("enable endpoint failed!\n");
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

STATIC EFI_STATUS UsbMassUsbIoExit(
  SUNXI_USBMASS *This
 )
{

  EFI_STATUS      Status = EFI_SUCCESS;
  Status = This->UsbIo->StopController(This->UsbIo);
  if(Status){
    USBMASS_ERROR("Stop controller failed!\n");
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
UsbMassUsbIoWrite (
  IN UINTN                              NumberOfBytesToWrite,
  IN OUT UINTN                          *NumberOfBytesWrite,
  OUT VOID                              *Buffer
  )
{
  USBMASS_DEBUG_INFO("%a called!\n",__FUNCTION__);
  return EFI_UNSUPPORTED;
}


EFI_STATUS
UsbMassUsbIoRead (
  IN UINTN                              NumberOfBytesToRead,
  IN OUT UINTN                          *NumberOfBytesRead,
  OUT VOID                              *Buffer
  )
{
  USBMASS_DEBUG_INFO("%a called!\n",__FUNCTION__);
  return EFI_UNSUPPORTED;
}



EFI_STATUS
UsbMassUsbIoSetUpHandle(
  SUNXI_USBMASS  *This,
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
          USBMASS_DEBUG_INFO("standard_req:USB_REQ_GET_STATUS\n");
        }
        break;
      }
      case USB_REQ_CLEAR_FEATURE:   //   0x01
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
            USBMASS_DEBUG_INFO("standard_req:USB_REQ_CLEAR_FEATURE\n");
        }
        break;
      }
      case USB_REQ_SET_FEATURE:   //   0x03
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
           USBMASS_DEBUG_INFO("standard_req:USB_REQ_SET_FEATURE\n");
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
            USBMASS_DEBUG_INFO("standard_req:USB_REQ_SET_ADDRESS\n");
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
              USBMASS_DEBUG_INFO("standard_req:USB_REQ_GET_DESCRIPTOR\n");
            Status  = UsbMassGetDescriptor(UsbIo,Req);
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
            USBMASS_DEBUG_INFO("standard_req:USB_REQ_SET_DESCRIPTOR\n");            
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
            USBMASS_DEBUG_INFO("standard_req:USB_REQ_GET_CONFIG\n");
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
             USBMASS_DEBUG_INFO("standard_req:USB_REQ_SET_CONFIG\n");

             //device configured!, prepare for bulk transfer.
             UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
                        UsbBusSpeedHigh,&MaxPacketSize);
             TxSize = MaxPacketSize;
             //set up Bulk receive buffer for the controller.
             UsbIo->Transfer(UsbIo,USBMASS_BULK_ENDPOINT_INDEX,\
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
            USBMASS_DEBUG_INFO("standard_req:USB_REQ_GET_INTERFACE\n");
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
            USBMASS_DEBUG_INFO("standard_req:USB_REQ_SET_INTERFACE\n");
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
            USBMASS_DEBUG_INFO("standard_req:USB_REQ_SYNCH_FRAME\n");
          }
        }
        break;
      }

      default:
      {
        USBMASS_DEBUG_INFO("ep0 control unsupported request:Request=0x%x\n",Req->Request);
        break;
      }
    }
  }
  else
  {
    switch(Req->RequestType)    //PBURN ????????
    {
      case 161:
        if(Req->Request == 0xFE)
        {
          USBMASS_DEBUG_INFO("ask for max lun\n");
          TxSize = 1;
          SunxiUsbMass->ControlSetUpBuffer[0] = 0;
          This->UsbIo->Transfer(This->UsbIo,USBMASS_CONTROL_ENDPOINT_INDEX, \
          EfiUsbEndpointDirectionDeviceTx,&TxSize,(VOID*)SunxiUsbMass->ControlSetUpBuffer);

          //buffer[0] = 0;
          //sunxi_udc_send_setup(1, buffer);
        }
        else
        {
          USBMASS_DEBUG_INFO("unknown ep0 req\n");
        }
        break;

      default:
        USBMASS_DEBUG_ERROR("unknown non standard ep0 req\n");
        break;
    }
    USBMASS_DEBUG_INFO("ep0 control non-standard request:Request=0x%x\n",Req->Request);
  }
  return Status;
}

STATIC EFI_STATUS 
UsbMassSendResponse(VOID)
{
  UINTN Length;
  Length = sizeof(UMASS_CSW_T);
  //gCSW.bCSWStatus    = Status;
  CopyMem(SunxiUsbMass->DataSetUpBuffer,&gCSW,Length);
  SunxiUsbMass->UsbIo->Transfer(SunxiUsbMass->UsbIo,USBMASS_BULK_ENDPOINT_INDEX,EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiUsbMass->DataSetUpBuffer);
  SunxiUsbMass->State = SunxiUsbMassResponse;
  SunxiUsbMass->NextState = SunxiUsbMassSetup;
  return EFI_SUCCESS;
}


u32 ConvertEndian(u32 data)
{
   return ((data & 0xff000000) >> 24)
         | ((data & 0x00ff0000) >>  8)
         | ((data & 0x0000ff00) <<  8)
         | ((data & 0x000000ff) << 24);
}


STATIC VOID ScsICmdHandler(IN UINTN Size,IN CONST CHAR8 * Data)
{
  UMASS_CBW_T* Cbw;
  UINT8 StdCommand;
  UINTN Length;
  UINT32 Sectors;
  UINT32 Start;
  EFI_STATUS Status;
    
  Cbw = (UMASS_CBW_T*)Data;
  StdCommand = Cbw->CBWCDB[0];

  if(CBWSIGNATURE != Cbw->dCBWSignature)
  {
    USBMASS_ERROR("the cbw signature 0x%x is bad, need 0x%x\n", Cbw->dCBWSignature, CBWSIGNATURE);
    return ;
  }

  gCSW.dCSWSignature = CSWSIGNATURE;
  gCSW.dCSWTag     = Cbw->dCBWTag;
  gCSW.bCSWStatus    = 0;
    
  switch(StdCommand)
  {
  case EFI_SCSI_OP_TEST_UNIT_READY:
    USBMASS_DEBUG_INFO("SCSI_TST_U_RDY\n");
    UsbMassSendResponse();
    break;

  case EFI_SCSI_OP_REQUEST_SENSE:
    USBMASS_DEBUG_INFO("SCSI_REQ_SENSE\n");
    USBMASS_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);

    Length = MIN(Cbw->dCBWDataTransferLength, 18);
    CopyMem(SunxiUsbMass->DataSetUpBuffer, UsbMass_RequestSense,Length);
    SunxiUsbMass->UsbIo->Transfer(SunxiUsbMass->UsbIo,USBMASS_BULK_ENDPOINT_INDEX,\
    EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiUsbMass->DataSetUpBuffer);
    SunxiUsbMass->State = SunxiUsbMassSendData;
    break;

    case EFI_SCSI_OP_VERIFY:
    USBMASS_DEBUG_INFO("SCSI_VERIFY\n");
    USBMASS_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
    UsbMassSendResponse();
    break;

  case EFI_SCSI_OP_INQUIRY:
    USBMASS_DEBUG_INFO("SCSI_INQUIRY\n");
    USBMASS_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);

    Length= MIN(Cbw->dCBWDataTransferLength, 36);
    CopyMem(SunxiUsbMass->DataSetUpBuffer, UsbMass_InquiryData,Length);
    SunxiUsbMass->UsbIo->Transfer(SunxiUsbMass->UsbIo,USBMASS_BULK_ENDPOINT_INDEX,\
    EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiUsbMass->DataSetUpBuffer);
    SunxiUsbMass->State = SunxiUsbMassSendData;

    break;

  case EFI_SCSI_OP_MODE_SEN6:
    USBMASS_DEBUG_INFO("SCSI_MODE_SEN6\n");
    USBMASS_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
      
    SunxiUsbMass->DataSetUpBuffer[0] = 3;
    SunxiUsbMass->DataSetUpBuffer[1] = 0;   //??????????0
    SunxiUsbMass->DataSetUpBuffer[2] = 0;   //?豸???????, ?0
    SunxiUsbMass->DataSetUpBuffer[3] = 0;   //??????????????????0

    Length= MIN(Cbw->dCBWDataTransferLength, 4);
    SunxiUsbMass->UsbIo->Transfer(SunxiUsbMass->UsbIo,USBMASS_BULK_ENDPOINT_INDEX,\
    EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiUsbMass->DataSetUpBuffer);
    SunxiUsbMass->State = SunxiUsbMassSendData;
    break;

  case EFI_SCSI_OP_READ_CAPACITY:
    USBMASS_DEBUG_INFO("SCSI_RD_CAPAC\n");
    USBMASS_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
        
    ZeroMem(SunxiUsbMass->DataSetUpBuffer, 8);
    //SunxiUsbMass->DataSetUpBuffer[2] = 0x80;
    *((u32*)SunxiUsbMass->DataSetUpBuffer) = ConvertEndian(MassPartitionLen);
    SunxiUsbMass->DataSetUpBuffer[6] = 2;

    Length= MIN(Cbw->dCBWDataTransferLength, 8);
    SunxiUsbMass->UsbIo->Transfer(SunxiUsbMass->UsbIo,USBMASS_BULK_ENDPOINT_INDEX,\
    EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiUsbMass->DataSetUpBuffer);
    SunxiUsbMass->State = SunxiUsbMassSendData;

    break;

  case SCSI_READ_FORMAT_CAPACITY:
    USBMASS_DEBUG_INFO("SCSI_RD_FMT_CAPAC\n");
    USBMASS_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
    
    ZeroMem(SunxiUsbMass->DataSetUpBuffer, 12);

    //SunxiUsbMass->DataSetUpBuffer[2] = 0x80;
    *((u32*)SunxiUsbMass->DataSetUpBuffer) = ConvertEndian(MassPartitionLen);
            
    SunxiUsbMass->DataSetUpBuffer[6] = 2;
    SunxiUsbMass->DataSetUpBuffer[8] = 2;
    SunxiUsbMass->DataSetUpBuffer[10] = 2;
            
    Length= MIN(Cbw->dCBWDataTransferLength, 12);
            
    SunxiUsbMass->UsbIo->Transfer(SunxiUsbMass->UsbIo,USBMASS_BULK_ENDPOINT_INDEX,\
    EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiUsbMass->DataSetUpBuffer);
    SunxiUsbMass->State = SunxiUsbMassSendData;
    break;

  case EFI_SCSI_OP_MEDIA_REMOVAL:
    USBMASS_DEBUG_INFO("SCSI_MED_REMOVL\n");
    UsbMassSendResponse();
    break;
        
    //case EFI_SCSI_OP_READ6:
    case EFI_SCSI_OP_READ10:    //HOST READ, not this device read
    USBMASS_DEBUG_INFO("SCSI_READ\n");
    USBMASS_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);

    Start = (Cbw->CBWCDB[2]<<24) | Cbw->CBWCDB[3]<<16 | Cbw->CBWCDB[4]<<8 | Cbw->CBWCDB[5]<<0;
    Sectors = (Cbw->CBWCDB[7]<<8) | Cbw->CBWCDB[8];

    Length = MIN(Cbw->dCBWDataTransferLength,Sectors*512);
    Status = SunxiUsbMass->FlashIo->SunxiFlashIoRead(SunxiUsbMass->FlashIo,Start + MassPartitionStart, Sectors, SunxiUsbMass->DataSetUpBuffer);
    if(EFI_ERROR(Status))
    {
      gCSW.bCSWStatus = 1;
    }
    SunxiUsbMass->UsbIo->Transfer(SunxiUsbMass->UsbIo,USBMASS_BULK_ENDPOINT_INDEX,\
    EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiUsbMass->DataSetUpBuffer);
    SunxiUsbMass->State = SunxiUsbMassSendData;
           
    break;

    //case EFI_SCSI_OP_WRITE6:
    case EFI_SCSI_OP_WRITE10:   //HOST WRITE, not this device write
    USBMASS_DEBUG_INFO("SCSI_WRITE\n");
    USBMASS_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);

    FlashStart   = (Cbw->CBWCDB[2]<<24) | Cbw->CBWCDB[3]<<16 | Cbw->CBWCDB[4]<<8 | Cbw->CBWCDB[5]<<0;
    FlashSectors = (Cbw->CBWCDB[7]<<8) | Cbw->CBWCDB[8];
          
            
    FlashStart += MassPartitionStart;
            
    Length = MIN(Cbw->dCBWDataTransferLength, FlashSectors * 512);

    USBMASS_DEBUG_INFO("try to receive data 0x%x\n", Length);
    USBMASS_DEBUG_INFO("command write start: 0x%x, sectors 0x%x\n", FlashStart, FlashSectors);

    SunxiUsbMass->TryToReceiveBytes = Length;
    if (SunxiUsbMass->TryToReceiveBytes == 0 ||
    SunxiUsbMass->TryToReceiveBytes > SUNXI_USBMASS_BUFFER_MAX) {
      USBMASS_ERROR("ERROR: bytes to write:0x%x ,buf max len is 0x%x.\n",SunxiUsbMass->TryToReceiveBytes, SUNXI_USBMASS_BUFFER_MAX);
      gCSW.bCSWStatus = 1;
      UsbMassSendResponse();
      break;
    }

    SunxiUsbMass->ActuallyReceivedBytes = 0;
    SunxiUsbMass->UsbIo->Transfer(SunxiUsbMass->UsbIo,USBMASS_BULK_ENDPOINT_INDEX,\
    EfiUsbEndpointDirectionDeviceRx,&Length,(VOID*)SunxiUsbMass->DataSetUpBuffer);
    SunxiUsbMass->State = SunxiUsbMassReceiveData;
    break;

  default:
    USBMASS_DEBUG_INFO("not supported command 0x%x now\n", Cbw->CBWCDB[0]);
    USBMASS_DEBUG_INFO("asked size 0x%x\n", Cbw->dCBWDataTransferLength);
    gCSW.bCSWStatus = 1;
    UsbMassSendResponse();
    break;
  }
}

EFI_STATUS
UsbMassRxHandle(
  SUNXI_USBMASS  *This,
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
  
  if(TransferResult->EndpointIndex!=USBMASS_BULK_ENDPOINT_INDEX){
    return EFI_SUCCESS;
  }
  if(TransferStatus!=UsbTransferStatusComplete){
    USBMASS_DEBUG_INFO("%a:Transfer has not completed yet\n");
    return EFI_SUCCESS;
  }
  //USBMASS_DEBUG_INFO("********%a:TransferStatus: %d********\n",__FUNCTION__,TransferResult->TransferStatus);
  //USBMASS_DEBUG_INFO("********%a:EndpointIndex: %d********\n",__FUNCTION__,TransferResult->EndpointIndex);
  //USBMASS_DEBUG_INFO("********%a:Direction: %a********\n",__FUNCTION__,TransferResult->Direction==EfiUsbEndpointDirectionDeviceTx?"DeviceTx":"DeviceRx");
  //USBMASS_DEBUG_INFO("********%a:Buffer: 0x%p********\n",__FUNCTION__,TransferResult->Buffer);
  //USBMASS_DEBUG_INFO("********%a:BytesTransferred: %d********\n",__FUNCTION__,TransferResult->BytesTransferred);
  switch(This->State){
    case SunxiUsbMassSetup:
       USBMASS_DEBUG_INFO("SUNXI_USB_USBMASSSETUP\n");
       ScsICmdHandler(BytesTransferred,Buffer);
       break;

    case SunxiUsbMassSendData:  
      break;
    case SunxiUsbMassReceiveData: 
      CopyMem(This->BaseReceiveBuffer+(This->ActuallyReceivedBytes),\
      TransferResult->Buffer,TransferResult->BytesTransferred);
       
      This->ActuallyReceivedBytes += TransferResult->BytesTransferred;
      if(This->ActuallyReceivedBytes>=This->TryToReceiveBytes){
        USBMASS_DEBUG_INFO("begin to write to flash: start 0x%x, size 0x%x\n", FlashStart,This->ActuallyReceivedBytes);
        Status =   SunxiUsbMass->FlashIo->SunxiFlashIoWrite(SunxiUsbMass->FlashIo,FlashStart, 
          (This->ActuallyReceivedBytes+511)/512, (void *)This->BaseReceiveBuffer);
        if(EFI_ERROR(Status))
        {
          USBMASS_ERROR("sunxi flash write err: start,0x%x sectors 0x%x\n", FlashStart, 
            (This->ActuallyReceivedBytes+511)/512);
          gCSW.bCSWStatus = 1;   
        }
        USBMASS_DEBUG_INFO("write to flash end\n");
        //clear 
        This->ActuallyReceivedBytes = 0;
        This->TryToReceiveBytes = 0;
      
        UsbMassSendResponse();
        break;
      }
       
      UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
                UsbBusSpeedHigh,&MaxPacketSize);
      TxSize = MaxPacketSize;
      UsbIo->Transfer(UsbIo,USBMASS_BULK_ENDPOINT_INDEX,\
      EfiUsbEndpointDirectionDeviceRx,&TxSize,(VOID*)This->DataSetUpBuffer);
      break;
    default:
      USBMASS_ERROR("State Machine Error!\n");
  }
    
  return EFI_SUCCESS;

}

 EFI_STATUS
UsbMassTxHandle(
  SUNXI_USBMASS  *This,
  EFI_USBFN_TRANSFER_RESULT* TransferResult
 )
{ 

  //UINT16 MaxPacketSize=0;
  //UINTN TxSize =0;
  //EFI_USBFN_IO_PROTOCOL *UsbIo=This->UsbIo;

  if(TransferResult->EndpointIndex!=USBMASS_BULK_ENDPOINT_INDEX){
    return EFI_SUCCESS;
  }
  if(TransferResult->TransferStatus!=UsbTransferStatusComplete){
    USBMASS_DEBUG_INFO("%a:Transfer has not completed yet\n");
    return EFI_SUCCESS;
  }
  //USBMASS_DEBUG_INFO("********%a:TransferStatus: %d********\n",__FUNCTION__,TransferResult->TransferStatus);
  //USBMASS_DEBUG_INFO("********%a:EndpointIndex: %d********\n",__FUNCTION__,TransferResult->EndpointIndex);
  //USBMASS_DEBUG_INFO("********%a:Direction: %a********\n",__FUNCTION__,TransferResult->Direction==EfiUsbEndpointDirectionDeviceTx?"DeviceTx":"DeviceRx");
  //USBMASS_DEBUG_INFO("********%a:Buffer: 0x%p********\n",__FUNCTION__,TransferResult->Buffer);
  //USBMASS_DEBUG_INFO("********%a:BytesTransferred: %d********\n",__FUNCTION__,TransferResult->BytesTransferred);
  switch(This->State){
    case SunxiUsbMassSetup:
      break;
    case SunxiUsbMassSendData:
      UsbMassSendResponse();
      break;
    case SunxiUsbMassReceiveData:
      break;
    case SunxiUsbMassResponse:
#if 0
      //device configured!, prepare for bulk transfer.
      UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
              UsbBusSpeedHigh,&MaxPacketSize);
      TxSize = MaxPacketSize;
      //set up Bulk receive buffer for the controller.
      UsbIo->Transfer(UsbIo,USBMASS_BULK_ENDPOINT_INDEX,\
      EfiUsbEndpointDirectionDeviceRx,&TxSize,(VOID*)This->DataSetUpBuffer);
#endif
      if(This->NextState==SunxiUsbMassReceiveData){
        This->ActuallyReceivedBytes = 0; 
      }
      //response done, enter the next state.
      This->State  = This->NextState;

      break;
    default:
      USBMASS_ERROR("State Machine Error!\n");
  }
    
  return EFI_SUCCESS;
}
 
EFI_STATUS
UsbMassUsbIoStateLoop (
  SUNXI_USBMASS        *This
  )
{
  
  EFI_USBFN_MESSAGE Message;
  UINTN PayloadSize=0;
  EFI_USBFN_MESSAGE_PAYLOAD Payload;
  
  This->UsbIo->EventHandler(This->UsbIo,&Message,&PayloadSize,&Payload);

  switch(Message){    
    case EfiUsbMsgSetupPacket:
      USBMASS_DEBUG_INFO ("\n\n");
      USBMASS_DEBUG_INFO("Message:EfiUsbMsgSetupPacket\n");
      UsbMassUsbIoSetUpHandle(This,&(Payload.udr));

      break;
        
    case EfiUsbMsgEndpointStatusChangedRx:
      USBMASS_DEBUG_INFO ("\n\n");
      USBMASS_DEBUG_INFO("Message:EfiUsbMsgEndpointStatusChangedRx\n");
      UsbMassRxHandle(This,&(Payload.utr));      
      break;
        
    case EfiUsbMsgEndpointStatusChangedTx:
      USBMASS_DEBUG_INFO ("\n\n");
      USBMASS_DEBUG_INFO("Message:EfiUsbMsgEndpointStatusChangedTx\n");
      UsbMassTxHandle(This,&(Payload.utr));  
      break;
        
    case EfiUsbMsgBusEventDetach:
      USBMASS_DEBUG_INFO ("\n\n");
      USBMASS_DEBUG_INFO("Message:EfiUsbMsgBusEventDetach\n\n");

      break;
      
    case EfiUsbMsgBusEventAttach:
      USBMASS_DEBUG_INFO ("\n\n");
      USBMASS_DEBUG_INFO("Message:EfiUsbMsgBusEventAttach\n\n");

      break;
      
    case EfiUsbMsgBusEventReset:
      /*reset the SW state machine here.*/
      This->State = SunxiUsbMassSetup;
      USBMASS_DEBUG_INFO ("\n\n");
      USBMASS_DEBUG_INFO("Message:EfiUsbMsgBusEventReset\n\n");

      break;
      
    case EfiUsbMsgBusEventSuspend:
      USBMASS_DEBUG_INFO ("\n\n");
      USBMASS_DEBUG_INFO("Message:EfiUsbMsgBusEventSuspend\n\n");

      break;

    case EfiUsbMsgBusEventResume:
      USBMASS_DEBUG_INFO ("\n\n");
      USBMASS_DEBUG_INFO("Message:EfiUsbMsgBusEventResume\n\n");

      break;

    case EfiUsbMsgBusEventSpeed:
      USBMASS_DEBUG_INFO ("\n\n");
      USBMASS_DEBUG_INFO("Message:EfiUsbMsgBusEventSpeed\n\n");

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
#define GIC_PEND_SET(_n)  (0x01c81000L + 0x200 + 4 * (_n))

EFI_STATUS
EFIAPI
UsbMassMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )

{

  EFI_STATUS Status = EFI_SUCCESS;
  
  EFI_INPUT_KEY Key;
  
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *TextIn;

  SunxiUsbMass = AllocatePool(sizeof(SUNXI_USBMASS));
  if(!SunxiUsbMass){
    Status = EFI_OUT_OF_RESOURCES;  
    goto UsbMassMainOut;
  }

  Status = gBS->LocateProtocol (&gEfiSimpleTextInProtocolGuid, NULL, (VOID **) &TextIn);
  if (EFI_ERROR (Status)) {
    USBMASS_ERROR("Couldn't open Text Input Protocol: %r\n", Status);
    goto UsbMassMainOut;
  }

  Status = UsbMassUsbIoInitialize(SunxiUsbMass,100,100);
  if(Status){
    USBMASS_ERROR("UsbMassUsbIoInitialize error\n");
    goto UsbMassMainOut;
  }
 
  // Disable watchdog
  Status = gBS->SetWatchdogTimer (0, 0x10000, 0, NULL);
  if (EFI_ERROR (Status)) {
    USBMASS_ERROR("UsbMass: Couldn't disable watchdog timer: %r\n", Status);
  }

  SunxiUsbMass->State = SunxiUsbMassSetup;
  SunxiUsbMass->NextState = SunxiUsbMassSetup;
     // Talk to the user
  USBMASS_INFO("Allwinner  UsbMass - version " ALLWINNER_USBMASS_VERSION ". Press Q to quit.\r\n");
  while(1){
    UsbMassUsbIoStateLoop(SunxiUsbMass);
    
    Status = TextIn->ReadKeyStroke(TextIn,&Key);
    if(Status==EFI_SUCCESS){
      if (Key.UnicodeChar == 113) {
        break;
      }
    }
  }

  Status = UsbMassUsbIoExit(SunxiUsbMass);
  
UsbMassMainOut:
  if(SunxiUsbMass){
    FreePool(SunxiUsbMass);
  }

  return Status;  
}

  

 
