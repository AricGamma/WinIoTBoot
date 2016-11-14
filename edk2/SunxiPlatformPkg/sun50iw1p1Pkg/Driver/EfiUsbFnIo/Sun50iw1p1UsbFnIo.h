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

#ifndef  __SUN9IW1_USB_FN_IO_H__
#define  __SUN9IW1_USB_FN_IO_H__

#include <Library/SunxiQueueLib.h>
#include <Sunxi_type/Sunxi_type.h>

/* USB Requests
 *
 */

struct usb_device_request {
  u8 bmRequestType;
  u8 bRequest;
  u16 wValue;
  u16 wIndex;
  u16 wLength;
} __attribute__ ((packed));
  
typedef struct sunxi_udc
{
  ulong usbc_hd;

  u32 address;    /* device address, that host distribute */
  int speed;        /* flag. is high speed?         */

  u32 bulk_ep_max;  /* bulk ep max packet size        */
  u32 fifo_size;    /* fifo size              */
  u32 bulk_in_addr;
  u32 bulk_out_addr;
  u32 dma_send_channal;
  u32 dma_recv_channal;
  struct usb_device_request standard_reg;
} 
sunxi_udc_t;


typedef struct sunxi_ubuf_s
{
  UINT8 *rx_base_buffer;
  UINT8 *rx_req_buffer;     //bulk传输的请求阶段buffer
  UINT32   rx_req_length;     //bulk传输的请求阶段数据长度
  UINT32   tx_req_length;     //bulk传输的请求阶段数据长度
  UINT32   tx_package_send_finish;          //tx bulk finished flag.

  UINT32   rx_ready_for_data;   //表示数据接收已经完成标志

  UINT32   request_size;      //需要发送的数据长度
  UINT32   dma_rx_addr;
  UINT32   dma_rx_size;
}
sunxi_ubuf_t;


extern  void sunxi_udc_ep_reset(void);

//extern  INT32 sunxi_udc_start_recv_by_dma(UINT32 mem_buf, UINT32 length);

extern  void sunxi_udc_send_setup(UINT32 bLength, void *buffer);
extern  INT32  sunxi_udc_send_data(void *buffer,  UINT32 buffer_size);


extern  INT32 sunxi_udc_set_address(UINT8 address);
extern  INT32 sunxi_udc_set_configuration(INT32 config_param);

extern  INT32 sunxi_udc_get_ep_max(void);
extern  INT32 sunxi_udc_get_ep_in_type(void);
extern  INT32 sunxi_udc_get_ep_out_type(void);


#define USB_EVENT_QUEUE_MAX_EVENT_COUNT 64
#define USB_MAX_LOGIC_EP_COUNT    2

extern 
EFI_STATUS
UsbQueueEventMassage(
EFI_USBFN_MESSAGE             Message,
UINT8                         EndpointIndex,
EFI_USBFN_ENDPOINT_DIRECTION  Direction,
EFI_USBFN_TRANSFER_STATUS     TransferStatus,
VOID                          *Buffer,
UINTN                         BufferLengh
);

extern EFI_STATUS SunxiUsbInit(
  IN EFI_USBFN_IO_PROTOCOL *This,
  IN UINTN delaytime
  );

extern EFI_STATUS SunxiUsbExit(
  IN EFI_USBFN_IO_PROTOCOL *This
  );

extern
EFI_STATUS
SunxiEp0SendData(
  IN UINTN Length, 
  IN VOID *Buffer
  );

typedef struct{
 EFI_USBFN_MESSAGE           Message;
 UINTN                       PayloadSize;
 EFI_USBFN_MESSAGE_PAYLOAD   Payload;
}USB_EVENT_MESSAGE;


extern
EFI_STATUS
SunxiEpXSendData(
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                         EndpointIndex,
  IN OUT UINTN                     *BufferSize,
  IN OUT VOID                      *Buffer
);

extern
EFI_STATUS
SunxiEpXReceiveData(
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                         EndpointIndex,
  IN OUT UINTN                     *BufferSize,
  IN OUT VOID                      *Buffer
);

extern 
EFI_STATUS 
SunxiConfigureEnableEndpoints(
  IN EFI_USBFN_IO_PROTOCOL  *This,
  EFI_USB_DEVICE_INFO       *DeviceInfo
  );

extern 
EFI_STATUS
SunxiUsbFnIoAbortTransfer(
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction
  );

extern 
EFI_STATUS
SunxiSetEndpointStallState (
  IN EFI_USBFN_IO_PROTOCOL        *This,
  IN UINT8                        EndpointIndex,
  IN EFI_USBFN_ENDPOINT_DIRECTION Direction,
  IN BOOLEAN                      State
);


#endif
