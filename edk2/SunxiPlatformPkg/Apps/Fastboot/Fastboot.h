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

#ifndef  __USB_FASTBOOT_H__
#define  __USB_FASTBOOT_H__

#define ALLWINNER_FASTBOOT_VERSION "0.5"

#define  SUNXI_USB_FASTBOOT_DEV_MAX       (6)

#define SUNXI_FASTBOOT_DEVICE_MANUFACTURER        "USB Developer"     /* 厂商信息   */
#define SUNXI_FASTBOOT_DEVICE_PRODUCT         "Android Fastboot"    /* 产品信息   */
#define SUNXI_FASTBOOT_DEVICE_SERIAL_NUMBER       "20080411"        /* 产品序列号   */
#define SUNXI_FASTBOOT_DEVICE_CONFIG              "Android Fastboot"
#define SUNXI_FASTBOOT_DEVICE_INTERFACE           "Android Bootloader Interface"

/* String 0 is the language id */
#define SUNXI_FASTBOOT_DEVICE_STRING_PRODUCT_INDEX        1
#define SUNXI_FASTBOOT_DEVICE_STRING_SERIAL_NUMBER_INDEX  2
#define SUNXI_FASTBOOT_DEVICE_STRING_CONFIG_INDEX         3
#define SUNXI_FASTBOOT_DEVICE_STRING_INTERFACE_INDEX      4
#define SUNXI_FASTBOOT_DEVICE_STRING_MANUFACTURER_INDEX   5

typedef enum _SUNXI_USB_FASTBOOT_STATE {      
  SunxiUsbFastbootSetup =0,  
  SunxiUsbFastbootSendData,   
  SunxiUsbFastbootReceiveData, 
  SunxiUsbFastbootResponse,    
  SunxiUsbFastbootMaximum = SunxiUsbFastbootResponse,
} SUNXI_USB_FASTBOOT_STATE;

typedef struct _SUNXI_FASTBOOT
{
  UINT8 *BaseReceiveBuffer;   //存放接收到的数据
//  UINT8 *ActualReceiveBuffer;
  UINT64  TryToReceiveBytes;      //bytes that pc try to sent/target try to recive
  UINT64  ActuallyReceivedBytes;    //Actually RecivedBytes
  UINT8 *BaseSendBuffer;       //存放预发送数据
//  UINT8 *ActualSendBuffer;
//  UINT64  SizeNeedToBeSent;       //需要发送数据的长度
  UINT8 *ControlSetUpBuffer;
  UINT8 *DataSetUpBuffer;
//  UINTN IsDataTransferState;
  SUNXI_USB_FASTBOOT_STATE State;
  SUNXI_USB_FASTBOOT_STATE NextState;
  SUNXI_FLASH_IO_PROTOCOL*  FlashIo;
  EFI_USBFN_IO_PROTOCOL *UsbIo;
}
SUNXI_FASTBOOT;


#define FASTBOOT_CONTROL_SET_UP_BUFFER_SIZE    (1<<12)
#define FASTBOOT_DATA_SET_UP_BUFFER_SIZE       (1<<20)
#define FASTBOOT_DATA_RECIVE_BUFFER_SIZE         (32<<20)
#define FASTBOOT_DATA_SEND_BUFFER_SIZE           (64 * 1024)
#define FASTBOOT_ERASE_BUFFER_SIZE               (1 <<20)

#define  SUNXI_USB_FASTBOOT_BUFFER_MAX               (128 * 1024 * 1024)

#define   DEVICE_VENDOR_ID            0x1F3A
#define   DEVICE_PRODUCT_ID           0x1010
#define   DEVICE_BCD                  0x0200

#define FASTBOOT_CONTROL_ENDPOINT_INDEX   (0)
#define FASTBOOT_BULK_ENDPOINT_INDEX    (1)

#define USBFN_FASTBOOT_DEBUG_ENABLE 1

#if(USBFN_DRIVER_DEBUG_ENABLE)
#define FASTBOOT_DEBUG_INFO(ARG...)  DEBUG (( EFI_D_INFO,"[Fastboot Info]:"ARG))
#define FASTBOOT_DEBUG_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[Fastboot Error]:"ARG))
#else
#define FASTBOOT_DEBUG_INFO(ARG...)  
#define FASTBOOT_DEBUG_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[Fastboot Error]:"ARG))
#endif

#define FASTBOOT_INFO(ARG...)  Print (L"[Fastboot Info]:"ARG)
#define FASTBOOT_ERROR(ARG...) Print (L"[Fastboot Error]:"ARG)

#endif

