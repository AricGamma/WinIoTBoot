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
#include <Library/SunxiPartitionLib.h>
#include <Library/SunxiMbr.h>

#include <IndustryStandard/Usb.h>
#include <Protocol/EfiUsbFnIo.h>
#include <Protocol/SunxiFlashIo.h>
#include <Sunxi_type/Sunxi_type.h>
#include <Interinc/sunxi_uefi.h>
#include <Fastboot.h>

#define MATCH_CMD_LITERAL(Cmd, Buf) !AsciiStrnCmp (Cmd, Buf, sizeof (Cmd) - 1)

STATIC SUNXI_FASTBOOT* SunxiFastboot;

char SunxiFastbootNormalLangID[8]     = {0x04, 0x03, 0x09, 0x04, '\0'};
char  *SunxiUsbFastbootDev[SUNXI_USB_FASTBOOT_DEV_MAX] = {      SunxiFastbootNormalLangID,      \
                                SUNXI_FASTBOOT_DEVICE_MANUFACTURER,     \
                                SUNXI_FASTBOOT_DEVICE_PRODUCT,        \
                                SUNXI_FASTBOOT_DEVICE_SERIAL_NUMBER,    \
                                SUNXI_FASTBOOT_DEVICE_CONFIG,         \
                                SUNXI_FASTBOOT_DEVICE_INTERFACE
                              };


STATIC EFI_USB_DEVICE_DESCRIPTOR mDeviceDescriptor = {
  sizeof (USB_DEVICE_DESCRIPTOR),                  //Length
  USB_DESC_TYPE_DEVICE,                            //DescriptorType
  0x0200,                                          //BcdUSB
  0xff,                                            //DeviceClass
  0xff,                                            //DeviceSubClass
  0xff,                                            //DeviceProtocol
  0x40,                                            //MaxPacketSize0
  DEVICE_VENDOR_ID,                  //IdVendor
  DEVICE_PRODUCT_ID,                   //IdProduct
  DEVICE_BCD,                                      //BcdDevice
  SUNXI_FASTBOOT_DEVICE_STRING_MANUFACTURER_INDEX, //StrManufacturer
  SUNXI_FASTBOOT_DEVICE_STRING_PRODUCT_INDEX,      //StrProduct
  SUNXI_FASTBOOT_DEVICE_STRING_SERIAL_NUMBER_INDEX,//StrSerialNumber
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
    SUNXI_FASTBOOT_DEVICE_STRING_CONFIG_INDEX,        //Configuration;
    0xc0,                                             //Attributes;
    0xFA                                              //MaxPower;
};

EFI_USB_INTERFACE_DESCRIPTOR  mInterfaceDescriptor = {
    sizeof (USB_INTERFACE_DESCRIPTOR), //Length;
    USB_DESC_TYPE_INTERFACE, //DescriptorType;
    0,                                                //InterfaceNumber;
    0,                                                //AlternateSetting;
    2,                                                //NumEndpoints;
    0xFF,                                             //InterfaceClass;
    // Vendor specific interface subclass and protocol codes.
    // I found these values in the Fastboot code
    // (in match_fastboot_with_serial in fastboot.c).
    0x42,                                             //InterfaceSubClass;
    0x03,                                             //InterfaceProtocol;
    SUNXI_FASTBOOT_DEVICE_STRING_INTERFACE_INDEX      //Interface;
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
FastbootGetDescriptor(
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
        This->Transfer(This,0,EfiUsbEndpointDirectionDeviceTx,&bLength,(VOID*)SunxiUsbFastbootDev[0]);
      }
      else if(StringIndex < SUNXI_USB_FASTBOOT_DEV_MAX)
      {
        /* Size of string in chars */
        UINTN i = 0;
        UINTN StringLengh = AsciiStrLen ((CONST CHAR8*)SunxiUsbFastbootDev[StringIndex]);
        bLength  = 2 + (2 * StringLengh);
        SunxiFastboot->ControlSetUpBuffer[0] = bLength;        /* length */
        SunxiFastboot->ControlSetUpBuffer[1] = USB_DESC_TYPE_STRING;  /* descriptor = string */

        /* Copy device string to fifo, expand to simple unicode */
        for(i = 0; i < StringLengh; i++)
        {
          SunxiFastboot->ControlSetUpBuffer[2+ 2*i + 0] = SunxiUsbFastbootDev[StringIndex][i];
          SunxiFastboot->ControlSetUpBuffer[2+ 2*i + 1] = 0;
        }
        bLength = MIN(bLength, Req->Length);
        This->Transfer(This,FASTBOOT_CONTROL_ENDPOINT_INDEX,EfiUsbEndpointDirectionDeviceTx,&bLength,(VOID*)SunxiFastboot->ControlSetUpBuffer);
      }
      else
      {
        FASTBOOT_ERROR("sunxi usb err: string line %d is not supported\n", StringIndex);
      }
    
    }
    break;

    case 0X06:
      {
        FASTBOOT_DEBUG_INFO("get qualifier descriptor\n");
      }
      break;

    default:
      FASTBOOT_ERROR("err: unkown wValue(%d)\n", Req->Value);
      Status = EFI_DEVICE_ERROR;
  }

  return Status;
}


STATIC EFI_STATUS FastbootUsbIoInitialize(
  IN OUT SUNXI_FASTBOOT*          This,
  IN UINTN                              ConnectionTimeout,
  IN UINTN                              ReadWriteTimeout
 )
{

  EFI_STATUS      Status = EFI_SUCCESS;
  EFI_USB_DEVICE_INFO    *DeviceInfo = NULL;
  EFI_USB_CONFIG_INFO    *ConfigInfoTable =NULL;
  EFI_USB_INTERFACE_INFO *InterfaceInfo = NULL;
  EFI_USB_ENDPOINT_DESCRIPTOR *EndpointDescriptor =NULL;
  EFI_USBFN_IO_PROTOCOL *UsbIo;
  CHAR8 *GptPrimaryDataBuffer =NULL;
  CHAR8 *GptBackupDataBuffer =NULL;
  EFI_LBA LastBlock;
  UINT32 PrimaryGptFlag = 0;
  UINT32 BackupGptFlag = 0;
  UINT32 GptInitFlag = 0;
  
  UINT16 Pid,Vid;
  UINT16 MaxPacketSize;
  
  Status = gBS->LocateProtocol (&gSunxiFlashIoProtocolGuid, NULL, (VOID **) &This->FlashIo);
  if (EFI_ERROR (Status)) {
    FASTBOOT_ERROR("Couldn't open Sunxi Flash Io Protocol: %r\n", Status);
    return Status;
  }
    
  Status = gBS->LocateProtocol(&gEfiUsbFnIoProtocolGuid, NULL, (VOID **)&This->UsbIo);
  ASSERT_EFI_ERROR(Status);

  UsbIo  = This->UsbIo;

  // gpt partition operation
  GptPrimaryDataBuffer = AllocatePool(SUNXI_MBR_SIZE);    
  if(GptPrimaryDataBuffer == NULL){   
    return EFI_OUT_OF_RESOURCES;  
  }

  GptBackupDataBuffer = AllocatePool(SUNXI_MBR_SIZE);    
  if(GptBackupDataBuffer == NULL){   
    FreePool(GptPrimaryDataBuffer);
    return EFI_OUT_OF_RESOURCES;  
  }

  // read gpt info
  LastBlock = SunxiFastboot->FlashIo->BlockIo->Media->LastBlock;
  Status = SunxiFastboot->FlashIo->BlockIo->ReadBlocks(SunxiFastboot->FlashIo->BlockIo,SunxiFastboot->FlashIo->BlockIo->Media->MediaId,\
  SUNXI_GPT_PRIMARY_HEADER_LBA,SunxiFastboot->FlashIo->BlockIo->Media->BlockSize,GptPrimaryDataBuffer);  
  if(Status){
    FASTBOOT_ERROR("Read gpt primary header table error, try to read backup table\n");
    Status = SunxiFastboot->FlashIo->BlockIo->ReadBlocks(SunxiFastboot->FlashIo->BlockIo,SunxiFastboot->FlashIo->BlockIo->Media->MediaId,\
    LastBlock,SunxiFastboot->FlashIo->BlockIo->Media->BlockSize,GptBackupDataBuffer);
    if(Status){
      FASTBOOT_ERROR("read backup table failure\n");
      FreePool(GptPrimaryDataBuffer);
      FreePool(GptBackupDataBuffer);
      return EFI_DEVICE_ERROR;
    }
    BackupGptFlag = 1;
  }
  else{
    PrimaryGptFlag = 1;
  }
  // gpt init
  if(PrimaryGptFlag == 1){
    if(SunxiGptPartitionInitialize(SunxiFastboot->FlashIo->BlockIo,GptPrimaryDataBuffer, SUNXI_GPT_SIZE)){
      FASTBOOT_ERROR("init gpt partition by primary table failure, try to backup table\n");
      if(BackupGptFlag == 0){
        Status = SunxiFastboot->FlashIo->BlockIo->ReadBlocks(SunxiFastboot->FlashIo->BlockIo,SunxiFastboot->FlashIo->BlockIo->Media->MediaId,\
        LastBlock,SunxiFastboot->FlashIo->BlockIo->Media->BlockSize,GptBackupDataBuffer);
        if(Status){
          FASTBOOT_ERROR("read backup table failure\n");
          FreePool(GptPrimaryDataBuffer);
          FreePool(GptBackupDataBuffer);
          return EFI_DEVICE_ERROR;
        }
        BackupGptFlag = 1;
      }
      
      if(SunxiGptPartitionInitialize(SunxiFastboot->FlashIo->BlockIo,GptBackupDataBuffer, SUNXI_GPT_SIZE)){
        FASTBOOT_ERROR("init gpt partition by backup table failure\n");
        FreePool(GptPrimaryDataBuffer);
        FreePool(GptBackupDataBuffer);
        return EFI_DEVICE_ERROR;      
      }
      GptInitFlag = 1;
    }
    else{
      GptInitFlag = 1;
    }
  }
  
  if(GptInitFlag == 0){ 
    if(SunxiGptPartitionInitialize(SunxiFastboot->FlashIo->BlockIo,GptBackupDataBuffer, SUNXI_GPT_SIZE)){
      FASTBOOT_ERROR("init gpt partition by backup table failure\n");
      FreePool(GptPrimaryDataBuffer);
      FreePool(GptBackupDataBuffer);
      return EFI_DEVICE_ERROR;        
    }
    GptInitFlag = 1;
  }
  
  FreePool(GptPrimaryDataBuffer);
  FreePool(GptBackupDataBuffer);    

  Status = UsbIo->AllocateTransferBuffer(UsbIo,FASTBOOT_CONTROL_SET_UP_BUFFER_SIZE,(VOID**)&This->ControlSetUpBuffer);
  if(Status){
    FASTBOOT_ERROR("Allocate ControlSetupBuffer failed\n"); 
    return Status;
  }

  Status = UsbIo->AllocateTransferBuffer(UsbIo,FASTBOOT_DATA_SET_UP_BUFFER_SIZE,(VOID**)&This->DataSetUpBuffer);
  if(Status){
    FASTBOOT_ERROR("Allocate DataSetupBuffer failed\n");  
    goto ErrorOut1;
  }
  
  This->BaseReceiveBuffer = (UINT8*)AllocatePool(FASTBOOT_DATA_RECIVE_BUFFER_SIZE);
  if(!This->BaseReceiveBuffer){
    FASTBOOT_ERROR("Allocate DataRecvieBuffer failed\n");
    Status = EFI_DEVICE_ERROR;
    goto ErrorOut2;
  }
  FASTBOOT_DEBUG_INFO("DataRecvieBuffer = 0x%x\n",This->BaseReceiveBuffer);
  
  This->BaseSendBuffer = (UINT8 *)AllocatePool(FASTBOOT_DATA_SEND_BUFFER_SIZE);
  if(!This->BaseSendBuffer){
    FASTBOOT_ERROR("Allocate DataSendBuffer failed\n"); 
    Status = EFI_DEVICE_ERROR;
    goto ErrorOut3;
  }
  FASTBOOT_DEBUG_INFO("BaseSendBuffer = 0x%x\n",This->BaseSendBuffer);

  FASTBOOT_DEBUG_INFO("Start Controller\n");
  Status = UsbIo->StartController(UsbIo);
  if(Status){
    FASTBOOT_DEBUG_INFO("StartController failed\n");  
    return Status;
  }
  DeviceInfo = AllocateZeroPool(sizeof(EFI_USB_DEVICE_INFO));
  if(!DeviceInfo){
    FASTBOOT_ERROR("AllocateZeroPool DeviceInfo failed\n"); 
    return EFI_DEVICE_ERROR;
  }

  ConfigInfoTable = AllocateZeroPool(sizeof(EFI_USB_CONFIG_INFO));
  if(!ConfigInfoTable){
    FASTBOOT_ERROR("AllocateZeroPool ConfigInfoTable failed\n");  
    return EFI_DEVICE_ERROR;
  }
  
  InterfaceInfo = AllocateZeroPool(sizeof(EFI_USB_INTERFACE_INFO));
  if(!InterfaceInfo){
    FASTBOOT_ERROR("AllocateZeroPool InterfaceInfo failed\n");  
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
    FASTBOOT_ERROR("Get pid/vid failed!\n");
    goto Out;
  }
  //DeviceInfo->DeviceDescriptor->IdVendor = Vid;
  //DeviceInfo->DeviceDescriptor->IdProduct = Pid;

  //fix the MaxPacketSize in EndPointDescripter
  Status =UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,UsbBusSpeedHigh,&MaxPacketSize);
  if(Status){
    FASTBOOT_ERROR("Get MaxPacketSize failed!\n");
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
  FASTBOOT_DEBUG_INFO("Configure and enable endpoints\n");  
  Status = UsbIo->ConfigureEnableEndpoints(UsbIo,DeviceInfo);
  if(Status){
    FASTBOOT_ERROR("enable endpoint failed!\n");
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

STATIC EFI_STATUS FastbootUsbIoExit(
  SUNXI_FASTBOOT *This
 )
{

  EFI_STATUS      Status = EFI_SUCCESS;
  Status = This->UsbIo->StopController(This->UsbIo);
  if(Status){
    FASTBOOT_ERROR("Stop controller failed!\n");
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
FastbootUsbIoWrite (
  IN UINTN                              NumberOfBytesToWrite,
  IN OUT UINTN                          *NumberOfBytesWrite,
  OUT VOID                              *Buffer
  )
{
  FASTBOOT_DEBUG_INFO("%a called!\n",__FUNCTION__);
  return EFI_UNSUPPORTED;
}


EFI_STATUS
FastbootUsbIoRead (
  IN UINTN                              NumberOfBytesToRead,
  IN OUT UINTN                          *NumberOfBytesRead,
  OUT VOID                              *Buffer
  )
{
  FASTBOOT_DEBUG_INFO("%a called!\n",__FUNCTION__);
  return EFI_UNSUPPORTED;
}



EFI_STATUS
FastbootUsbIoSetUpHandle(
  SUNXI_FASTBOOT  *This,
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
          FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_GET_STATUS\n");
        }
        break;
      }
      case USB_REQ_CLEAR_FEATURE:   //   0x01
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_CLEAR_FEATURE\n");
        }

        break;
      }
      case USB_REQ_SET_FEATURE:   //   0x03
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_SET_FEATURE\n");
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
            FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_SET_ADDRESS\n");
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
            FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_GET_DESCRIPTOR\n");
            Status  = FastbootGetDescriptor(UsbIo,Req);
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
            FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_SET_DESCRIPTOR\n");          
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
            FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_GET_CONFIG\n");
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
             FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_SET_CONFIG\n");

             //device configured!, prepare for bulk transfer.
             UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
                        UsbBusSpeedHigh,&MaxPacketSize);
             TxSize = MaxPacketSize;
             //set up Bulk receive buffer for the controller.
             UsbIo->Transfer(UsbIo,FASTBOOT_BULK_ENDPOINT_INDEX,\
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
            FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_GET_INTERFACE\n");
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
            FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_SET_INTERFACE\n");
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
            FASTBOOT_DEBUG_INFO("standard_req:USB_REQ_SYNCH_FRAME\n");
          }
        }

        break;
      }

      default:
      {
        FASTBOOT_DEBUG_INFO("ep0 control unsupported request:Request=0x%x\n",Req->Request);
        break;
      }
    }
  }
  else
  {
    FASTBOOT_DEBUG_INFO("ep0 control non-standard request:Request=0x%x\n",Req->Request);
  }
  return Status;
}

STATIC EFI_STATUS 
FastbootSendResponse(
  IN CHAR8* Response
  )
{
  UINTN Length;
  Length  =AsciiStrLen(Response);
  FASTBOOT_DEBUG_INFO("response%d: %a\n",Length, Response);
  CopyMem(SunxiFastboot->DataSetUpBuffer,Response,Length);
  SunxiFastboot->UsbIo->Transfer(SunxiFastboot->UsbIo,FASTBOOT_BULK_ENDPOINT_INDEX,EfiUsbEndpointDirectionDeviceTx,&Length,(VOID*)SunxiFastboot->DataSetUpBuffer);
  SunxiFastboot->State = SunxiUsbFastbootResponse;
  return EFI_SUCCESS;
}

#define FASTBOOT_STRING_MAX_LENGTH  256
#define FASTBOOT_COMMAND_MAX_LENGTH 64


STATIC
VOID
HandleGetVar (
  IN CHAR8 *CmdArg
  )
{
  CHAR8      Response[FASTBOOT_COMMAND_MAX_LENGTH + 1] = "OKAY";
  
  ZeroMem(Response,sizeof(Response));
  AsciiStrCpy(Response,"OKAY");
  if(!AsciiStrCmp(CmdArg, "version"))
  {
    AsciiStrCpy(Response + 4, ALLWINNER_FASTBOOT_VERSION);
  }
  else if(!AsciiStrCmp(CmdArg, "product"))
  {
    AsciiStrCpy(Response + 4, SUNXI_FASTBOOT_DEVICE_PRODUCT);
  }
  else if(!AsciiStrCmp(CmdArg, "serialno"))
  {
    AsciiStrCpy(Response + 4, SUNXI_FASTBOOT_DEVICE_SERIAL_NUMBER);
  }
  else if(!AsciiStrCmp(CmdArg, "downloadsize"))
  {
    AsciiSPrint(Response + 4, sizeof(Response)-4,"0x%08x", SUNXI_USB_FASTBOOT_BUFFER_MAX);
  }
  else if(!AsciiStrCmp(CmdArg, "secure"))
  {
    AsciiStrCpy(Response + 4, "yes");
  }
  else if(!AsciiStrCmp(CmdArg, "max-download-size"))
  {
    AsciiSPrint(Response + 4,sizeof(Response)-4, "0x%08x", SUNXI_USB_FASTBOOT_BUFFER_MAX);    
  }
  else
  {
    AsciiStrCpy(Response + 4, "not supported");
  }
   
  FastbootSendResponse(Response);

}


STATIC
VOID
HandleDownload (
  IN CHAR8 *NumBytesString
  )
{
  CHAR8       Response[13] = "DATA";

  // Parse out number of data bytes to expect
  SunxiFastboot->TryToReceiveBytes = AsciiStrHexToUint64 (NumBytesString);
  FASTBOOT_INFO("DATA:TryToReceiveBytes = %ld\n", SunxiFastboot->TryToReceiveBytes);
  if (SunxiFastboot->TryToReceiveBytes == 0) {
    FASTBOOT_ERROR("ERROR: Fail to get the number of bytes to download.\n");
    FastbootSendResponse("FAILFailed to get the number of bytes to download");
    return;
  }
  else if (SunxiFastboot->TryToReceiveBytes > SUNXI_USB_FASTBOOT_BUFFER_MAX)
  {
    FASTBOOT_ERROR("ERROR:download: data > buffer\n");
    FastbootSendResponse("FAILdownload: data > buffer");
    return;
  }
  SunxiFastboot->ActuallyReceivedBytes = 0;
  AsciiStrnCpy (Response + 4, NumBytesString, 8);
  //AsciiSPrint(Response + 4,sizeof(Response)-4, "0x%08x", SunxiFastboot->TryToReceiveBytes);
  FastbootSendResponse(Response);

  SunxiFastboot->NextState = SunxiUsbFastbootReceiveData;
}

STATIC
VOID
HandleFlash (
  IN CHAR8 *PartitionName
  )
{
  EFI_STATUS  Status;  
  UINT32 PartStart,PartSectors;
  UINTN ReceviedDataSectors;
  UINTN BlockSize;  
  // Build output string
  FASTBOOT_INFO("Flashing partition: %a\r\n", PartitionName);

  Status = SunxiGptPartitionGetInfoByName(PartitionName,&PartStart,&PartSectors);
  if(Status){
    FASTBOOT_DEBUG_ERROR("can't find relevant partition.\n");
    FastbootSendResponse("FAILcan't find relevant partition");
    return;
  }

  FASTBOOT_INFO("partition %a: Start:0x%x,Size=0x%x\n",PartitionName,PartStart,PartSectors);

  if(SunxiFastboot->ActuallyReceivedBytes!=SunxiFastboot->TryToReceiveBytes){
    FASTBOOT_DEBUG_ERROR("received data integrity error\n");
    FastbootSendResponse("FAILreceived data integrity error");
    return;
  }

  if (SunxiFastboot->BaseReceiveBuffer == NULL) {
    // Doesn't look like we were sent any data
    FastbootSendResponse("FAILNo data to flash");
    return;
  }

  BlockSize = SunxiFastboot->FlashIo->BlockIo->Media->BlockSize;
  ReceviedDataSectors = (SunxiFastboot->ActuallyReceivedBytes+BlockSize-1)/BlockSize;
  if(ReceviedDataSectors>PartSectors){
    
    FASTBOOT_DEBUG_ERROR("flash FAIL: partition %s size 0x%x is smaller than data size 0x%x\n",\
    PartitionName, PartSectors*BlockSize, ReceviedDataSectors*BlockSize);
    FastbootSendResponse("FAILdownload: partition size < data size");
    return;
  }
  
  Status = SunxiFastboot->FlashIo->SunxiFlashIoWrite(SunxiFastboot->FlashIo,\
  PartStart,ReceviedDataSectors,SunxiFastboot->BaseReceiveBuffer);

  if (Status) {
    FASTBOOT_DEBUG_ERROR("Write partition %a to flash error\n",PartitionName);
    FastbootSendResponse("FAILFlash:write partition to flash error\n");
  }
  FASTBOOT_INFO("Flashing done\n\n");
  FastbootSendResponse("OKAY");
}

STATIC
VOID
HandleErase (
  IN CHAR8 *PartitionName
  )
{
  EFI_STATUS  Status;  
  UINT32 PartStart,PartSectors;
  UINTN UnerasedSectors,BufferSectors;
  UINTN BlockSize;  
  UINT8* EraseBuffer;
  // Build output string
  FASTBOOT_INFO("Erase partition: %a\r\n", PartitionName);

  Status = SunxiGptPartitionGetInfoByName(PartitionName,&PartStart,&PartSectors);
  if(Status){
    FASTBOOT_DEBUG_ERROR("can't find relevant partition.\n");
    FastbootSendResponse("FAILcan't find relevant partition");
    return;
  }

  FASTBOOT_INFO("partition %a: Start:0x%x,Size=0x%x\n",PartitionName,PartStart,PartSectors);

  EraseBuffer = AllocatePool(FASTBOOT_ERASE_BUFFER_SIZE);
  if(!EraseBuffer){
    FASTBOOT_DEBUG_ERROR("can't allocate erase buffer.\n");
    FastbootSendResponse("FAILcan't allocate erase buffer");
    return;
  }
  SetMem(EraseBuffer,FASTBOOT_ERASE_BUFFER_SIZE,~0);
  
  UnerasedSectors = PartSectors;
  BlockSize = SunxiFastboot->FlashIo->BlockIo->Media->BlockSize;
  BufferSectors = FASTBOOT_ERASE_BUFFER_SIZE/BlockSize;

  while(UnerasedSectors >= BufferSectors)
  {
    Status = SunxiFastboot->FlashIo->SunxiFlashIoWrite(SunxiFastboot->FlashIo,\
    PartStart,BufferSectors,SunxiFastboot->BaseReceiveBuffer);
    if(Status)
    {
      FASTBOOT_ERROR("sunxi fastboot erase FAIL: failed to erase partition %a \n", PartitionName);
      FastbootSendResponse("FAILerase: failed to erase partition");
      return ;
    }
    PartStart += BufferSectors;
    UnerasedSectors -= BufferSectors;
  }
  if(UnerasedSectors)
  {
    Status = SunxiFastboot->FlashIo->SunxiFlashIoWrite(SunxiFastboot->FlashIo,\
    PartStart,UnerasedSectors,SunxiFastboot->BaseReceiveBuffer);
    if(Status)
    {
      FASTBOOT_ERROR("sunxi fastboot erase FAIL: failed to erase partition %a \n", PartitionName);
      FastbootSendResponse("FAILerase: failed to erase partition");
      return ;
    }
  }

  FASTBOOT_INFO("Flashing done\n");
  FastbootSendResponse("OKAY");
}


STATIC
VOID
AcceptCmd (
  IN        UINTN  Size,
  IN  CONST CHAR8 *Data
  )
{
  CHAR8       Command[FASTBOOT_COMMAND_MAX_LENGTH + 1];

  FASTBOOT_DEBUG_INFO("%d:Data=%p,size=%d\n",__LINE__,Data,Size);
  // Max command size is 64 bytes
  if (Size > FASTBOOT_COMMAND_MAX_LENGTH) {
    FastbootSendResponse ("FAILCommand too large");
    return;
  }

  // Commands aren't null-terminated. Let's get a null-terminated version.
  // AsciiStrnCpy (Command, Data, Size);
  CopyMem (Command, Data,Size);
  Command[Size] = '\0';
  
  FASTBOOT_DEBUG_INFO("cmd =%a\n",Command);
    
  // Parse command
  if (MATCH_CMD_LITERAL ("getvar", Command)) {
    HandleGetVar (Command + sizeof ("getvar"));
  } else if (MATCH_CMD_LITERAL ("download", Command)) {
    HandleDownload (Command + sizeof ("download"));
  } else if (MATCH_CMD_LITERAL ("verify", Command)) {
    FastbootSendResponse ("FAILNot supported");
  } else if (MATCH_CMD_LITERAL ("flash", Command)) {
    HandleFlash (Command + sizeof ("flash"));
  } else if (MATCH_CMD_LITERAL ("erase", Command)) {
    HandleErase (Command + sizeof ("erase"));
  } else if (MATCH_CMD_LITERAL ("boot", Command)) {
  //  HandleBoot ();
  } else if (MATCH_CMD_LITERAL ("continue", Command)) {
    FastbootSendResponse ("OKAY");
  } else if (MATCH_CMD_LITERAL ("reboot", Command)) {
    if (MATCH_CMD_LITERAL ("reboot-booloader", Command)) {
      // fastboot_protocol.txt:
      //    "reboot-bootloader    Reboot back into the bootloader."
      // I guess this means reboot back into fastboot mode to save the user
      // having to do whatever they did to get here again.
      // Here we just reboot normally.
     // SEND_LITERAL ("INFOreboot-bootloader not supported, rebooting normally.");
    }
    FastbootSendResponse ("OKAY");
    gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);

    // Shouldn't get here
    DEBUG ((EFI_D_ERROR, "Fastboot: gRT->ResetSystem didn't work\n"));
  } else if (MATCH_CMD_LITERAL ("powerdown", Command)) {
    FastbootSendResponse ("OKAY");
    gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);

    // Shouldn't get here
    DEBUG ((EFI_D_ERROR, "Fastboot: gRT->ResetSystem didn't work\n"));
  } else if (MATCH_CMD_LITERAL ("oem", Command)) {
    /* NULL */
  } else{
    FastbootSendResponse ("FAILCommand not recognised. Check Fastboot version.");
  }
}

EFI_STATUS
FastbootRxHandle(
  SUNXI_FASTBOOT  *This,
  EFI_USBFN_TRANSFER_RESULT* TransferResult
)
{
  UINT16 MaxPacketSize=0;
  UINTN TxSize =0;
  EFI_USBFN_IO_PROTOCOL *UsbIo=This->UsbIo;
  UINTN  BytesTransferred  =TransferResult->BytesTransferred;
  EFI_USBFN_TRANSFER_STATUS TransferStatus =TransferResult->TransferStatus;
  VOID *Buffer =TransferResult->Buffer;
  
  if(TransferResult->EndpointIndex!=FASTBOOT_BULK_ENDPOINT_INDEX){
    return EFI_SUCCESS;
  }
  if(TransferStatus!=UsbTransferStatusComplete){
    FASTBOOT_DEBUG_INFO("%a:Transfer has not completed yet\n");
    return EFI_SUCCESS;
  }
  FASTBOOT_DEBUG_INFO("********%a:TransferStatus: %d********\n",__FUNCTION__,TransferResult->TransferStatus);
  FASTBOOT_DEBUG_INFO("********%a:EndpointIndex: %d********\n",__FUNCTION__,TransferResult->EndpointIndex);
  FASTBOOT_DEBUG_INFO("********%a:Direction: %a********\n",__FUNCTION__,TransferResult->Direction==EfiUsbEndpointDirectionDeviceTx?"DeviceTx":"DeviceRx");
  FASTBOOT_DEBUG_INFO("********%a:Buffer: 0x%p********\n",__FUNCTION__,TransferResult->Buffer);
  FASTBOOT_DEBUG_INFO("********%a:BytesTransferred: %d********\n",__FUNCTION__,TransferResult->BytesTransferred);
  switch(This->State){
    case SunxiUsbFastbootSetup:
      FASTBOOT_DEBUG_INFO("SUNXI_USB_FASTBOOT_SETUP\n");

      AcceptCmd(BytesTransferred,Buffer);
      break;

    case SunxiUsbFastbootSendData:  
    case SunxiUsbFastbootReceiveData: 
      CopyMem(This->BaseReceiveBuffer+(This->ActuallyReceivedBytes),\
      TransferResult->Buffer,TransferResult->BytesTransferred);
       
      This->ActuallyReceivedBytes += TransferResult->BytesTransferred;
      if(This->ActuallyReceivedBytes>=This->TryToReceiveBytes){
        FASTBOOT_INFO("fastboot Data transfer finish\n");
        FastbootSendResponse("OKAY");
        This->NextState = SunxiUsbFastbootSetup;
        break;
      }
       
      UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
                UsbBusSpeedHigh,&MaxPacketSize);
      TxSize = MaxPacketSize;
      UsbIo->Transfer(UsbIo,FASTBOOT_BULK_ENDPOINT_INDEX,\
      EfiUsbEndpointDirectionDeviceRx,&TxSize,(VOID*)This->DataSetUpBuffer);
      break;
    default:
      FASTBOOT_ERROR("State Machine Error!\n");
  }
    
  return EFI_SUCCESS;

}

 EFI_STATUS
FastbootTxHandle(
  SUNXI_FASTBOOT  *This,
  EFI_USBFN_TRANSFER_RESULT* TransferResult
 )
{ 

  //UINT16 MaxPacketSize=0;
  //UINTN TxSize =0;
  //EFI_USBFN_IO_PROTOCOL *UsbIo=This->UsbIo;

  if(TransferResult->EndpointIndex!=FASTBOOT_BULK_ENDPOINT_INDEX){
    return EFI_SUCCESS;
  }
  if(TransferResult->TransferStatus!=UsbTransferStatusComplete){
    FASTBOOT_DEBUG_INFO("%a:Transfer has not completed yet\n");
    return EFI_SUCCESS;
  }
  FASTBOOT_DEBUG_INFO("********%a:TransferStatus: %d********\n",__FUNCTION__,TransferResult->TransferStatus);
  FASTBOOT_DEBUG_INFO("********%a:EndpointIndex: %d********\n",__FUNCTION__,TransferResult->EndpointIndex);
  FASTBOOT_DEBUG_INFO("********%a:Direction: %a********\n",__FUNCTION__,TransferResult->Direction==EfiUsbEndpointDirectionDeviceTx?"DeviceTx":"DeviceRx");
  FASTBOOT_DEBUG_INFO("********%a:Buffer: 0x%p********\n",__FUNCTION__,TransferResult->Buffer);
  FASTBOOT_DEBUG_INFO("********%a:BytesTransferred: %d********\n",__FUNCTION__,TransferResult->BytesTransferred);
  switch(This->State){
    case SunxiUsbFastbootSetup:
      break;
    case SunxiUsbFastbootSendData:
      break;
    case SunxiUsbFastbootReceiveData:
      break;
    case SunxiUsbFastbootResponse:
#if 0
       //device configured!, prepare for bulk transfer.
      UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
                UsbBusSpeedHigh,&MaxPacketSize);
      TxSize = MaxPacketSize;
      //set up Bulk receive buffer for the controller.
      UsbIo->Transfer(UsbIo,FASTBOOT_BULK_ENDPOINT_INDEX,\
      EfiUsbEndpointDirectionDeviceRx,&TxSize,(VOID*)This->DataSetUpBuffer);
#endif
      if(This->NextState==SunxiUsbFastbootReceiveData){
        This->ActuallyReceivedBytes = 0; 
      }
      //response done, enter the next state.
      This->State  =This->NextState;
      if(This->NextState == SunxiUsbFastbootReceiveData){
        FASTBOOT_INFO("fastboot begin partition data transfer\n");
      }

      break;
    default:
      FASTBOOT_ERROR("State Machine Error!\n");
  }
    
  return EFI_SUCCESS;

}
 
EFI_STATUS
FastbootUsbIoStateLoop (
  SUNXI_FASTBOOT        *This
  )
{
  
  EFI_USBFN_MESSAGE Message;
  UINTN PayloadSize=0;
  EFI_USBFN_MESSAGE_PAYLOAD Payload;
  
  This->UsbIo->EventHandler(This->UsbIo,&Message,&PayloadSize,&Payload);

  switch(Message){  
  case EfiUsbMsgSetupPacket:
    DEBUG (( EFI_D_INFO,"\n\n"));
    FASTBOOT_DEBUG_INFO("Message:EfiUsbMsgSetupPacket\n");
    FastbootUsbIoSetUpHandle(This,&(Payload.udr));

    break;
      
  case EfiUsbMsgEndpointStatusChangedRx:
    DEBUG (( EFI_D_INFO,"\n\n"));
    FASTBOOT_DEBUG_INFO("Message:EfiUsbMsgEndpointStatusChangedRx\n");
    FastbootRxHandle(This,&(Payload.utr));     
    break;
      
  case EfiUsbMsgEndpointStatusChangedTx:
    DEBUG (( EFI_D_INFO,"\n\n"));
    FASTBOOT_DEBUG_INFO("Message:EfiUsbMsgEndpointStatusChangedTx\n");
    FastbootTxHandle(This,&(Payload.utr)); 
    break;
      
  case EfiUsbMsgBusEventDetach:
    DEBUG (( EFI_D_INFO,"\n\n"));
    FASTBOOT_DEBUG_INFO("Message:EfiUsbMsgBusEventDetach\n\n");

    break;
      
  case EfiUsbMsgBusEventAttach:
    DEBUG (( EFI_D_INFO,"\n\n"));
    FASTBOOT_DEBUG_INFO("Message:EfiUsbMsgBusEventAttach\n\n");

    break;
      
  case EfiUsbMsgBusEventReset:
    /*reset the SW state machine here.*/
    This->State = SunxiUsbFastbootSetup;
     DEBUG (( EFI_D_INFO,"\n\n"));
    FASTBOOT_DEBUG_INFO("Message:EfiUsbMsgBusEventReset\n\n");

    break;
      
  case EfiUsbMsgBusEventSuspend:
    DEBUG (( EFI_D_INFO,"\n\n"));
    FASTBOOT_DEBUG_INFO("Message:EfiUsbMsgBusEventSuspend\n\n");

    break;
      
  case EfiUsbMsgBusEventResume:
    DEBUG (( EFI_D_INFO,"\n\n"));
    FASTBOOT_DEBUG_INFO("Message:EfiUsbMsgBusEventResume\n\n");

    break;
      
  case EfiUsbMsgBusEventSpeed:
    DEBUG (( EFI_D_INFO,"\n\n"));
    FASTBOOT_DEBUG_INFO("Message:EfiUsbMsgBusEventSpeed\n\n");

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
FastbootMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )

{

  EFI_STATUS Status = EFI_SUCCESS;
  
  EFI_INPUT_KEY Key;
  
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *TextIn;

  SunxiFastboot = AllocatePool(sizeof(SUNXI_FASTBOOT));
  if(!SunxiFastboot){
    Status = EFI_OUT_OF_RESOURCES;  
    goto FastbootMainOut;
  }

  Status = gBS->LocateProtocol (&gEfiSimpleTextInProtocolGuid, NULL, (VOID **) &TextIn);
  if (EFI_ERROR (Status)) {
    FASTBOOT_ERROR("Couldn't open Text Input Protocol: %r\n", Status);
    goto FastbootMainOut;
  }


  Status = FastbootUsbIoInitialize(SunxiFastboot,100,100);
  if(Status){
    FASTBOOT_ERROR("FastbootUsbIoInitialize error\n");
  }
 
  // Disable watchdog
  Status = gBS->SetWatchdogTimer (0, 0x10000, 0, NULL);
  if (EFI_ERROR (Status)) {
    FASTBOOT_ERROR("Fastboot: Couldn't disable watchdog timer: %r\n", Status);
  }

  SunxiFastboot->State = SunxiUsbFastbootSetup;
  SunxiFastboot->NextState = SunxiUsbFastbootSetup;
     // Talk to the user
  FASTBOOT_INFO("Allwinner android Fastboot - version " ALLWINNER_FASTBOOT_VERSION ". Press Q to quit.\r\n");
  while(1){
    FastbootUsbIoStateLoop(SunxiFastboot);

    Status = TextIn->ReadKeyStroke(TextIn,&Key);
    if(Status==EFI_SUCCESS){
      if (Key.UnicodeChar == 113) {
        break;
      }
    }
  }

  Status = FastbootUsbIoExit(SunxiFastboot);
  
FastbootMainOut:
  if(SunxiFastboot){
    FreePool(SunxiFastboot);
  }

  return Status;
  
}

  

 
