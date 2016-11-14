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
#include <Library/SysConfigLib.h>
#include <Library/SunxiPartitionLib.h>
#include <Library/SunxiMbr.h>

#include <Library/SunxiBootInfoLib.h>
#include <Library/SunxiSpriteHelperLib.h>

#include <Library/SunxiCommonLib.h>
#include <Library/SunxiCheckLib.h>
#include <Library/SunxiFileLib.h>

#include <IndustryStandard/Usb.h>

#include <Protocol/EfiUsbFnIo.h>
#include <Protocol/SunxiFlashIo.h>
#include <Protocol/AxpPower.h>
#include <Protocol/LoadedImage.h>

#include <Sunxi_type/Sunxi_type.h>
#include <Interinc/sunxi_uefi.h>
#include <BootAutoTest.h>
#include <EfexBufferQueue.h>

#pragma pack()

#define readl(addr) (*(volatile u32 *) (addr))
#define writel(val, addr) ((*(volatile u32 *) (addr)) = (val))
static   PMU_CONFIG  PmuConfig;
static   MULTI_UNSEQ_MEM GlobalUnseqMemAddr;
boot_logo_info  logo_info ;
CHAR8 *logo_buf = NULL;


#define DEVICE_VENDOR_ID      0x1F3A
#define DEVICE_PRODUCT_ID     0xEfE8


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
  0xffff,                                        //BcdDevice
  0,                         //StrManufacturer
  0,                           //StrProduct
  0,                         //StrSerialNumber
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
    0,                            //Configuration;
    0x80,                                             //Attributes;
    0xFA                                              //MaxPower;
};

EFI_USB_INTERFACE_DESCRIPTOR  mInterfaceDescriptor = {
    sizeof (USB_INTERFACE_DESCRIPTOR), //Length;
    USB_DESC_TYPE_INTERFACE, //DescriptorType;
    0,                                                //InterfaceNumber;
    0,                                                //AlternateSetting;
    2,                                                //NumEndpoints;
    0xFF,                                             //InterfaceClass;
    0xff,                                             //InterfaceSubClass;
    0xff,                                             //InterfaceProtocol;
    0                             //Interface;
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


STATIC SUNXI_EFEX *SunxiEfex = NULL;

#define printf(ARG...)        AsciiPrint(ARG)
#define malloc(x)             AllocateZeroPool((x))
#define free(x)               FreePool((x))

INT32 sunxi_probe_axp_each_volt(PMU_VOLT_t *pmu_para)
{
  INT32  ret;//onoff;
  UINT32 power_supply_hd;
  CHAR8 pmu_name[16];
  INT32 i = 0;
  INT32  power_vol, power_index = 0;
  PMU_VOLT_t *pmu_temp = NULL;

  EFI_HANDLE                    *HandleBuffer;
  UINTN                         NumberOfHandles;
  UINTN                         Index;
  AXP_POWER_ID                  AxpID;
  AXP_POWER_PROTOCOL           *AxpInstance = NULL;
  EFI_STATUS                    Status;
  
  if(!pmu_para)
  {
    AsciiPrint("pmu_para is NULL \n");
    return -1;
  }
    
  AsciiPrint("start =====\n");
  pmu_temp = pmu_para;

  power_supply_hd = script_parser_fetch_subkey_start("power_sply");
  if(!power_supply_hd)
  {
    AsciiPrint("unable to find power supply\n");
    return -1;
  }
  
  //
  // Locate protocol.
  //
  Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gAxpPowerProtocolGuid,
                 NULL,
                 &NumberOfHandles,
                 &HandleBuffer
                 );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AsciiPrint("AXP NumberOfHandles = %d\n",NumberOfHandles);
  for (Index = 0; Index < NumberOfHandles; Index++) 
  {
    if(Index == 1)
    {
      power_supply_hd = script_parser_fetch_subkey_start("slave_power_sply");
      if(!power_supply_hd)
      {
        printf("unable to find slave power supply\n");
        return 0;
      }
    }

    Status = gBS->HandleProtocol (HandleBuffer[Index],
                         &gAxpPowerProtocolGuid,
                         (VOID**) &AxpInstance);

    AxpID = AxpInstance->AxpPowerId;
    switch(AxpID)
    {
      case AXP_POWER_ID_AXP809:
        AsciiStrCpy(pmu_name,"axp809");
        break;
      case AXP_POWER_ID_AXP806:
        AsciiStrCpy(pmu_name,"axp806");
        break;
      case AXP_POWER_ID_AXP221:
        AsciiStrCpy(pmu_name,"axp22x");
        break;
      default:
        AsciiStrCpy(pmu_name,"axperror");
        break;         
    }
    AsciiPrint("current pmu_name == %s == \n",pmu_name);
    do
    {
      SetMem(pmu_temp->volt[i].pmu_type, 16, 0);
      AsciiStrCpy((void *)pmu_temp->volt[i].pmu_type,pmu_name);
      SetMem(pmu_temp->volt[i].vol_name, 16, 0);
      ret = script_parser_fetch_subkey_next(power_supply_hd, pmu_temp->volt[i].vol_name, &power_vol, &power_index);
      if(ret < 0)
      {
        AsciiPrint("find power_sply to end\n");
        pmu_temp->voltCnt = i;
        i++;
        break;
      }

      Status = AxpInstance->ProbeSupplyStatusByName(pmu_temp->volt[i].vol_name,&(pmu_temp->volt[i].voltage));
      if(EFI_ERROR(Status))
      {
        printf("axp probe %s failed\n", pmu_temp->volt[i].vol_name);
        return -1;
      }
      AsciiPrint("%d \n",pmu_temp->volt[i].voltage);
      i++;
    }while(1);
  }
  gBS->FreePool(HandleBuffer);
  AsciiPrint("end====\n");
  return 0;
}

STATIC EFI_STATUS EfexUsbIoInitialize(
  IN OUT SUNXI_EFEX*          This,
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
  
  UINT16 Pid,Vid;
  UINT16 MaxPacketSize;
  
  Status = gBS->LocateProtocol (&gSunxiFlashIoProtocolGuid, NULL, (VOID **) &This->FlashIo);
  if (EFI_ERROR (Status)) {
    EFEX_ERROR("Couldn't open Sunxi Flash Io Protocol: %r\n", Status);
    return Status;
  }
    
  Status = gBS->LocateProtocol(&gEfiUsbFnIoProtocolGuid, NULL, (VOID **)&This->UsbIo);
  ASSERT_EFI_ERROR(Status);

  UsbIo  = This->UsbIo;

  Status = UsbIo->AllocateTransferBuffer(UsbIo,EFEX_CONTROL_SET_UP_BUFFER_SIZE,(VOID**)&This->Transfer.ControlSetUpBuffer);
  if(Status){
    EFEX_ERROR("Allocate ControlSetupBuffer failed\n"); 
    return Status;
  }

  Status = UsbIo->AllocateTransferBuffer(UsbIo,EFEX_DATA_SET_UP_BUFFER_SIZE,(VOID**)&This->Transfer.DataSetUpBuffer);
  if(Status){
    EFEX_ERROR("Allocate DataSetupBuffer failed\n");  
    goto ErrorOut3;
  }

  Status = UsbIo->AllocateTransferBuffer(UsbIo,EFEX_DATA_SEND_BUFFER_SIZE,(VOID**)&This->Transfer.BaseSendBuffer);
  if(Status){
    EFEX_ERROR("Allocate BaseSendBuffer failed\n"); 
    goto ErrorOut2;
  }
  
  EFEX_DEBUG_INFO("BaseSendBuffer = 0x%x\n",This->Transfer.BaseSendBuffer);

  Status = UsbIo->AllocateTransferBuffer(UsbIo,EFEX_DATA_RECEIVE_BUFFER_SIZE,(VOID**)&This->Transfer.BaseReceiveBuffer);
  if(Status){
    EFEX_ERROR("Allocate BaseReceiveBuffer failed\n");  
    goto ErrorOut1;
  }
  
  EFEX_DEBUG_INFO("BaseReceiveBuffer = 0x%x\n",This->Transfer.BaseReceiveBuffer);

  EFEX_DEBUG_INFO("Start Controller\n");
  Status = UsbIo->StartController(UsbIo);
  if(Status){
    EFEX_DEBUG_INFO("StartController failed\n");  
    goto ErrorOut0;
  }
  DeviceInfo = AllocateZeroPool(sizeof(EFI_USB_DEVICE_INFO));
  if(!DeviceInfo){
    EFEX_ERROR("AllocateZeroPool DeviceInfo failed\n"); 
    Status = EFI_DEVICE_ERROR;
    goto ErrorOut0;
  }

  ConfigInfoTable = AllocateZeroPool(sizeof(EFI_USB_CONFIG_INFO));
  if(!ConfigInfoTable){
    EFEX_ERROR("AllocateZeroPool ConfigInfoTable failed\n");  
    Status = EFI_DEVICE_ERROR;
    goto Out;
  }
  
  InterfaceInfo = AllocateZeroPool(sizeof(EFI_USB_INTERFACE_INFO));
  if(!InterfaceInfo){
    EFEX_ERROR("AllocateZeroPool InterfaceInfo failed\n");  
    Status = EFI_DEVICE_ERROR;
    goto Out;
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
    EFEX_ERROR("Get pid/vid failed!\n");
    goto Out;
  }
  //DeviceInfo->DeviceDescriptor->IdVendor = Vid;
  //DeviceInfo->DeviceDescriptor->IdProduct = Pid;

  //fix the MaxPacketSize in EndPointDescripter
  Status =UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,UsbBusSpeedHigh,&MaxPacketSize);
  if(Status){
    EFEX_ERROR("Get MaxPacketSize failed!\n");
    goto Out;
  }
  mEndpointDescriptor[0].MaxPacketSize = MaxPacketSize;
  mEndpointDescriptor[1].MaxPacketSize = MaxPacketSize;

  //to do:fix  device-specific information 

  //fix the config descriptor size.
  mConfigDescriptor.TotalLength =sizeof(EFI_USB_CONFIG_DESCRIPTOR)+\
  sizeof(EFI_USB_INTERFACE_DESCRIPTOR)*mConfigDescriptor.NumInterfaces+\
  sizeof(EFI_USB_ENDPOINT_DESCRIPTOR)*mInterfaceDescriptor.NumEndpoints;
  Status  =EfexQueueInitialize(This);
  if(Status){
    goto Out;
  }
  //Config and enable the endpoint.
  EFEX_DEBUG_INFO("Configure and enable endpoints\n");  
  Status = UsbIo->ConfigureEnableEndpoints(UsbIo,DeviceInfo);
  if(Status){
    EFEX_ERROR("enable endpoint failed!\n");
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

ErrorOut0:
  UsbIo->FreeTransferBuffer(UsbIo,(VOID*)This->Transfer.BaseReceiveBuffer);
ErrorOut1:
  UsbIo->FreeTransferBuffer(UsbIo,(VOID*)This->Transfer.BaseSendBuffer);
ErrorOut2:
  UsbIo->FreeTransferBuffer(UsbIo,(VOID*)This->Transfer.DataSetUpBuffer);
ErrorOut3:
  UsbIo->FreeTransferBuffer(UsbIo,(VOID*)This->Transfer.ControlSetUpBuffer);
  
  return Status;
  
}

STATIC EFI_STATUS EfexUsbIoExit(
  SUNXI_EFEX *This
 )
{

  EFI_STATUS      Status = EFI_SUCCESS;
  Status = This->UsbIo->StopController(This->UsbIo);
  if(Status){
    EFEX_ERROR("Stop controller failed!\n");
  }
  
  EfexQueueExit(This);
  
  This->UsbIo->FreeTransferBuffer(This->UsbIo,(VOID*)This->Transfer.ControlSetUpBuffer);
  This->UsbIo->FreeTransferBuffer(This->UsbIo,(VOID*)This->Transfer.DataSetUpBuffer);
  This->UsbIo->FreeTransferBuffer(This->UsbIo,(VOID*)This->Transfer.BaseSendBuffer);
  This->UsbIo->FreeTransferBuffer(This->UsbIo,(VOID*)This->Transfer.BaseReceiveBuffer);
  return Status;
}


EFI_STATUS
EfexUsbIoSetUpHandle(
  SUNXI_EFEX  *This,
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
          EFEX_DEBUG_INFO("standard_req:USB_REQ_GET_STATUS\n");
        }

        break;
      }
      case USB_REQ_CLEAR_FEATURE:   //   0x01
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          EFEX_DEBUG_INFO("standard_req:USB_REQ_CLEAR_FEATURE\n");
        }

        break;
      }
      case USB_REQ_SET_FEATURE:   //   0x03
      {
        /* host-to-device */
        if(USB_DIR_OUT == (Req->RequestType & USB_REQ_DIRECTION_MASK))
        {
          EFEX_DEBUG_INFO("standard_req:USB_REQ_SET_FEATURE\n");
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
            EFEX_DEBUG_INFO("standard_req:USB_REQ_SET_ADDRESS\n");
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
            EFEX_DEBUG_INFO("standard_req:USB_REQ_GET_DESCRIPTOR\n");
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
            EFEX_DEBUG_INFO("standard_req:USB_REQ_SET_DESCRIPTOR\n");            
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
            EFEX_DEBUG_INFO("standard_req:USB_REQ_GET_CONFIG\n");
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
             EFEX_DEBUG_INFO("standard_req:USB_REQ_SET_CONFIG\n");

             //device configured!, prepare for bulk transfer.
             UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
                        UsbBusSpeedHigh,&MaxPacketSize);
             TxSize = MaxPacketSize;
             //set up Bulk receive buffer for the controller.
             UsbIo->Transfer(UsbIo,EFEX_BULK_ENDPOINT_INDEX,\
             EfiUsbEndpointDirectionDeviceRx,&TxSize,(VOID*)This->Transfer.DataSetUpBuffer);
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
            EFEX_DEBUG_INFO("standard_req:USB_REQ_GET_INTERFACE\n");
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
            EFEX_DEBUG_INFO("standard_req:USB_REQ_SET_INTERFACE\n");
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
            EFEX_DEBUG_INFO("standard_req:USB_REQ_SYNCH_FRAME\n");
          }
        }

        break;
      }

      default:
      {
        EFEX_DEBUG_INFO("ep0 control unsupported request:Request=0x%x\n",Req->Request);
        break;
      }
    }
  }
  else
  {
    switch(Req->RequestType){
      case 161:
        if(Req->Request == 0xFE)
        {
          EFEX_DEBUG_INFO("efex ask for max lun\n");

          This->Transfer.ControlSetUpBuffer[0] = 0;
          TxSize =1;
          UsbIo->Transfer(UsbIo,EFEX_CONTROL_ENDPOINT_INDEX,EfiUsbEndpointDirectionDeviceTx,\
                  &TxSize,(VOID*)This->Transfer.ControlSetUpBuffer);
        }
        else
        {
          EFEX_ERROR(" unknown ep0 req in efex\n");
        }
        break;

      default:
        EFEX_ERROR("unknown non standard ep0 req\n");
        break;
    }
    EFEX_DEBUG_INFO("ep0 control non-standard request:Request=0x%x\n",Req->Request);
  }
  return Status;
}


static EFI_STATUS
SunxiEfexUsbOpCmd(IN OUT SUNXI_EFEX *This,VOID* CmdBuffer)
{
  GLOBAL_CMD  *Cmd = (GLOBAL_CMD*)CmdBuffer;

  EFEX_TRANSFER *EfexTransfer = &This->Transfer;
  SUNXI_FLASH_IO_PROTOCOL *FlashIo = This->FlashIo;
  
  switch(Cmd->AppCmd)
  {
    case APP_LAYER_COMMEN_CMD_VERIFY_DEV:
      EFEX_DEBUG_INFO("APP_LAYER_COMMEN_CMD_VERIFY_DEV\n");
      {
        VERIFY_DEV_DATA *AppVerifyDev;

        AppVerifyDev = (VERIFY_DEV_DATA *)EfexTransfer->BaseSendBuffer;

        CopyMem(AppVerifyDev->Tag, AL_VERIFY_DEV_TAG_DATA, sizeof(AL_VERIFY_DEV_TAG_DATA));
        AppVerifyDev->PlatformHwId    = FES_PLATFORM_HW_ID;
        AppVerifyDev->PlatformFwId    = 0x0001;
        AppVerifyDev->Mode        = AL_VERIFY_DEV_MODE_SRV;//
        AppVerifyDev->PhoenixDataFlag   = 'D';
        AppVerifyDev->PhoenixDataLengh  = PHOENIX_PRIV_DATA_LEN_NR;
        AppVerifyDev->PhoenixDataStartAddress= PHOENIX_PRIV_DATA_ADDR;

        EfexTransfer->ActualSendBuffer  = EfexTransfer->BaseSendBuffer;
        EfexTransfer->SizeNeedToBeSent  = sizeof(VERIFY_DEV_DATA);
        EfexTransfer->LastError         = 0;
        EfexTransfer->AppNextState      = SunxiUsbEfexAppSendData;
      }

      break;

    case APP_LAYER_COMMEN_CMD_SWITCH_ROLE:
      EFEX_DEBUG_INFO("APP_LAYER_COMMEN_CMD_SWITCH_ROLE\n");
      EFEX_DEBUG_INFO("not supported\n");

      EfexTransfer->LastError          = -1;
      EfexTransfer->AppNextState   = SunxiUsbEfexAppStatus;

      break;

    case APP_LAYER_COMMEN_CMD_IS_READY:
      EFEX_DEBUG_INFO("APP_LAYER_COMMEN_CMD_IS_READY\n");
      {
        IS_READY_DATA *AppIsReadyData;

        AppIsReadyData = (IS_READY_DATA*)EfexTransfer->BaseSendBuffer;

        AppIsReadyData->IntervalMs= 500;
        AppIsReadyData->State = AL_IS_READY_STATE_READY;

        EfexTransfer->ActualSendBuffer   = EfexTransfer->BaseSendBuffer;
        EfexTransfer->SizeNeedToBeSent           = sizeof(IS_READY_DATA);
        EfexTransfer->LastError          = 0;
        EfexTransfer->AppNextState       = SunxiUsbEfexAppSendData;
      }

      break;

    case APP_LAYER_COMMEN_CMD_GET_CMD_SET_VER:
      EFEX_DEBUG_INFO("APP_LAYER_COMMEN_CMD_GET_CMD_SET_VER\n");
      {
        GET_CMD_SET_VER_DATA *AppGetCmdVerData;

        AppGetCmdVerData = (GET_CMD_SET_VER_DATA*)EfexTransfer->BaseSendBuffer;

        AppGetCmdVerData->VersionHigh = 1;
        AppGetCmdVerData->VersionLow  = 0;

        EfexTransfer->ActualSendBuffer   = EfexTransfer->BaseSendBuffer;
        EfexTransfer->SizeNeedToBeSent           = sizeof(GET_CMD_SET_VER_DATA);
        EfexTransfer->LastError          = 0;
        EfexTransfer->AppNextState     = SunxiUsbEfexAppSendData;
      }

      break;

    case APP_LAYER_COMMEN_CMD_DISCONNECT:
      EFEX_DEBUG_INFO("APP_LAYER_COMMEN_CMD_DISCONNECT\n");
      EFEX_DEBUG_INFO("not supported\n");

      EfexTransfer->LastError          = -1;
      EfexTransfer->AppNextState   = SunxiUsbEfexAppStatus;

      break;

    case FEX_CMD_fes_trans:
      EFEX_DEBUG_INFO("FEX_CMD_fes_trans\n");

      //需要发送数据
      {
        FES_TRANS_OLD  *FesOldData = (FES_TRANS_OLD *)CmdBuffer;

        if(FesOldData->Length)
        {
          if(FesOldData->U2.DOU == 2)   //upload data
          {

            //UINT32 value;

            //value = *(UINT32 *)FesOldData->Address;

            //EFEX_DEBUG_INFO("send id 0x%x, Address 0x%x, Length 0x%x\n", value, FesOldData->Address, FesOldData->Length);
            EfexTransfer->ActualSendBuffer   = (UINT8*)FesOldData->Address; //set send addr
            EfexTransfer->SizeNeedToBeSent           = FesOldData->Length;  //set send len
            EfexTransfer->LastError          = 0;
            EfexTransfer->AppNextState   = SunxiUsbEfexAppSendData;
          }
          else  //(FesOldData->u2.DOU == (0 or 1))  //download data
          {

            //UINT32 value;

            //value = *(UINT32 *)FesOldData->Address;

            //EFEX_DEBUG_INFO("receive id 0x%x, Address 0x%x, length 0x%x\n", value, FesOldData->Address, FesOldData->Length);

            EfexTransfer->DataType        = SUNXI_EFEX_DRAM_TAG;  //data to dram
            EfexTransfer->ActualReceiveBuffer   = (UINT8*)FesOldData->Address;  //set recv addr
            EfexTransfer->TryToReceiveBytes     = FesOldData->Length; //set recv len
            EfexTransfer->LastError          = 0;
            EfexTransfer->AppNextState   = SunxiUsbEfexAppReceiveData;
          }
        }
        else
        {
          EFEX_ERROR("FEX_CMD_fes_trans: no data need to send or receive\n");
          EfexTransfer->AppNextState = SunxiUsbEfexAppStatus;
        }
      }
      EfexTransfer->LastError = 0;

      break;

    case FEX_CMD_fes_run:
      EFEX_DEBUG_INFO("FEX_CMD_fes_run\n");
      {
#ifdef  CONFIG_CMD_ELF

#else
        FES_RUN       *FesRun = (FES_RUN *)CmdBuffer;
        INT32         *AppRet;
        EFI_STATUS    Status;
        EFI_HANDLE    ImageHandle;
        EFI_LOADED_IMAGE_PROTOCOL*   LoadedImage;
        AppRet = (int *)EfexTransfer->BaseSendBuffer;
        *AppRet = -1;
                
        if(0x5a4d == *(UINT32*)(FesRun->CodeAddress))
        {
          Status = gBS->LoadImage (TRUE, gImageHandle, NULL, (VOID*)(FesRun->CodeAddress), FesRun->CodeSize, &ImageHandle);
          if (!EFI_ERROR(Status)) 
          {
            if(FesRun->ParaAddr != NULL)
            {
              Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
              if (!EFI_ERROR (Status)) 
              {
                 LoadedImage->LoadOptionsSize  = sizeof(FesRun->ParaAddr);
                *((INT32*)LoadedImage->LoadOptions) = (INT32)(FesRun->ParaAddr);
              }
            }
            Status = gBS->StartImage (ImageHandle, NULL, NULL);
            *AppRet = EFI_ERROR(Status) ? -1:0;
          }
          else
          {
            EFEX_ERROR("Load UEFI Application from 0x%x fail\n",FesRun->CodeAddress);
          }
        }
        else
        {
          EFEX_ERROR("This is not a UEFI Application\n");
          *AppRet = -1;
        }

        EfexTransfer->ActualSendBuffer = EfexTransfer->BaseSendBuffer;
        EfexTransfer->SizeNeedToBeSent         = 4;
        EfexTransfer->LastError        = 0;
        EfexTransfer->AppNextState     = SunxiUsbEfexAppSendData;
#endif
      }

      break;

    case FEX_CMD_fes_down:
      EFEX_DEBUG_INFO("FEX_CMD_fes_down\n");
      {
        FES_TRANS  *Trans = (FES_TRANS *)CmdBuffer;

        EfexTransfer->DataType  = Trans->Type;                   //data type MBR,BOOT1,BOOT0...and partition type
        if((Trans->Type & SUNXI_EFEX_DRAM_MASK) == SUNXI_EFEX_DRAM_MASK) //
        {
          if((SUNXI_EFEX_DRAM_MASK | SUNXI_EFEX_TRANS_FINISH_TAG) == Trans->Type)
          {
            EfexTransfer->ActualReceiveBuffer = EfexTransfer->BaseReceiveBuffer;
            EfexTransfer->DramTransferBuffer = Trans->Address;
            //printf("dram write: start 0x%x: length 0x%x\n", trans->Address, trans->len);
          }
          else
          {
            //set recv addr
            EfexTransfer->ActualReceiveBuffer   = EfexTransfer->BaseReceiveBuffer + EfexTransfer->ActuallyReceivedBytes;
          }
          //set recv len,in bytes
          EfexTransfer->TryToReceiveBytes         = Trans->Length;
          EfexTransfer->ActuallyReceivedBytes    += Trans->Length;
          EFEX_DEBUG_INFO("%d:EfexTransfer->TryToReceiveBytes=%ld\n",__LINE__,EfexTransfer->TryToReceiveBytes);
          EFEX_DEBUG_INFO("%d:ActuallyReceivedBytes=%ld\n",__LINE__,EfexTransfer->ActuallyReceivedBytes);
          EFEX_DEBUG_INFO("down dram: start 0x%x, sectors 0x%x\n", EfexTransfer->FlashStart, EfexTransfer->FlashSectors);
        }
        else
        {
          //data to flash
          EfexTransfer->ActualReceiveBuffer   = (EfexTransfer->BaseReceiveBuffer + EFEX_DATA_RECEIVE_BUFFER_SIZE/2);
          EfexTransfer->TryToReceiveBytes         = Trans->Length;

          EfexTransfer->FlashStart       = Trans->Address;
          EfexTransfer->FlashSectors     = (Trans->Length + 511) >> 9;
          EFEX_DEBUG_INFO("down flash: start 0x%x, sectors 0x%x\n", EfexTransfer->FlashStart, EfexTransfer->FlashSectors);
        }
        EfexTransfer->LastError          = 0;
        EfexTransfer->AppNextState   = SunxiUsbEfexAppReceiveData;
      }

      break;
    case FEX_CMD_fes_up:
      EFEX_DEBUG_INFO("FEX_CMD_fes_up\n");
      {
        FES_TRANS  *Trans = (FES_TRANS *)CmdBuffer;

        EfexTransfer->LastError            = 0;
        EfexTransfer->AppNextState     = SunxiUsbEfexAppSendData;
        EfexTransfer->DataType  = Trans->Type;                  
        if((Trans->Type & SUNXI_EFEX_DRAM_MASK) == SUNXI_EFEX_DRAM_MASK) 
        {
#if 0
          if((SUNXI_EFEX_DRAM_MASK | SUNXI_EFEX_TRANS_FINISH_TAG) == trans->type)
          {
              EfexTransfer->ActualSendBuffer = EfexTransfer->BaseSendBuffer;
            EfexTransfer->dram_trans_buffer = trans->Address;
            EFEX_DEBUG_INFO("dram write: start 0x%x: length 0x%x\n", trans->Address, trans->len);
          }
          else
          {
            EfexTransfer->ActualSendBuffer   = EfexTransfer->BaseSendBuffer + EfexTransfer->SizeNeedToBeSent;  //设置发送数据地址
          }
#endif

          EfexTransfer->ActualSendBuffer   = (UINT8*)Trans->Address;
          EfexTransfer->SizeNeedToBeSent         = Trans->Length;
          EFEX_DEBUG_INFO("dram read: start 0x%x: length 0x%x\n", Trans->Address, Trans->Length);
        }
        else if((Trans->Type & FEX_CMD_BOOT2_GET_VOLTAGE) == FEX_CMD_BOOT2_GET_VOLTAGE )
        {
          AsciiPrint("PC get axp_each_volt form machine\n");
          PMU_VOLT_t *pmu_para = NULL;
          pmu_para = (PMU_VOLT_t *)EfexTransfer->BaseSendBuffer;
          EfexTransfer->ActualSendBuffer= EfexTransfer->BaseSendBuffer;
          EfexTransfer->SizeNeedToBeSent= sizeof(PMU_VOLT_t);
          EfexTransfer->LastError = sunxi_probe_axp_each_volt(pmu_para);
        }
        else if((Trans->Type & FEX_CMD_BOOT2_GET_FREQ) == FEX_CMD_BOOT2_GET_FREQ)
        {
          
          boot_clock *cpu_pll = NULL;
          cpu_pll = (boot_clock *)EfexTransfer->BaseSendBuffer;
          EfexTransfer->ActualSendBuffer   = EfexTransfer->BaseSendBuffer;
          EfexTransfer->SizeNeedToBeSent         = sizeof(boot_clock);
          cpu_pll->cpu_id = 0;
          cpu_pll->gate  = 1;
          cpu_pll->freqency = SunxiGetCoreClock();
          AsciiPrint("PC get cpu freqency %d\n", cpu_pll->freqency );
        }
        else if((Trans->Type & FEX_CMD_BOOT2_GET_CURRENT_LIMIT) == FEX_CMD_BOOT2_GET_CURRENT_LIMIT)
        {
          UINT32 *charger_current = NULL;
          AXP_POWER_PROTOCOL  *AxpPowerProtocol = NULL;
          EFI_STATUS Status;
          charger_current = (UINT32 *)EfexTransfer->BaseSendBuffer;

          Status = gBS->LocateProtocol (&gAxpPowerProtocolGuid, NULL, (VOID **)&AxpPowerProtocol);
          if(EFI_ERROR(Status))
          {
            EfexTransfer->LastError = -1;
          }
          else
          {  
            EfexTransfer->LastError = AxpPowerProtocol->ProbeVbusCurrentLimit(charger_current);
          }
          AsciiPrint("========  current limit is %d =======\n",*charger_current);
          EfexTransfer->ActualSendBuffer   = EfexTransfer->BaseSendBuffer;
          EfexTransfer->SizeNeedToBeSent = sizeof(UINT32);
        }
        else if((Trans->Type & FEX_CMD_BOOT2_GET_LOGO_INFO) ==FEX_CMD_BOOT2_GET_LOGO_INFO)
        {         
          EFI_STATUS Status;
          
          Status = ReadFileFromAwFs(L"bootlogo.bmp",(VOID**)&logo_buf,&logo_info.boot_logo_size);
                   
          if(EFI_ERROR(Status) || logo_info.boot_logo_size == 0)
          {
            AsciiPrint("read file fail \n");
            EfexTransfer->LastError = -1;
          }
          else
          {
            AsciiPrint("send logo info %d bytes\n",logo_info.boot_logo_size);
            //printf("logo buf addr  %x \n",logo_info.boot_logo_addr);
            logo_info.boot_logo_addr = (UINT32)logo_buf;
            AsciiPrint("logo buf addr  %x \n",logo_info.boot_logo_addr);
            logo_info.check_sum = SunxiCrc32((void*)logo_buf,logo_info.boot_logo_size);
            AsciiPrint("check_sum   %x \n",logo_info.check_sum);
                        
            EfexTransfer->ActualSendBuffer = (UINT8*)(&logo_info);
                        EfexTransfer->SizeNeedToBeSent = sizeof(boot_logo_info);
            EfexTransfer->LastError = 0;
          }
       
        }
        else
        {
          EfexTransfer->ActualSendBuffer   = EfexTransfer->BaseSendBuffer;
          EfexTransfer->SizeNeedToBeSent         = Trans->Length;

          EfexTransfer->FlashStart       = Trans->Address;
          EfexTransfer->FlashSectors     = (Trans->Length + 511) >> 9;

          EFEX_DEBUG_INFO("upload flash: start 0x%x, sectors 0x%x\n", EfexTransfer->FlashStart, EfexTransfer->FlashSectors);
          if(!FlashIo->SunxiFlashIoRead(FlashIo,EfexTransfer->FlashStart, EfexTransfer->FlashSectors, (void *)EfexTransfer->ActualSendBuffer))
          {
            EFEX_ERROR("flash read err: start 0x%x, sectors 0x%x\n", EfexTransfer->FlashStart, EfexTransfer->FlashSectors);
            EfexTransfer->LastError      = -1;
          }
        }
      }

      break;
    case FEX_CMD_fes_verify_value:
      EFEX_DEBUG_INFO("FEX_CMD_fes_verify_value\n");
      {
        FES_CMD_VERIFY_VALUE  *CmdVerify = (FES_CMD_VERIFY_VALUE *)CmdBuffer;
        FES_EFEX_VERIFY     *VerifyData= (FES_EFEX_VERIFY *)EfexTransfer->BaseSendBuffer;

        VerifyData->MediaCrc = SunxiSpritePartRawDataVerify(CmdVerify->Start, CmdVerify->Size);
        VerifyData->Flag     = EFEX_CRC32_VALID_FLAG;

        EFEX_INFO("FEX_CMD_fes_verify_value, start 0x%x, size high 0x%x:low 0x%x\n", CmdVerify->Start, (UINT32)(CmdVerify->Size>>32), (UINT32)(CmdVerify->Size));
        EFEX_INFO("FEX_CMD_fes_verify_value 0x%x\n", VerifyData->MediaCrc);
      }
      EfexTransfer->ActualSendBuffer   = EfexTransfer->BaseSendBuffer;
      EfexTransfer->SizeNeedToBeSent         = sizeof(FES_EFEX_VERIFY);
      EfexTransfer->AppNextState   = SunxiUsbEfexAppSendData;

      break;
    case FEX_CMD_fes_verify_status:
      EFEX_DEBUG_INFO("FEX_CMD_fes_verify_status\n");
      {
        //fes_cmd_verify_status_t *cmd_verify = (fes_cmd_verify_status_t *)cmd_buf;
        FES_EFEX_VERIFY     *VerifyData= (FES_EFEX_VERIFY *)EfexTransfer->BaseSendBuffer;

        VerifyData->Flag     = EFEX_CRC32_VALID_FLAG;
        VerifyData->MediaCrc = EfexTransfer->LastError;

        EFEX_INFO("FEX_CMD_fes_verify last err=%d\n", VerifyData->MediaCrc);
      }
      EfexTransfer->ActualSendBuffer   = EfexTransfer->BaseSendBuffer;
      EfexTransfer->SizeNeedToBeSent         = sizeof(FES_EFEX_VERIFY);
      EfexTransfer->AppNextState   = SunxiUsbEfexAppSendData;

      break;
    case FEX_CMD_fes_query_storage:
      EFEX_DEBUG_INFO("FEX_CMD_fes_query_storage\n");
      {
        UINT32 *storage_type = (UINT32 *)EfexTransfer->BaseSendBuffer;

        *storage_type = SunxiGetBurnStorage();
        EfexTransfer->ActualSendBuffer   = EfexTransfer->BaseSendBuffer;
        EfexTransfer->SizeNeedToBeSent         = 4;
        EfexTransfer->LastError          = 0;
        EfexTransfer->AppNextState   = SunxiUsbEfexAppSendData;
      }

      break;

    case FEX_CMD_fes_flash_set_on:
      EFEX_DEBUG_INFO("FEX_CMD_fes_flash_set_on\n");
      EfexTransfer->LastError =  This->FlashIo->SunxiFlashIoInit(This->FlashIo,0);
      EfexTransfer->AppNextState   = SunxiUsbEfexAppStatus;

      break;

    case FEX_CMD_fes_flash_set_off:
      EFEX_DEBUG_INFO("FEX_CMD_fes_flash_set_off\n");
      EfexTransfer->LastError = This->FlashIo->SunxiFlashIoExit(This->FlashIo,0);
      EfexTransfer->AppNextState   = SunxiUsbEfexAppStatus;

      break;

    case FEX_CMD_fes_flash_size_probe:
      EFEX_DEBUG_INFO("FEX_CMD_fes_flash_size_probe\n");
      {
        UINT32 *FlashSize = (UINT32 *)EfexTransfer->BaseSendBuffer;
        *FlashSize = (UINT32)(This->FlashIo->BlockIo->Media->LastBlock);
        EFEX_INFO("flash sectors: 0x%x\n", *FlashSize);
        EfexTransfer->ActualSendBuffer   = EfexTransfer->BaseSendBuffer;
        EfexTransfer->SizeNeedToBeSent         = 4;
        EfexTransfer->LastError          = 0;
        EfexTransfer->AppNextState   = SunxiUsbEfexAppSendData;
      }
      break;

    case FEX_CMD_fes_tool_mode:
      EFEX_DEBUG_INFO("FEX_CMD_fes_tool_mode\n");
      {
        EFEX_TOOL *FesWork = (EFEX_TOOL *)CmdBuffer;

        if(FesWork->ToolMode== WORK_MODE_USB_TOOL_UPDATE)
        { //if it is update tools ,next work is restart system
          if(!FesWork->NextMode)
          {
            This->NextAction= SUNXI_UPDATE_NEXT_ACTION_REBOOT;
          }
          else
          {
            This->NextAction = FesWork->NextMode;
          }
          EfexTransfer->AppNextState = SUNXI_USB_EFEX_APPS_EXIT;
        }
        else  //if it is mass product  tools,next work  is by config
        {
          if(!FesWork->NextMode)
          {
            if(script_parser_fetch("platform", "next_work", (INT32*)&This->NextAction, 1))
            { //if next_work is not set ,do nothing 
              This->NextAction = SUNXI_UPDATE_NEXT_ACTION_NORMAL;
            }
          }
          else
          {
            This->NextAction = SUNXI_UPDATE_NEXT_ACTION_REBOOT;
          }
          if((This->NextAction <= SUNXI_UPDATE_NEXT_ACTION_NORMAL) || (This->NextAction > SUNXI_UPDATE_NEXT_ACTION_REUPDATE))
          {
            This->NextAction = SUNXI_UPDATE_NEXT_ACTION_NORMAL;
            EfexTransfer->AppNextState   = SunxiUsbEfexAppStatus;
          }
          else
          {
            EfexTransfer->AppNextState   = SUNXI_USB_EFEX_APPS_EXIT;
          }
        }
      }
      EFEX_DEBUG_INFO("sunxi_efex_next_action=%d\n", This->NextAction);
      EfexTransfer->LastError          = 0;

      break;
            
    case FEX_CMD_fes_reset_cpu:   
      AsciiPrint("reset cpu \n");        
      This->NextAction = SUNXI_UPDATE_NEXT_ACTION_REBOOT;         
      EfexTransfer->AppNextState   = SUNXI_USB_EFEX_APPS_EXIT;         
      EfexTransfer->LastError = 0;           
      if(logo_buf)            
      {  
        free(logo_buf);            
        logo_buf = NULL;           
      }      
      break;
#if 1
    case  FEX_CMD_fes_memset:
      EFEX_DEBUG_INFO("FEX_CMD_fes_memset\n");
      {
        FES_EFEX_MEMSET *fes_memset = (FES_EFEX_MEMSET *)CmdBuffer;

        EFEX_DEBUG_INFO("start 0x%x, value 0x%x, length 0x%x\n", fes_memset->StartAddress, fes_memset->Value & 0xff, fes_memset->Length);
        SetMem((void *)fes_memset->StartAddress, fes_memset->Length,fes_memset->Value & 0xff);
      }
      EfexTransfer->LastError          = 0;
      EfexTransfer->AppNextState   = SunxiUsbEfexAppStatus;

      break;

    case FEX_CMD_fes_pmu:
      EFEX_DEBUG_INFO("FEX_CMD_fes_pmu\n");
      {
        FES_EFEX_PMU *fes_pmu = (FES_EFEX_PMU *)CmdBuffer;

        EfexTransfer->TryToReceiveBytes = fes_pmu->Size;
                EfexTransfer->ActualReceiveBuffer = EfexTransfer->BaseReceiveBuffer;
        EfexTransfer->DataType      = fes_pmu->Type;

        ZeroMem(&PmuConfig, sizeof( PMU_CONFIG));

        EfexTransfer->LastError        = 0;
        EfexTransfer->AppNextState = SunxiUsbEfexAppReceiveData;
      }

      break;

    case FEX_CMD_fes_unseqmem_read:
      EFEX_DEBUG_INFO("FEX_CMD_fes_unseqmem_read\n");
      {
        EFEX_UNSEQ_MEM  *fes_unseq = (EFEX_UNSEQ_MEM *)CmdBuffer;

        EfexTransfer->ActualReceiveBuffer = EfexTransfer->BaseReceiveBuffer;
        EfexTransfer->TryToReceiveBytes   = fes_unseq->Size;
        EfexTransfer->DataType      = fes_unseq->Type;

        if(GlobalUnseqMemAddr.UnseqMemory == NULL)
        {
          EFEX_INFO("there is no memory to load unsequence data\n");
          EfexTransfer->LastError        = -1;
          EfexTransfer->ActualSendBuffer = EfexTransfer->BaseSendBuffer;//PcdGet64(PcdSystemMemoryBase);
        }
        else
        {
          int i;
          UNSEQ_MEM_CONFIG *unseq_mem = GlobalUnseqMemAddr.UnseqMemory;

          for(i=0;i<GlobalUnseqMemAddr.Count;i++)
          {
            unseq_mem[i].Value = readl(unseq_mem[i].Address);
            EFEX_DEBUG_INFO("read 0x%x, value 0x%x\n", unseq_mem[i].Address, unseq_mem[i].Value);
          }
          EfexTransfer->LastError        = 0;
          EfexTransfer->ActualSendBuffer = (UINT8*)(GlobalUnseqMemAddr.UnseqMemory);

        }
        EfexTransfer->SizeNeedToBeSent       = GlobalUnseqMemAddr.Count * sizeof(UNSEQ_MEM_CONFIG);
        EfexTransfer->AppNextState = SunxiUsbEfexAppSendData;
      }

      break;
    case FEX_CMD_fes_unseqmem_write:
      EFEX_DEBUG_INFO("FEX_CMD_fes_unseqmem_write\n");
      {
        EFEX_UNSEQ_MEM  *fes_unseq = (EFEX_UNSEQ_MEM *)CmdBuffer;

        EfexTransfer->ActualReceiveBuffer = EfexTransfer->BaseReceiveBuffer;
        EfexTransfer->TryToReceiveBytes   = fes_unseq->Size;
        EfexTransfer->DataType      = fes_unseq->Type;

        if(GlobalUnseqMemAddr.UnseqMemory != NULL)
        {
          FreePool(GlobalUnseqMemAddr.UnseqMemory);
        }
        GlobalUnseqMemAddr.UnseqMemory = (UNSEQ_MEM_CONFIG *)AllocateZeroPool(fes_unseq->Count * sizeof(UNSEQ_MEM_CONFIG));
        //memset(GlobalUnseqMemAddr.UnseqMemory, 0, fes_unseq->count * sizeof(UNSEQ_MEM_CONFIG));
        GlobalUnseqMemAddr.Count = fes_unseq->Count;

        EfexTransfer->LastError        = 0;
        EfexTransfer->AppNextState = SUNXI_USB_EFEX_APPS_RECEIVE_DATA;
      }
        break;
    #endif
    default:
      EFEX_ERROR("not supported command 0x%x now\n", Cmd->AppCmd);

      EfexTransfer->LastError        = -1;
      EfexTransfer->AppNextState = SunxiUsbEfexAppStatus;
  }

  return 0;
}

#include <Interinc/sunxi_uefi.h>
#include <IndustryStandard/Bmp.h>

EFI_STATUS
DramDataReceiveFinish(IN OUT SUNXI_EFEX *This)
{ 
  #if 1
  EFI_STATUS Status=EFI_SUCCESS;
  EFEX_TRANSFER *EfexTransfer = &This->Transfer;
  SUNXI_FLASH_IO_PROTOCOL *FlashIo = This->FlashIo;

  //PMU_CONFIG *PmuConfig;
  
  UINT32 DataType = EfexTransfer->DataType & SUNXI_EFEX_DATA_TYPE_MASK;

  if (DataType ==  FEX_CMD_BOOT2_DOWNLOAD_SYS_CONFIG)
  {
    CHAR8 *tmp_target_buffer = NULL;
    UINT32 script_offset = 0;
    UINT32 script_length = 0;
    struct spare_boot_head_t* spare_head; 
    spare_head   = get_spare_head(PcdGet32 (PcdFdBaseAddress));
    AsciiPrint("download sys_config form PC \n");

    //get script_offset and script_length
    script_offset = spare_head->boot_head.uefi_length;
    script_length = spare_head->boot_head.length - script_offset;
    // tmp_target_buffer = (void *)(CONFIG_SYS_TEXT_BASE -0x01000000);
    tmp_target_buffer = (char*)AllocateZeroPool(1024*1024);
    ASSERT(tmp_target_buffer != NULL);

    //copy uboot.bin to CONFIG_SYS_TEXT_BASE -0x01000000
    CopyMem(tmp_target_buffer,(char *)(PcdGet32 (PcdFdBaseAddress)),spare_head->boot_head.uefi_length);
    //copy sys_config.bin to tmp_buf
    CopyMem(tmp_target_buffer+script_offset,(void *)EfexTransfer->BaseReceiveBuffer,script_length);

    spare_head = (struct spare_boot_head_t*)tmp_target_buffer;
    spare_head->boot_head.check_sum = SunxiSpriteGenerateChecksum((void*)tmp_target_buffer,spare_head->boot_head.length,spare_head->boot_head.check_sum);
    if(0 != SunxiSpriteVerifyChecksum((void*)tmp_target_buffer,spare_head->boot_head.length,spare_head->boot_head.check_sum))
    {
      AsciiPrint("ReGenerate Check Sum Fail\n");
      EfexTransfer->LastError = -1;
    }
    else
    {
      Status = FlashIo->SunxiFlashIoWriteBootFiles(FlashIo,SunxiBootFileUefi,\
        (VOID *)tmp_target_buffer,spare_head->boot_head.length);

      EfexTransfer->LastError = Status;
    }

  }
  else if(DataType == FEX_CMD_BOOT2_DOWNLOAD_LOGO)
  {
    BMP_IMAGE_HEADER *BmpHead = NULL;
    EFI_STATUS Status ;
    AsciiPrint("recv boot_logo_info from pc %d  \n",EfexTransfer->ActuallyReceivedBytes);
    BmpHead = (BMP_IMAGE_HEADER *)EfexTransfer->BaseReceiveBuffer;
    //judge logo is bmp
    if((BmpHead->CharB !='B') || (BmpHead->CharM !='M')) 
    {
      AsciiPrint("this is not a bmp picture \n");
      EfexTransfer->LastError = -1;
    }
    else
    {
      Status = WriteFileToAwFs(L"bootlogo.bmp",EfexTransfer->BaseReceiveBuffer,BmpHead->ImageSize);
      EfexTransfer->LastError = EFI_ERROR(Status)? -1:0;
    }  
  }
  
  if(DataType == SUNXI_EFEX_MBR_TAG)      //mbr data is ok
  {
    EfexTransfer->LastError = -1;
  }
  else if(DataType == SUNXI_EFEX_BOOT1_TAG) //boot1 data is ok
  {
    EFEX_INFO("SUNXI_EFEX_BOOT1_TAG\n");
    EFEX_INFO("boot1 size = 0x%lx\n", EfexTransfer->ActuallyReceivedBytes);
    Status = FlashIo->SunxiFlashIoWriteBootFiles(FlashIo,SunxiBootFileUefi,\
        (VOID *)EfexTransfer->BaseReceiveBuffer,EfexTransfer->ActuallyReceivedBytes);
    
    EfexTransfer->LastError = Status;
  }
  else if(DataType == SUNXI_EFEX_BOOT0_TAG) //boot0 data is ok
  {
    EFEX_INFO("SUNXI_EFEX_BOOT0_TAG\n");
    EFEX_INFO("boot0 size = 0x%lx\n", EfexTransfer->ActuallyReceivedBytes);
    Status = FlashIo->SunxiFlashIoWriteBootFiles(FlashIo,SunxiBootFileBoot0,\
        (VOID *)EfexTransfer->BaseReceiveBuffer,EfexTransfer->ActuallyReceivedBytes);
    
    EfexTransfer->LastError = Status;
  }
  else if(DataType == SUNXI_EFEX_ERASE_TAG)
  {
    UINT32 EraseFlag;

    EFEX_INFO("SUNXI_EFEX_ERASE_TAG\n");
    EraseFlag = *(UINT32 *)EfexTransfer->BaseReceiveBuffer;
    if(EraseFlag)
    {
      EraseFlag = 1;
    }
    EFEX_INFO("EraseFlag = 0x%x\n", EraseFlag);
    script_parser_patch("platform","eraseflag",&EraseFlag,1);
  }
#if 1
  else if(DataType == SUNXI_EFEX_PMU_SET)
  {
    AXP_POWER_PROTOCOL  *AxpPowerProtocol = NULL;
    CopyMem(&PmuConfig, (void *)EfexTransfer->ActualReceiveBuffer, EfexTransfer->ActuallyReceivedBytes);
    Status = gBS->LocateProtocol (&gAxpPowerProtocolGuid, NULL, (VOID **)&AxpPowerProtocol);
    //PmuConfig.PmuType;
    if(EFI_ERROR(Status))
    {
      EfexTransfer->LastError = -1;
    }
    else
    {  
      EfexTransfer->LastError = AxpPowerProtocol->SetSupplyStatusByName((CHAR8*)(PmuConfig.VoltageName), PmuConfig.Voltage, PmuConfig.Gate);
    }
  }
  else if(DataType == SUNXI_EFEX_UNSEQ_MEM_FOR_WRITE)
  {
    INT32 i;
    UNSEQ_MEM_CONFIG *unseq_mem = GlobalUnseqMemAddr.UnseqMemory;

    EFEX_INFO("begin to load data to unsequency memory\n");
    CopyMem(unseq_mem, (void *)EfexTransfer->ActualReceiveBuffer, EfexTransfer->ActuallyReceivedBytes);
    for(i=0;i<GlobalUnseqMemAddr.Count;i++)
    {
      EFEX_DEBUG_INFO("write 0x%x, value 0x%x\n", unseq_mem[i].Address, unseq_mem[i].Value);
      writel(unseq_mem[i].Value, unseq_mem[i].Address);
    }
  }
  else if(DataType == SUNXI_EFEX_UNSEQ_MEM_FOR_READ)
  {
    UNSEQ_MEM_CONFIG *unseq_mem = GlobalUnseqMemAddr.UnseqMemory;

    EFEX_INFO("begin to set Addressess to unsequency memory\n");
    CopyMem(unseq_mem, (void *)EfexTransfer->ActualReceiveBuffer, EfexTransfer->ActuallyReceivedBytes);
  }
#endif
  else//other data ,write to memory
  {
    CopyMem((void *)EfexTransfer->DramTransferBuffer, (void *)EfexTransfer->ActualReceiveBuffer, EfexTransfer->ActuallyReceivedBytes);
    EFEX_DEBUG_INFO("SUNXI_EFEX_DRAM_TAG\n");
    EfexTransfer->LastError = 0;
  }
  EfexTransfer->ActuallyReceivedBytes = 0;
#endif
  return Status;
}


EFI_STATUS 
SunxiEfexSendCsw(SUNXI_EFEX* This,SUNXI_EFEX_CSW* Csw)
{
  EFI_USBFN_IO_PROTOCOL *UsbIo=This->UsbIo;
  UINTN TxSize =sizeof(SUNXI_EFEX_CSW);

  CopyMem((VOID*)This->Transfer.DataSetUpBuffer,(VOID*)Csw,TxSize);
  return UsbIo->Transfer(UsbIo,EFEX_BULK_ENDPOINT_INDEX,\
        EfiUsbEndpointDirectionDeviceTx,&TxSize,(VOID*)This->Transfer.DataSetUpBuffer);
}

EFI_STATUS  SunxiEfexAppSendStatus(SUNXI_EFEX* This)
{
  SUNXI_EFEX_STATUS  *EfexStatus;
  EFI_USBFN_IO_PROTOCOL *UsbIo=This->UsbIo;
  UINTN TxSize =sizeof(SUNXI_EFEX_STATUS);
  
  EfexStatus = (SUNXI_EFEX_STATUS *)This->Transfer.BaseSendBuffer;
  ZeroMem(EfexStatus,sizeof(SUNXI_EFEX_STATUS));
  EfexStatus->Mark  = 0xffff;
  EfexStatus->Tag   = 0;
  EfexStatus->State = 0;
  
  return UsbIo->Transfer(UsbIo,EFEX_BULK_ENDPOINT_INDEX,\
        EfiUsbEndpointDirectionDeviceTx,&TxSize,(VOID*)EfexStatus);  
}

  
EFI_STATUS
EfexRxHandle(
  SUNXI_EFEX  *This,
  EFI_USBFN_TRANSFER_RESULT* TransferResult
)
{

  UINTN RxSize =0;
  UINTN TxSize =0;
  EFI_USBFN_IO_PROTOCOL *UsbIo=This->UsbIo;
  EFEX_TRANSFER *EfexTransfer = &This->Transfer;
  UINTN  BytesTransferred  =TransferResult->BytesTransferred;
  EFI_USBFN_TRANSFER_STATUS TransferStatus =TransferResult->TransferStatus;
  VOID *Buffer =TransferResult->Buffer;

  static SUNXI_EFEX_CBW  *Cbw;
  static SUNXI_EFEX_CSW   Csw;

  if(TransferResult->EndpointIndex!=EFEX_BULK_ENDPOINT_INDEX){
    return EFI_SUCCESS;
  }
  if(TransferStatus!=UsbTransferStatusComplete){
    EFEX_DEBUG_INFO("%a:Transfer has not completed yet\n");
    return EFI_SUCCESS;
  }
  #if 0
  EFEX_DEBUG_INFO("********%a:TransferStatus: %d********\n",__FUNCTION__,TransferResult->TransferStatus);
  EFEX_DEBUG_INFO("********%a:EndpointIndex: %d********\n",__FUNCTION__,TransferResult->EndpointIndex);
  EFEX_DEBUG_INFO("********%a:Direction: %a********\n",__FUNCTION__,TransferResult->Direction==EfiUsbEndpointDirectionDeviceTx?"DeviceTx":"DeviceRx");
  EFEX_DEBUG_INFO("********%a:Buffer: 0x%p********\n",__FUNCTION__,TransferResult->Buffer);
  EFEX_DEBUG_INFO("********%a:BytesTransferred: %d********\n",__FUNCTION__,TransferResult->BytesTransferred);
  #endif
  switch(This->EfexState){
    case SunxiUsbEfexSetup:
      //receive Cbw here...if Cbw is vild, 
      EFEX_DEBUG_INFO("SUNXI_USB_EFEX_SETUP\n");
      if((BytesTransferred == sizeof(SUNXI_EFEX_CBW)))
      {
        Cbw = (SUNXI_EFEX_CBW*)Buffer;
        if(CBW_MAGIC != Cbw->Magic)
        {
          EFEX_ERROR("the Cbw signature 0x%x is bad, need 0x%x\n", Cbw->Magic, CBW_MAGIC);

          This->EfexState = SunxiUsbEfexSetup;
          break;
        }
      } 
      else if(BytesTransferred == FES_NEW_CMD_LEN)
      {
        EFEX_DEBUG_INFO("----------new cmd format--------\n");
        if(CBW_MAGIC != ((UINT32*)(Buffer))[4])
        {
          EFEX_ERROR("the cmd signature 0x%x is bad, need 0x%x\n",
               ((UINT32*)(Buffer))[4],CBW_MAGIC);

          This->EfexState = SunxiUsbEfexSetup;
          break;
        }

        This->EfexState = SunxiUsbEfexSetupNew;
        goto EfexSetupNew;           
      }
      else
      {
        EFEX_ERROR("received bytes 0x%x is not equal Cbw struct size 0x%x or new cmd size 0x%x\n",
           BytesTransferred, sizeof(SUNXI_EFEX_CBW),FES_NEW_CMD_LEN);
        This->EfexState = SunxiUsbEfexSetup;
        break;
      }
      Csw.Magic = CSW_MAGIC;    //"AWUS"
      Csw.Tag   = Cbw->Tag;
      EFEX_DEBUG_INFO("usb Cbw trans direction = 0x%x\n", Cbw->CmdPackage.Direction);
      if(This->AppState== SunxiUsbEfexAppIdle)
      {
        EFEX_DEBUG_INFO("APPS: SUNXI_USB_EFEX_APPS_IDLE\n");
        if(Cbw->CmdPackage.Direction == TL_CMD_RECEIVE) //recv data from pc
        {
          EFEX_DEBUG_INFO("APPS: SUNXI_USB_EFEX_APPS_IDLE: TL_CMD_RECEIVE\n");
          RxSize = MIN(Cbw->DataTransferLen, CBW_MAX_CMD_SIZE);
          EFEX_DEBUG_INFO("try to receive data 0x%x\n",RxSize);
          if(RxSize)
          {
            UsbIo->Transfer(UsbIo,EFEX_BULK_ENDPOINT_INDEX,\
             EfiUsbEndpointDirectionDeviceRx,&RxSize,(VOID*)EfexTransfer->DataSetUpBuffer);
          }
          else
          {
            EFEX_DEBUG_INFO("APPS: SUNXI_USB_EFEX_APPS_IDLE: the send data length is 0\n");
            return -1;
          }
          //next step will recv app cmd
          This->EfexState = SunxiUsbEfexReceiveData;
          This->AppState = SunxiUsbEfexAppCmd;
        }
        else  
        {
          //setup step is the  first step of usb  bulk,only  support recv data
          EFEX_ERROR("APPS: SUNXI_USB_EFEX_APPS_IDLE: INVALID direction\n");
          EFEX_ERROR("sunxi usb efex app cmd err: usb transfer direction is receive only\n");
                    
          return -1;
        }
      }
      else if((This->AppState == SunxiUsbEfexAppSendData) ||      \
        (This->AppState == SunxiUsbEfexAppReceiveData)) //
      {
        EFEX_DEBUG_INFO("APPS: SUNXI_USB_EFEX_APPS_DATA\n");
        switch(Cbw->CmdPackage.Direction){
          case TL_CMD_RECEIVE:           
            EFEX_DEBUG_INFO("APPS: SUNXI_USB_EFEX_APPS_DATA: TL_CMD_RECEIVE\n");
            RxSize = MIN(Cbw->DataTransferLen, EfexTransfer->TryToReceiveBytes);  
            EFEX_DEBUG_INFO("try to receive data 0x%x\n", Cbw->DataTransferLen);
            if(RxSize)
            {
              EFEX_DEBUG_INFO("dma recv Address = 0x%x\n", EfexTransfer->ActualReceiveBuffer);
              UsbIo->Transfer(UsbIo,EFEX_BULK_ENDPOINT_INDEX,\
               EfiUsbEndpointDirectionDeviceRx,&RxSize,(VOID*)EfexTransfer->ActualReceiveBuffer);
            }

            break;
              
          case TL_CMD_TRANSMIT:            
            EFEX_DEBUG_INFO("APPS: SUNXI_USB_EFEX_APPS_DATA: TL_CMD_TRANSMIT\n");
            TxSize = MIN(Cbw->DataTransferLen, EfexTransfer->SizeNeedToBeSent); 
            EFEX_DEBUG_INFO("try to transmit data 0x%x\n",Cbw->DataTransferLen);
            if(TxSize)
            {
              EFEX_DEBUG_INFO("dma send Address = 0x%x\n", EfexTransfer->ActualSendBuffer);
              UsbIo->Transfer(UsbIo,EFEX_BULK_ENDPOINT_INDEX,\
               EfiUsbEndpointDirectionDeviceTx,&TxSize,(VOID*)EfexTransfer->ActualSendBuffer);
            }

            break;
              
          default:
            EFEX_ERROR("APPS: SUNXI_USB_EFEX_APPS_DATA: the send data length is 0\n");
            return -1;
        }
        
        This->EfexState = This->AppState&0xffff;
                
      }
      else if(This->AppState == SunxiUsbEfexAppStatus)
      {
        EFEX_DEBUG_INFO("APPS: SunxiUsbEfexAppStatus\n");
        if(Cbw->CmdPackage.Direction == TL_CMD_TRANSMIT)
        {
          EFEX_DEBUG_INFO("APPS: SunxiUsbEfexAppStatus: TL_CMD_TRANSMIT\n");
          SunxiEfexAppSendStatus(This);

          This->EfexState = SunxiUsbEfexSendData;
        }
        else  //last step,only support send data
        {
          EFEX_ERROR("APPS: SunxiUsbEfexAppStatus: INVALID direction\n");
          EFEX_ERROR("usb transfer direction is transmit only\n");
        }
      }
      else if(This->AppState == SunxiUsbEfexAppExit)
      {
        EFEX_DEBUG_INFO("APPS: SUNXI_USB_EFEX_APPS_EXIT\n");
        SunxiEfexAppSendStatus(This);
        This->EfexState = SunxiUsbEfexSendData;
      }
      
       break;

    case SunxiUsbEfexSendData:
       break;
    case SunxiUsbEfexReceiveData:       
      EFEX_DEBUG_INFO("SUNXI_USB_RECEIVE_DATA\n");
      Csw.Status = 0;
      
      if(This->AppState == SunxiUsbEfexAppCmd)
      {
        EFEX_DEBUG_INFO("SUNXI_USB_RECEIVE_DATA: SunxiUsbEfexAppCmd\n");
        if(BytesTransferred != CBW_MAX_CMD_SIZE)
        {
          EFEX_ERROR("received cmd size 0x%x is not equal to CBW_MAX_CMD_SIZE 0x%x\n", BytesTransferred, CBW_MAX_CMD_SIZE);

          This->EfexState= SunxiUsbEfexSetup;
          Csw.Status = -1;
        }
        else
        {
          EFEX_DEBUG_INFO("begain to parse the cmd...\n");
          SunxiEfexUsbOpCmd(This,Buffer);
          Csw.Status = EfexTransfer->LastError;
        }
        
        This->AppState = EfexTransfer->AppNextState;
      }
      else if(This->AppState == SunxiUsbEfexAppReceiveData)
      {
        //UINT32 DataType = EfexTransfer->DataType & SUNXI_EFEX_DATA_TYPE_MASK;
        EFEX_DEBUG_INFO("SUNXI_USB_RECEIVE_DATA: SUNXI_USB_EFEX_APPS_RECEIVE_DATA\n");
        This->AppState = SunxiUsbEfexAppStatus;
        if(EfexTransfer->DataType & SUNXI_EFEX_DRAM_MASK)
        {
          EFEX_DEBUG_INFO("SUNXI_EFEX_DRAM_MASK\n");
          if(EfexTransfer->DataType & SUNXI_EFEX_TRANS_FINISH_TAG)
          {
            DramDataReceiveFinish(This);
          }
          //not finish,continue to recv data
        }
        else
        {
                    //data to flash
          EFEX_DEBUG_INFO("SUNXI_EFEX_FLASH_MASK\n");
          if(This->FlashIo->SunxiFlashIoWrite(This->FlashIo,EfexTransfer->FlashStart, EfexTransfer->FlashSectors, (void *)EfexTransfer->ActualReceiveBuffer))
          {
            EFEX_ERROR("write flash from 0x%x, sectors 0x%x failed\n", EfexTransfer->FlashStart, EfexTransfer->FlashSectors);
            Csw.Status = -1;
            EfexTransfer->LastError= -1;

            This->AppState = SunxiUsbEfexAppIdle;
          }
        }
      }
      This->EfexState= SunxiUsbEfexStatus;      //next step is send (csw)
      break;
       
    case SunxiUsbEfexSetupNew: 
      {
EfexSetupNew:                
        EFEX_DEBUG_INFO("SUNXI_USB_EFEX_SETUP_NEW\n");
#ifdef _EFEX_USE_BUF_QUEUE_ 
        //flush queue buff   when verify cmd or flash set off cmd coming
        if(FEX_CMD_fes_verify_value == ((GLOBAL_CMD*)Buffer)->AppCmd
           ||  FEX_CMD_fes_flash_set_off==  ((GLOBAL_CMD*)Buffer)->AppCmd)
        {
          EFEX_DEBUG_INFO("verify or flash set off cmd,Queue write all back\n");
          if(EfexQueueWriteAllPage(This))
          {
            EFEX_ERROR("efex queue error: buf_queue_write_all_page fail\n");
            Csw.Status = -1;
            Csw.Magic = CSW_MAGIC;      //"AWUS"
            Csw.Tag   = 0;
            This->EfexState= SunxiUsbEfexStatus;
          }
        }
#endif
        SunxiEfexUsbOpCmd(This,Buffer);
        Csw.Status = EfexTransfer->LastError;
        Csw.Magic = CSW_MAGIC;      //"AWUS"
        Csw.Tag   = 0;
        if(Csw.Status != 0 )
        {
          EFEX_DEBUG_INFO("sunxi usb cmd error: 0x%x\n",Csw.Status);
          This->EfexState= SunxiUsbEfexStatus;
          break;
        }

        switch(EfexTransfer->AppNextState){
          case SunxiUsbEfexAppSendData:           
            This->EfexState = SunxiUsbEfexSendDataNew;
            This->AppState = SunxiUsbEfexAppSendData;

            EFEX_DEBUG_INFO("APPS: SUNXI_USB_EFEX_APPS_DATA:TRANSMIT\n");
            TxSize =EfexTransfer->SizeNeedToBeSent;
            EFEX_DEBUG_INFO("try to transmit data 0x%x\n",Cbw->DataTransferLen);
            if(TxSize)
            {
              UsbIo->Transfer(UsbIo,EFEX_BULK_ENDPOINT_INDEX,\
              EfiUsbEndpointDirectionDeviceTx,&TxSize,(VOID*)EfexTransfer->ActualSendBuffer);
            }
            break;
             
          case SunxiUsbEfexAppReceiveData:            
            This->EfexState = SunxiUsbEfexReceiveDataNew;
            This->AppState = SunxiUsbEfexAppSendData;  

            EFEX_DEBUG_INFO("APPS: SUNXI_USB_EFEX_APPS_DATA:RECEIVE\n");
            RxSize = EfexTransfer->TryToReceiveBytes;
            EFEX_DEBUG_INFO("try to receive data 0x%x\n", RxSize);
            if(RxSize)
            {
              UsbIo->Transfer(UsbIo,EFEX_BULK_ENDPOINT_INDEX,\
              EfiUsbEndpointDirectionDeviceRx,&RxSize,(VOID*)EfexTransfer->ActualReceiveBuffer);
            }
            break;
             
          case SunxiUsbEfexAppStatus:
            This->EfexState= SunxiUsbEfexStatus;
            break;
          case SunxiUsbEfexAppExit:
            This->EfexState= SunxiUsbEfexExit;
            SunxiEfexSendCsw(This,&Csw);
            break;
          default:
            EFEX_DEBUG_INFO("sunxi usb next status set error:0x%x\n",EfexTransfer->AppNextState);
            This->EfexState = SunxiUsbEfexIdle;
        }
      break;
      }

    case SunxiUsbEfexReceiveDataNew:       
      EFEX_DEBUG_INFO("SUNXI_USB_RECEIVE_DATA_NEW\n");
      This->AppState = SunxiUsbEfexAppStatus;
      if(EfexTransfer->DataType & SUNXI_EFEX_DRAM_MASK)
      {
        EFEX_DEBUG_INFO("SUNXI_EFEX_DRAM_MASK\n");
        if(EfexTransfer->DataType & SUNXI_EFEX_TRANS_FINISH_TAG)
        {
          DramDataReceiveFinish(This);
        }
      }
      else
      {
        EFEX_DEBUG_INFO("SUNXI_EFEX_FLASH_MASK\n");
#ifdef _EFEX_USE_BUF_QUEUE_
        if(EfexBufferEnqueue(This,EfexTransfer->FlashStart, EfexTransfer->FlashSectors, (void *)EfexTransfer->ActualReceiveBuffer))
        {
          EFEX_ERROR("efex queue not enough space...\n");
          EfexTransfer->LastError= -1;
        }
#else
        if(This->FlashIo->SunxiFlashIoWrite(This->FlashIo,EfexTransfer->FlashStart, EfexTransfer->FlashSectors, (void *)EfexTransfer->ActualReceiveBuffer))
        {
          EFEX_ERROR("write flash from 0x%x, sectors 0x%x failed\n", EfexTransfer->FlashStart, EfexTransfer->FlashSectors);
          EfexTransfer->LastError= -1;
          This->AppState = SunxiUsbEfexAppIdle;
        }       
#endif
      }
      Csw.Status = EfexTransfer->LastError;
      This->EfexState = SunxiUsbEfexStatus;
      break;
      
    default:
      EFEX_ERROR("State Machine Error!\n");
  }

  if(This->EfexState== SunxiUsbEfexStatus){
    SunxiEfexSendCsw(This,&Csw);
  }
    
  return EFI_SUCCESS;
}

 EFI_STATUS
EfexTxHandle(
  SUNXI_EFEX  *This,
  EFI_USBFN_TRANSFER_RESULT* TransferResult
 )
{ 

  UINT16 MaxPacketSize=0;
  UINTN TxSize =0;
  EFI_USBFN_IO_PROTOCOL *UsbIo=This->UsbIo;
  SUNXI_EFEX_CSW  Csw;
  
  Csw.Magic = CSW_MAGIC;    //"AWUS"
  Csw.Status = This->Transfer.LastError;
  
  if(TransferResult->EndpointIndex!=EFEX_BULK_ENDPOINT_INDEX){
    return EFI_SUCCESS;
  }
  if(TransferResult->TransferStatus!=UsbTransferStatusComplete){
    EFEX_DEBUG_INFO("%a:Transfer has not completed yet\n");
    return EFI_SUCCESS;
  }
  #if 0
  EFEX_DEBUG_INFO("********%a:TransferStatus: %d********\n",__FUNCTION__,TransferResult->TransferStatus);
  EFEX_DEBUG_INFO("********%a:EndpointIndex: %d********\n",__FUNCTION__,TransferResult->EndpointIndex);
  EFEX_DEBUG_INFO("********%a:Direction: %a********\n",__FUNCTION__,TransferResult->Direction==EfiUsbEndpointDirectionDeviceTx?"DeviceTx":"DeviceRx");
  EFEX_DEBUG_INFO("********%a:Buffer: 0x%p********\n",__FUNCTION__,TransferResult->Buffer);
  EFEX_DEBUG_INFO("********%a:BytesTransferred: %d********\n",__FUNCTION__,TransferResult->BytesTransferred);
  #endif
  switch(This->EfexState){
    case SunxiUsbEfexSetup:
       break;
    case SunxiUsbEfexSendData:      
      //data send complete, enter the 
      switch(This->AppState){
        case SunxiUsbEfexAppSendData:
          //send the csw
          SunxiEfexSendCsw(This,&Csw);
          //app data send compete, enter the app status phase.
          This->EfexState = SunxiUsbEfexStatus;
          This->AppState = SunxiUsbEfexAppStatus;
          break;
         
          //App status data send complete
        case SunxiUsbEfexAppStatus:
          //send the csw
          SunxiEfexSendCsw(This,&Csw);
          This->EfexState = SunxiUsbEfexStatus;
          //let's enter the app idle status.
          This->AppState = SunxiUsbEfexAppIdle;
          break;
        case SunxiUsbEfexAppExit:
          This->EfexState = SunxiUsbEfexExit;
          break;
        default:
          EFEX_ERROR("%a:%d:state machine error:%d!\n",__FUNCTION__,__LINE__,This->AppState);        
      }
      break;
    case SunxiUsbEfexReceiveData:
      break;
    case SunxiUsbEfexSendDataNew:
      SunxiEfexSendCsw(This,&Csw);
      This->EfexState = SunxiUsbEfexStatus;
      break;
    case SunxiUsbEfexStatus:
      //Csw has been sent, prepare a new bulk transfer for cmd.
      UsbIo->GetEndpointMaxPacketSize(UsbIo,UsbEndpointBulk,\
              UsbBusSpeedHigh,&MaxPacketSize);
      TxSize = MaxPacketSize;
      //set up Bulk receive buffer for the controller.
      UsbIo->Transfer(UsbIo,EFEX_BULK_ENDPOINT_INDEX,\
      EfiUsbEndpointDirectionDeviceRx,&TxSize,(VOID*)This->Transfer.DataSetUpBuffer);
      This->EfexState  = SunxiUsbEfexSetup;
      break;
    case SunxiUsbEfexExit:
      //inform that we have done our job
      EFEX_INFO("Usb Efex Exit\n");
    default:
      EFEX_ERROR("%a:%d:State Machine Error:%d!\n",__FUNCTION__,__LINE__,This->EfexState);
  }
    
  return EFI_SUCCESS;

}
 
EFI_STATUS
EfexUsbIoStateLoop (
  SUNXI_EFEX        *This
  )
{
  
  EFI_USBFN_MESSAGE Message;
  UINTN PayloadSize=0;
  EFI_USBFN_MESSAGE_PAYLOAD Payload;
  
  This->UsbIo->EventHandler(This->UsbIo,&Message,&PayloadSize,&Payload);

  switch(Message){   
    case EfiUsbMsgSetupPacket:
      DEBUG (( EFI_D_INFO,"\n\n"));
      EFEX_DEBUG_INFO("Message:EfiUsbMsgSetupPacket\n");
      EfexUsbIoSetUpHandle(This,&(Payload.udr));
      break;
        
    case EfiUsbMsgEndpointStatusChangedRx:
      DEBUG (( EFI_D_INFO,"\n\n"));
      EFEX_DEBUG_INFO("Message:EfiUsbMsgEndpointStatusChangedRx\n");
      EfexRxHandle(This,&(Payload.utr));     
      break;
        
    case EfiUsbMsgEndpointStatusChangedTx:
      DEBUG (( EFI_D_INFO,"\n\n"));
      EFEX_DEBUG_INFO("Message:EfiUsbMsgEndpointStatusChangedTx\n");
      EfexTxHandle(This,&(Payload.utr)); 
      break;
        
    case EfiUsbMsgBusEventDetach:
      DEBUG (( EFI_D_INFO,"\n\n"));
      EFEX_DEBUG_INFO("Message:EfiUsbMsgBusEventDetach\n\n");
      break;
        
    case EfiUsbMsgBusEventAttach:
      DEBUG (( EFI_D_INFO,"\n\n"));
      EFEX_DEBUG_INFO("Message:EfiUsbMsgBusEventAttach\n\n");
      break;
        
    case EfiUsbMsgBusEventReset:
      /*reset the SW state machine here.*/
      This->EfexState = SunxiUsbEfexSetup;
      This->AppState = SunxiUsbEfexAppIdle;
      // This->AppNextState = SunxiUsbEfexAppCmd;

      DEBUG (( EFI_D_INFO,"\n\n"));
      EFEX_DEBUG_INFO("Message:EfiUsbMsgBusEventReset\n\n");
      break;
        
    case EfiUsbMsgBusEventSuspend:
      DEBUG (( EFI_D_INFO,"\n\n"));
      EFEX_DEBUG_INFO("Message:EfiUsbMsgBusEventSuspend\n\n");      
      break;
        
    case EfiUsbMsgBusEventResume:
      DEBUG (( EFI_D_INFO,"\n\n"));
      EFEX_DEBUG_INFO("Message:EfiUsbMsgBusEventResume\n\n");
      break;
        
    case EfiUsbMsgBusEventSpeed:
      DEBUG (( EFI_D_INFO,"\n\n"));
      EFEX_DEBUG_INFO("Message:EfiUsbMsgBusEventSpeed\n\n");      
      break;
        
    case EfiUsbMsgNone:
      EfexQueueWriteOnePage(This);
      break;
  }   
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SunxiEfexSubsequentProcessing(SUNXI_EFEX *This)
{
  EFI_STATUS Status = EFI_SUCCESS;
  EFEX_INFO("next work %d\n", This->NextAction);
  switch(This->NextAction)
  {
    case SUNXI_UPDATE_NEXT_ACTION_REBOOT: //restart
      EFEX_INFO("SUNXI_UPDATE_NEXT_ACTION_REBOOT\n");
      gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);

      break;
    case SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN: //shutdown
      EFEX_INFO("SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN\n");
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);

      break;
    case SUNXI_UPDATE_NEXT_ACTION_REUPDATE:
      EFEX_INFO("SUNXI_UPDATE_NEXT_ACTION_REUPDATE\n");
      //sunxi_board_run_fel();      //efex

      break;
    case SUNXI_UPDATE_NEXT_ACTION_BOOT:
    case SUNXI_UPDATE_NEXT_ACTION_NORMAL:
    default:
      EFEX_INFO("SUNXI_UPDATE_NEXT_ACTION_NULL\n");
      //Status = EFI_DEVICE_ERROR;
      break;
  }

  return Status;

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
EfexMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{

  EFI_STATUS Status = EFI_SUCCESS;
  
  EFI_INPUT_KEY Key;
  
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *TextIn;

  SunxiEfex = AllocateZeroPool(sizeof(SUNXI_EFEX));
  if(!SunxiEfex){
    Status = EFI_OUT_OF_RESOURCES;  
    goto EfexMainOut;
  }

  Status = gBS->LocateProtocol (&gEfiSimpleTextInProtocolGuid, NULL, (VOID **) &TextIn);
  if (EFI_ERROR (Status)) {
    EFEX_ERROR("Couldn't open Text Input Protocol: %r\n", Status);
    goto EfexMainOut;
  }

  Status = EfexUsbIoInitialize(SunxiEfex,100,100);
  if(Status){
    EFEX_ERROR("EfexUsbIoInitialize error\n");
  }
  
  // Disable watchdog
  Status = gBS->SetWatchdogTimer (0, 0x10000, 0, NULL);
  if (EFI_ERROR (Status)) {
    EFEX_ERROR("Efex: Couldn't disable watchdog timer: %r\n", Status);
  }

  SunxiEfex->EfexState = SunxiUsbEfexSetup;
  SunxiEfex->AppState = SunxiUsbEfexAppIdle;

  SunxiEfex->NextAction = 0;
   
  // Talk to the user
  EFEX_INFO("enter efex_test\n");
  EFEX_INFO("Allwinner BootAuto Test - version 0.1. Press Q to quit.\r\n");

  while(1){
    EfexUsbIoStateLoop(SunxiEfex);
    if(SunxiEfex->NextAction) break;
    
    Status = TextIn->ReadKeyStroke(TextIn,&Key);
    if(Status==EFI_SUCCESS){
      if (Key.UnicodeChar == 113) {
        break;
      }
    }
  }

  Status = EfexUsbIoExit(SunxiEfex);
  if(Status){
    EFEX_ERROR("Can't exit Usb Io\n");
  }
  
  Status = SunxiEfexSubsequentProcessing(SunxiEfex);
 
EfexMainOut:
  if(SunxiEfex){
    FreePool(SunxiEfex);
  }

  return Status;
  
}
