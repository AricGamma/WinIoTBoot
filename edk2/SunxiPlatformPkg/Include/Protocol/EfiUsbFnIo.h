/** @file

  Copyright (c) 2007 - 2014, Allwinner Technology Co., Ltd. <www.allwinnertech.com>

  Martin.zheng <zhengjiewen@allwinnertech.com>
  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SUNXI_EFI_USBFN_IO_H__
#define __SUNXI_EFI_USBFN_IO_H__

#include <IndustryStandard/Usb.h>

// {32D2963A-FE5D-4f30-B633-6E5DC55803CC}
#define EFI_USBFN_IO_PROTOCOL_GUID \
  {0x32d2963a, 0xfe5d, 0x4f30, 0xb6, 0x33, 0x6e, 0x5d, 0xc5, \
   0x58, 0x3, 0xcc };


#define EFI_USBFN_IO_PROTOCOL_REVISION   0x00010001

//
// USB standard descriptors and reqeust
//
typedef USB_DEVICE_REQUEST        EFI_USB_DEVICE_REQUEST;
typedef USB_DEVICE_DESCRIPTOR     EFI_USB_DEVICE_DESCRIPTOR;
typedef USB_CONFIG_DESCRIPTOR     EFI_USB_CONFIG_DESCRIPTOR;
typedef USB_INTERFACE_DESCRIPTOR  EFI_USB_INTERFACE_DESCRIPTOR;
typedef USB_ENDPOINT_DESCRIPTOR   EFI_USB_ENDPOINT_DESCRIPTOR;


/*
 * USB directions
 *
 * This bit flag is used in endpoint descriptors' bEndpointAddress field.
 * It's also one of three fields in control requests bRequestType.
 */
#define USB_DIR_OUT     0   /* to device */
#define USB_DIR_IN      0x80    /* to host */


/*
 * Device Requests  (c.f Table 9-2)
 */

#define USB_REQ_DIRECTION_MASK    0x80
#define USB_REQ_TYPE_MASK   0x60
#define USB_REQ_RECIPIENT_MASK    0x1f

#define USB_REQ_DEVICE2HOST   0x80
#define USB_REQ_HOST2DEVICE   0x00

#define USB_REQ_RECIPIENT_DEVICE  0x00
#define USB_REQ_RECIPIENT_INTERFACE 0x01
#define USB_REQ_RECIPIENT_ENDPOINT  0x02
#define USB_REQ_RECIPIENT_OTHER   0x03

//#define USBFN_DRIVER_DEBUG_ENABLE 1

#if(USBFN_DRIVER_DEBUG_ENABLE)
#define USBFN_DRIVER_DEBUG_INFO(ARG...)  DEBUG (( EFI_D_INFO,"[USBFN]:"ARG))
#define USBFN_DRIVER_DEBUG_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[USBFN]:"ARG))
#else
#define USBFN_DRIVER_DEBUG_INFO(ARG...)  
#define USBFN_DRIVER_DEBUG_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[USBFN]:"ARG))
#endif


typedef enum _EFI_USB_BUS_SPEED 
{
  UsbBusSpeedUnknown = 0,
  UsbBusSpeedLow,
  UsbBusSpeedFull,
  UsbBusSpeedHigh,
  UsbBusSpeedSuper,
  UsbBusSpeedMaximum = UsbBusSpeedSuper
} EFI_USB_BUS_SPEED;

typedef struct 
{
  EFI_USB_INTERFACE_DESCRIPTOR        *InterfaceDescriptor;
  EFI_USB_ENDPOINT_DESCRIPTOR         **EndpointDescriptorTable;
} EFI_USB_INTERFACE_INFO;

typedef struct 
{
  EFI_USB_CONFIG_DESCRIPTOR           *ConfigDescriptor;
  EFI_USB_INTERFACE_INFO              **InterfaceInfoTable;
} EFI_USB_CONFIG_INFO;

typedef struct 
{
  EFI_USB_DEVICE_DESCRIPTOR           *DeviceDescriptor;
  EFI_USB_CONFIG_INFO                 **ConfigInfoTable;
} EFI_USB_DEVICE_INFO;

typedef enum _EFI_USB_ENDPOINT_TYPE{
  UsbEndpointControl = 0x00,
  UsbEndpointIsochronous = 0x01,
  UsbEndpointBulk = 0x02,
  UsbEndpointInterrupt = 0x03
} EFI_USB_ENDPOINT_TYPE;



typedef enum _EFI_USBFN_DEVICE_INFO_ID   
{
  EfiUsbDeviceInfoUnknown = 0,
  EfiUsbDeviceInfoSerialNumber,
  EfiUsbDeviceInfoManufacturerName,
  EfiUsbDeviceInfoProductName
} EFI_USBFN_DEVICE_INFO_ID;

typedef enum _EFI_USBFN_ENDPOINT_DIRECTION 
{
  EfiUsbEndpointDirectionHostOut  = 0,
  EfiUsbEndpointDirectionHostIn,
  EfiUsbEndpointDirectionDeviceTx = EfiUsbEndpointDirectionHostIn,
  EfiUsbEndpointDirectionDeviceRx = EfiUsbEndpointDirectionHostOut
} EFI_USBFN_ENDPOINT_DIRECTION;

 
typedef enum _EFI_USBFN_MESSAGE
{
  EfiUsbMsgNone = 0,
  EfiUsbMsgSetupPacket,
  EfiUsbMsgEndpointStatusChangedRx,
  EfiUsbMsgEndpointStatusChangedTx,
  EfiUsbMsgBusEventDetach,
  EfiUsbMsgBusEventAttach,
  EfiUsbMsgBusEventReset,
  EfiUsbMsgBusEventSuspend,
  EfiUsbMsgBusEventResume,
  EfiUsbMsgBusEventSpeed
} EFI_USBFN_MESSAGE;
 
typedef enum _EFI_USBFN_POLICY_TYPE{
  EfiUsbPolicyUndefined = 0, 
  EfiUsbPolicyMaxTransactionSize, 
  EfiUsbPolicyZeroLengthTerminationSupport, 
  EfiUsbPolicyZeroLengthTermination
} EFI_USBFN_POLICY_TYPE;

typedef enum _EFI_USBFN_PORT_TYPE 
{
  EfiUsbUnknownPort = 0,
  EfiUsbStandardDownstreamPort,
  EfiUsbChargingDownstreamPort,
  EfiUsbDedicatedChargingPort,
  EfiUsbInvalidDedicatedChargingPort
} EFI_USBFN_PORT_TYPE;

typedef enum _EFI_USBFN_TRANSFER_STATUS 
{
  UsbTransferStatusUnknown = 0,
  UsbTransferStatusComplete,
  UsbTransferStatusAborted,
  UsbTransferStatusActive,
  UsbTransferStatusNone
} EFI_USBFN_TRANSFER_STATUS;

typedef struct _EFI_USBFN_TRANSFER_RESULT 
{
  UINTN                         BytesTransferred;
  EFI_USBFN_TRANSFER_STATUS     TransferStatus;
  UINT8                         EndpointIndex;
  EFI_USBFN_ENDPOINT_DIRECTION  Direction;
  VOID                          *Buffer;
} EFI_USBFN_TRANSFER_RESULT;

 
typedef union _EFI_USBFN_MESSAGE_PAYLOAD
{
  EFI_USB_DEVICE_REQUEST      udr;
  EFI_USBFN_TRANSFER_RESULT   utr;
  EFI_USB_BUS_SPEED           ubs;
} EFI_USBFN_MESSAGE_PAYLOAD;

typedef struct _EFI_USBFN_IO_PROTOCOL EFI_USBFN_IO_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_DETECT_PORT) (
  IN EFI_USBFN_IO_PROTOCOL   *This,
  OUT EFI_USBFN_PORT_TYPE    *PortType
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_CONFIGURE_ENABLE_ENDPOINTS) (
  IN EFI_USBFN_IO_PROTOCOL         *This,
  IN EFI_USB_DEVICE_INFO           *DeviceInfo
  );



typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_GET_DEVICE_INFO) (
  IN EFI_USBFN_IO_PROTOCOL      *This,
  IN EFI_USBFN_DEVICE_INFO_ID   Id,
  IN OUT UINTN                  *BufferSize,
  OUT VOID                      *Buffer OPTIONAL
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_GET_VENDOR_ID_PRODUCT_ID) (
  IN EFI_USBFN_IO_PROTOCOL      *This,
  OUT UINT16                    *Vid,
  OUT UINT16                    *Pid
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_ABORT_TRANSFER) (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_GET_ENDPOINT_STALL_STATE) (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction,
  IN OUT BOOLEAN                  *State
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_SET_ENDPOINT_STALL_STATE) (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction,
  IN BOOLEAN                      State
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_EVENTHANDLER) (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  OUT EFI_USBFN_MESSAGE           *Message,
  IN OUT UINTN                    *PayloadSize,
  OUT EFI_USBFN_MESSAGE_PAYLOAD   *Payload
  );

typedef
EFI_STATUS
(EFIAPI *EFI_USBFN_IO_TRANSFER) (
  IN EFI_USBFN_IO_PROTOCOL         *This,
  IN UINT8                         EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION  Direction,
  IN OUT UINTN                     *BufferSize,
  IN OUT VOID                      *Buffer
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_GET_MAXTRANSFER_SIZE) (
  IN EFI_USBFN_IO_PROTOCOL     *This,
  OUT UINTN                    *MaxTransferSize
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_ALLOCATE_TRANSFER_BUFFER) (
  IN EFI_USBFN_IO_PROTOCOL    *This,
  IN UINTN                    Size,
  OUT VOID                    **Buffer
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_FREE_TRANSFER_BUFFER) (
  IN EFI_USBFN_IO_PROTOCOL    *This,
  IN VOID                     *Buffer
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_GET_ENDPOINT_MAXPACKET_SIZE) (
  IN EFI_USBFN_IO_PROTOCOL      *This,
  IN EFI_USB_ENDPOINT_TYPE      EndpointType,
  IN EFI_USB_BUS_SPEED          BusSpeed,
  OUT UINT16                    *MaxPacketSize
  );

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_START_CONTROLLER) (
  IN EFI_USBFN_IO_PROTOCOL        *This
  );

 
typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_STOP_CONTROLLER) (
  IN EFI_USBFN_IO_PROTOCOL        *This
  );
 

typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_SET_ENDPOINT_POLICY) (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction,
  IN EFI_USBFN_POLICY_TYPE        PolicyType,
  IN UINTN                        BufferSize,
  IN VOID                         *Buffer
  );
 
typedef
EFI_STATUS
(EFIAPI * EFI_USBFN_IO_GET_ENDPOINT_POLICY) (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction,
  IN EFI_USBFN_POLICY_TYPE        PolicyType,
  IN OUT UINTN                    BufferSize,
  IN OUT VOID                     *Buffer
  );

  
struct _EFI_USBFN_IO_PROTOCOL 
{
  UINT32                                      Revision;
  EFI_USBFN_IO_DETECT_PORT                    DetectPort;
  EFI_USBFN_IO_CONFIGURE_ENABLE_ENDPOINTS     ConfigureEnableEndpoints;
  EFI_USBFN_IO_GET_ENDPOINT_MAXPACKET_SIZE    GetEndpointMaxPacketSize;
  EFI_USBFN_IO_GET_DEVICE_INFO                GetDeviceInfo;
  EFI_USBFN_IO_GET_VENDOR_ID_PRODUCT_ID       GetVendorIdProductId;
  EFI_USBFN_IO_ABORT_TRANSFER                 AbortTransfer;
  EFI_USBFN_IO_GET_ENDPOINT_STALL_STATE       GetEndpointStallState; 
  EFI_USBFN_IO_SET_ENDPOINT_STALL_STATE       SetEndpointStallState;
  EFI_USBFN_IO_EVENTHANDLER                   EventHandler;
  EFI_USBFN_IO_TRANSFER                       Transfer;
  EFI_USBFN_IO_GET_MAXTRANSFER_SIZE           GetMaxTransferSize;
  EFI_USBFN_IO_ALLOCATE_TRANSFER_BUFFER       AllocateTransferBuffer;
  EFI_USBFN_IO_FREE_TRANSFER_BUFFER           FreeTransferBuffer;
  EFI_USBFN_IO_START_CONTROLLER               StartController;
  EFI_USBFN_IO_STOP_CONTROLLER                StopController;
  EFI_USBFN_IO_SET_ENDPOINT_POLICY            SetEndpointPolicy;
  EFI_USBFN_IO_GET_ENDPOINT_POLICY            GetEndpointPolicy;
};


extern EFI_GUID gEfiUsbFnIoProtocolGuid;

#endif
