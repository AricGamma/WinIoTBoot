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

#ifndef  __USB_PRIVATEBURN_H__
#define  __USB_PRIVATEBURN_H__

#define SUNXI_PRIVATE_PART_NAME   ("private")
#define ALLWINNER_PRIVATEBURN_VERSION "0.1"

#define SUNXI_USB_PRIVATEBURN_DEV_MAX       (4)

#define SUNXI_PRIVATEBURN_DEVICE_MANUFACTURER       "AllWinner Technology"  /* 厂商信息   */
#define SUNXI_PRIVATEBURN_DEVICE_PRODUCT          "USB Mass Storage"      /* 产品信息   */
#define SUNXI_PRIVATEBURN_DEVICE_SERIAL_NUMBER        "20101201120001"        /* 产品序列号   */



/* String 0 is the language id */
#define SUNXI_PRIVATEBURN_DEVICE_STRING_MANUFACTURER_INDEX    1
#define SUNXI_PRIVATEBURN_DEVICE_STRING_PRODUCT_INDEX         2
#define SUNXI_PRIVATEBURN_DEVICE_STRING_SERIAL_NUMBER_INDEX   3


typedef enum _SUNXI_USB_PRIVATEBURN_STATE {      
  SunxiUsbPrivateBurnSetup =0,  
  SunxiUsbPrivateBurnSendData,   
  SunxiUsbPrivateBurnReceiveData, 
  SunxiUsbPrivateBurnResponse,    
  SunxiUsbPrivateBurnMaximum = SunxiUsbPrivateBurnResponse,
} SUNXI_USB_PRIVATEBURN_STATE;

typedef struct _SUNXI_PRIVATEBURN
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
  SUNXI_USB_PRIVATEBURN_STATE State;
  SUNXI_USB_PRIVATEBURN_STATE NextState;
  SUNXI_FLASH_IO_PROTOCOL*  FlashIo;
  EFI_USBFN_IO_PROTOCOL *UsbIo;
}
SUNXI_PRIVATEBURN;

typedef struct _USB_HANDSHAKE
{
  CHAR8  magic[16];         //特征字符串，固定是 "usbhandshake"，不足的填空
  INT32   sizelo;     //盘符的低32位，单位是扇区
  INT32   sizehi;     //盘符的高32位，单位是扇区
  INT32   res1;
  INT32   res2;
}
USB_HANDSHAKE;

#define PRIVATEBURN_CONTROL_SET_UP_BUFFER_SIZE   (1<<12)
#define PRIVATEBURN_DATA_SET_UP_BUFFER_SIZE    (1<<20)
#define PRIVATEBURN_DATA_RECIVE_BUFFER_SIZE      (1<<20)
#define PRIVATEBURN_DATA_SEND_BUFFER_SIZE        (64 * 1024)


#define  SUNXI_USB_PRIVATEBURN_BUFFER_MAX        (1<<20)

#define   DEVICE_VENDOR_ID            0xBB4
#define   DEVICE_PRODUCT_ID           0xfff
#define   DEVICE_BCD                  0x0200

#define PRIVATEBURN_CONTROL_ENDPOINT_INDEX    (0)
#define PRIVATEBURN_BULK_ENDPOINT_INDEX       (1)


/* Device and/or Interface Class codes */
#define USB_CLASS_PER_INTERFACE  0  /* for DeviceClass */
#define USB_CLASS_AUDIO          1
#define USB_CLASS_COMM           2
#define USB_CLASS_HID            3
#define USB_CLASS_PRINTER        7
#define USB_CLASS_MASS_STORAGE   8
#define USB_CLASS_HUB            9
#define USB_CLASS_DATA           10
#define USB_CLASS_VENDOR_SPEC    0xff

/* some HID sub classes */
#define USB_SUB_HID_NONE        0
#define USB_SUB_HID_BOOT        1

/* some UID Protocols */
#define USB_PROT_HID_NONE       0
#define USB_PROT_HID_KEYBOARD   1
#define USB_PROT_HID_MOUSE      2


/* Sub STORAGE Classes */
#define US_SC_RBC              1    /* Typically, flash devices */
#define US_SC_8020             2    /* CD-ROM */
#define US_SC_QIC              3    /* QIC-157 Tapes */
#define US_SC_UFI              4    /* Floppy */
#define US_SC_8070             5    /* Removable media */
#define US_SC_SCSI             6    /* Transparent */
#define US_SC_MIN              US_SC_RBC
#define US_SC_MAX              US_SC_SCSI

/* STORAGE Protocols */
#define US_PR_CB               1    /* Control/Bulk w/o interrupt */
#define US_PR_CBI              0    /* Control/Bulk/Interrupt */
#define US_PR_BULK             0x50   /* bulk only */


#define USBFN_PRIVATEBURN_DEBUG_ENABLE 0

#if(USBFN_DRIVER_DEBUG_ENABLE)
#define PRIVATEBURN_DEBUG_INFO(ARG...)  DEBUG (( EFI_D_INFO,"[PrivateBurn Info]:"ARG))
#define PRIVATEBURN_DEBUG_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[PrivateBurn Error]:"ARG))
#else
#define PRIVATEBURN_DEBUG_INFO(ARG...)  
#define PRIVATEBURN_DEBUG_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[PrivateBurn Error]:"ARG))
#endif

#define PRIVATEBURN_INFO(ARG...)  Print (L"[PrivateBurn Info]:"ARG)
#define PRIVATEBURN_ERROR(ARG...) Print (L"[PrivateBurn Error]:"ARG)

#endif

