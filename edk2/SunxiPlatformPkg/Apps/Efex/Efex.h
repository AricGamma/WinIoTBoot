/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Jerry Wang <wangflord@allwinnertech.com>
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

#ifndef  __USB_EFEX_H__
#define  __USB_EFEX_H__

#include <Protocol/SunxiFlashIo.h>
#include <IndustryStandard/Usb.h>
#include <Protocol/EfiUsbFnIo.h>

#define ALLWINNER_EFEX_VERSION "0.1"

#define CBW_MAGIC   0x43555741  //AWUC
#define CSW_MAGIC   0x53555741  //AWUS
#define CSW_STATUS_PASS 0x00
#define CSW_STATUS_FAIL 0x01

#define CBW_TOTAL_LEN   32  //
#define CBW_MAX_CMD_SIZE  16

#define FES_PLATFORM_HW_ID        0x00161000

#define AL_VERIFY_DEV_TAG_LEN     8
#define AL_VERIFY_DEV_TAG_DATA    "AWUSBFEX"

#define AL_VERIFY_DEV_MODE_NULL     0x00
#define AL_VERIFY_DEV_MODE_FEL      0x01
#define AL_VERIFY_DEV_MODE_SRV      0x02
#define AL_VERIFY_DEV_MODE_UPDATE_COOL  0x03
#define AL_VERIFY_DEV_MODE_UPDATE_HOT 0x04


#define SRAM_AREA_A           0x00000000

#define PHOENIX_PRIV_DATA_LEN_NR  8               //2的8次 = 256
#define PHOENIX_PRIV_DATA_ADDR      (SRAM_AREA_A + 0x7e00)      //给phoenix保留的空间
#define PHOENIX_PRIV_DATA_LEN     (1 << PHOENIX_PRIV_DATA_LEN_NR) //空间大小

#pragma pack(1)


//--hgl--传输层的命令
typedef struct _TRANSFERDATA
{
  UINT8 Direction;          ///
  UINT8 Reserved;           ///
  UINT32  DataLengh;          ///
  UINT8   Reserved2[10];          ///
}__attribute__ ((packed)) 
TRANSFERDATA;

typedef struct _SUNXI_EFEX_CBW
{
  UINT32  Magic;        //必须为CBW_MAGIC
  UINT32  Tag;
  UINT32  DataTransferLen;  //表示本次传输的数据阶段要传递的数据大小
  UINT16  Reserved_1;
  UINT8 Reserved_2;
  UINT8 CmdLengh;     //cmd_package的实际有效长度
  TRANSFERDATA  CmdPackage;
}__attribute__ ((packed))
SUNXI_EFEX_CBW;

#define CSW_TOTAL_LEN 13

typedef struct _SUNXI_EFEX_CSW
{
  UINT32  Magic;    //必须为CSW_MAGIC
  UINT32  Tag;
  UINT32  Residue;    //没有发送/接收的数据长度
  UINT8 Status;   //为CSW_STATUS_PASS或 CSW_STATUS_FAIL
}__attribute__ ((packed))
SUNXI_EFEX_CSW;


#define TL_CMD_RESERVED     0x00
#define TL_CMD_TRANSMIT     0x11
#define TL_CMD_RECEIVE    0x12


#define TRANSPORT_INFO_STATUS_NULL  0x00
#define TRANSPORT_INFO_STATUS_CBW 0x01
#define TRANSPORT_INFO_STATUS_DATA  0x02
#define TRANSPORT_INFO_STATUS_CSW 0x03


#define TRANSPORT_INFO_IS_CONNECT_FAIL  0x00
#define TRANSPORT_INFO_IS_CONNECT_TRUE  0x01


//==app layer之公共命令
#define APP_LAYER_COMMEN_CMD_VERIFY_DEV     0x0001
#define APP_LAYER_COMMEN_CMD_SWITCH_ROLE    0x0002
#define APP_LAYER_COMMEN_CMD_IS_READY     0x0003
#define APP_LAYER_COMMEN_CMD_GET_CMD_SET_VER  0x0004
#define APP_LAYER_COMMEN_CMD_DISCONNECT     0x0010

#define FEX_CMD_fes_trans           0x0201
#define FEX_CMD_fes_run             0x0202
#define FEX_CMD_fes_down            0x0206
#define FEX_CMD_fes_up                0x0207
#define FEX_CMD_fes_verify              0x0208
#define FEX_CMD_fes_query_storage       0x0209
#define FEX_CMD_fes_probe_hardware        0x020A
#define FEX_CMD_fes_flash_set_on        0x020A
#define FEX_CMD_fes_flash_set_off       0x020B
#define FEX_CMD_fes_verify_value          0x020C
#define FEX_CMD_fes_verify_status         0x020D
#define FEX_CMD_fes_flash_size_probe      0x020E
#define FEX_CMD_fes_tool_mode         0x020F
#define FEX_CMD_fes_memset                      0x0210
#define FEX_CMD_fes_pmu                         0x0211
#define FEX_CMD_fes_unseqmem_read         0x0212
#define FEX_CMD_fes_unseqmem_write        0x0213
#define FEX_CMD_fes_query_secure          0x0230

//各个app命令的cmd,data部分，status部分是共用的

//====================verify_dev====================

typedef struct _GLOBAL_CMD
{
  UINT16  AppCmd;         //
  UINT16  Tag;
}__attribute__ ((packed))
GLOBAL_CMD;

typedef struct _VERIFY_DEV_CMD
{
  UINT16  AppCmd;         //必须为APP_LAYER_COMMEN_CMD_VERIFY_DEV
  UINT16  Tag;
  UINT8 Reserved[12];
}__attribute__ ((packed))
VERIFY_DEV_CMD;

typedef struct _VERIFY_DEV_DATA
{
  UINT8  Tag[AL_VERIFY_DEV_TAG_LEN];  //必须为AL_VERIFY_DEV_TAG_DATA，用来区分
  UINT32 PlatformHwId;
  UINT32 PlatformFwId;
  UINT16 Mode;            //如AL_VERIFY_DEV_MODE_NULL
  UINT8  PhoenixDataFlag;
  UINT8  PhoenixDataLengh;        //
  UINT32 PhoenixDataStartAddress;   //phoenix data的 start addr
  UINT8  Reserved[8];
}__attribute__ ((packed))
VERIFY_DEV_DATA;

typedef struct _SWITCH_ROLE_CMD{
  UINT16  AppCmd;         //必须为APP_LAYER_COMMEN_CMD_SWITCH_ROLE
  UINT16  State;            //如AL_VERIFY_DEV_MODE_FEL
  UINT8 Reserved[12];
}SWITCH_ROLE_CMD;

typedef struct _IS_READY_CMD{
  UINT16  AppCmd;         //必须为APP_LAYER_COMMEN_CMD_IS_READY
  UINT16  State;            //目标state,如AL_VERIFY_DEV_MODE_FEL
  UINT8   Reserved[12];
}__attribute__ ((packed))
IS_READY_CMD;

#define AL_IS_READY_STATE_NULL    0x00  //
#define AL_IS_READY_STATE_BUSY    0x01  //忙，请等待
#define AL_IS_READY_STATE_READY   0x02  //就绪
#define AL_IS_READY_STATE_FAIL      0x03  //失败

typedef struct _IS_READY_DATA
{
  UINT16  State;            //当前所处的状态，如AL_IS_READY_STATE_READY
  UINT16  IntervalMs;         //下次发送is_ready命令的延时，单位为ms
  //推荐为500 ~2000 ，该字段只有当
  //state == AL_IS_READY_STATE_BUSY时候才有效
  UINT8 Reserved[12];
}__attribute__ ((packed))
IS_READY_DATA;

typedef struct _GET_CMD_SET_VER_CMD
{
  UINT16  AppCmd;     //必须为APP_LAYER_COMMEN_CMD_GET_CMD_SET_VER
  UINT16  Tag;
  UINT16  State;        //要查询cmd_set的state
  UINT8 Reserved[10];
}__attribute__ ((packed))
GET_CMD_SET_VER_CMD;

typedef struct _GET_CMD_SET_VER_DATA
{
  UINT16  VersionHigh;    //version的高word部分
  UINT16  VersionLow;   //version的低word部分
  UINT8 Reserved[12];

}__attribute__ ((packed))
GET_CMD_SET_VER_DATA;

//====================disconnect====================
typedef struct _DISCONNECT_CMD
{
  UINT16  AppCmd;
  UINT16  Tag;
  UINT8 Reserved[12];
}__attribute__ ((packed))
DISCONNECT_CMD;

typedef struct _SUNXI_EFEX_STATUS
{
  UINT16  Mark;           ///0xffff
  UINT16  Tag;            ///
  INT8  State;            ///STATUS_SUCCESS
  INT8  Rev[3];           ///
}SUNXI_EFEX_STATUS;

typedef struct _FES_TRANS
{
  UINT16  AppCmd;
  UINT16  Tag;
  UINT32  Address;        ///
  UINT32  Length;       ///
  UINT32  Type;
}FES_TRANS;

typedef struct _FES_TRANS_OLD
{
  UINT16 AppCmd;
  UINT16 Tag;
  UINT32 Address;       ///
  UINT32 Length;        ///
  struct
  {
    UINT8 LogicUnitIndex : 4; ///低4比特
    UINT8 MediaIndex     : 4; ///高4比特
  }U1;
  struct
  {
    UINT8 Res   : 4;  ///低4比特
    UINT8 DOU   : 2;  ///中间2比特 标识 Download Or Upload
    UINT8 OOC     : 2;  ///高2比特
  }U2;
  UINT8 Reserved[2];    ///
}FES_TRANS_OLD;

typedef struct _FES_CMD_VERIFY_VALUE
{
  UINT16     Cmd;
  UINT16     Tag;
  UINT32     Start;
  INT64    Size;
}FES_CMD_VERIFY_VALUE;

typedef struct _FES_CMD_VERIFY_STATUS
{
  UINT16     Cmd;
  UINT16     Tag;
  UINT32     Start;
  UINT32     Size;
  UINT32     DataTag;
}FES_CMD_VERIFY_STATUS;

#define  EFEX_CRC32_VALID_FLAG   (0x6a617603)

typedef  struct  _FES_EFEX_VERIFY
{
  UINT32  Flag;       //标志crc计算完成, 固定为0x6a617603

  INT32  FesCrc;      //fes接收数据的crc
  INT32  MediaCrc;         //media数据的crc
}FES_EFEX_VERIFY;

typedef struct _FES_RUN
{
  UINT16  AppCmd;
  UINT16  Tag;
  UINT32  CodeAddress;        ///
  INT32   CodeSize;
  INT32  *ParaAddr;
}FES_RUN;

typedef  struct  _EFEX_TOOL
{
  UINT16  Cmd;
  UINT16  Tag;
  UINT32  ToolMode;

  INT32  NextMode;
  INT32  Res0;
}EFEX_TOOL;

typedef  struct  _FES_EFEX_MEMSET
{
  UINT16  Cmd;
  UINT16  Tag;

  UINT32  StartAddress;
  UINT32  Length;
  INT32   Value;
}FES_EFEX_MEMSET;

typedef  struct  _FES_EFEX_PMU
{
  UINT16  Cmd;
  UINT16  Tag;

  UINT32  Size;   //
  INT32   Res1;      //
  INT32   Type;
}FES_EFEX_PMU;

typedef struct _PMU_CONFIG
{
  INT8 PmuType[16];
  INT8 VoltageName[16];
  UINT32  Voltage;
  UINT32  Gate;
}PMU_CONFIG;

typedef  struct  _EFEX_UNSEQ_MEM
{
  UINT16  Cmd;
  UINT16   Tag;

  UINT32   Size;        ///
  UINT32   Count;     ///
  UINT32   Type;
}EFEX_UNSEQ_MEM;


typedef struct _UNSEQ_MEM_CONFIG
{
  UINT32  Address;
  UINT32  Value;
}UNSEQ_MEM_CONFIG;


typedef struct _MULTI_UNSEQ_MEM
{
  UINT32 Count;

  UNSEQ_MEM_CONFIG *UnseqMemory;
}MULTI_UNSEQ_MEM;


#define SUNXI_EFEX_RECV_MEM_SIZE  (1024 * 1024)

#pragma pack()

typedef enum _SUNXI_USB_EFEX_STATE {      
  SunxiUsbEfexIdle      =0,  
  SunxiUsbEfexSetup     =1,  
  SunxiUsbEfexSendData    =2,   
  SunxiUsbEfexReceiveData   =3, 
  SunxiUsbEfexStatus      =4,    
  SunxiUsbEfexExit      =5, 
  SunxiUsbEfexSetupNew    =11,
  SunxiUsbEfexSendDataNew   =12,
  SunxiUsbEfexReceiveDataNew  =13,
} SUNXI_USB_EFEX_STATE;

typedef enum _SUNXI_USB_EFEX_APP_STATE {      
  SunxiUsbEfexAppIdle     =0x10000,  
  SunxiUsbEfexAppCmd      =0x20000,  
  SunxiUsbEfexAppData       =0x30000,
  SunxiUsbEfexAppSendData   =SunxiUsbEfexAppData|SunxiUsbEfexSendData,
  SunxiUsbEfexAppReceiveData  =SunxiUsbEfexAppData|SunxiUsbEfexReceiveData,
  SunxiUsbEfexAppStatus   =(0x40000)|SunxiUsbEfexStatus,
  SunxiUsbEfexAppExit     =(0x50000)|SunxiUsbEfexExit,
} SUNXI_USB_EFEX_APP_STATE;


#define  SUNXI_USB_EFEX_IDLE           (0)
#define  SUNXI_USB_EFEX_SETUP          (1)
#define  SUNXI_USB_EFEX_SEND_DATA        (2)
#define  SUNXI_USB_EFEX_RECEIVE_DATA       (3)
#define  SUNXI_USB_EFEX_STATUS           (4)
#define  SUNXI_USB_EFEX_EXIT           (5)

#define  SUNXI_USB_EFEX_SETUP_NEW                (11)
#define  SUNXI_USB_EFEX_SEND_DATA_NEW            (12)
#define  SUNXI_USB_EFEX_RECEIVE_DATA_NEW         (13)
#define  FES_NEW_CMD_LEN                         (20)


#define  SUNXI_USB_EFEX_APPS_MAST        (0xf0000)

#define  SUNXI_USB_EFEX_APPS_IDLE        (0x10000)
#define  SUNXI_USB_EFEX_APPS_CMD         (0x20000)
#define  SUNXI_USB_EFEX_APPS_DATA        (0x30000)
#define  SUNXI_USB_EFEX_APPS_SEND_DATA         (SUNXI_USB_EFEX_APPS_DATA | SUNXI_USB_EFEX_SEND_DATA)
#define  SUNXI_USB_EFEX_APPS_RECEIVE_DATA    (SUNXI_USB_EFEX_APPS_DATA | SUNXI_USB_EFEX_RECEIVE_DATA)
#define  SUNXI_USB_EFEX_APPS_STATUS        ((0x40000)  | SUNXI_USB_EFEX_STATUS)
#define  SUNXI_USB_EFEX_APPS_EXIT        ((0x50000)  | SUNXI_USB_EFEX_EXIT)

#define EFEX_CONTROL_SET_UP_BUFFER_SIZE    (1<<12)
#define EFEX_DATA_SET_UP_BUFFER_SIZE     (1<<12)
#define EFEX_DATA_RECEIVE_BUFFER_SIZE        (8<<20)
#define EFEX_DATA_SEND_BUFFER_SIZE           (1<<20)

typedef struct _EFEX_TRANSFER
{
  UINT32   DataType;      //存储内型，见下面的宏定义
  UINT8   *BaseReceiveBuffer;   //存放接收到的数据
  UINT8   *ActualReceiveBuffer;
  UINT64   TryToReceiveBytes;      //bytes that pc try to sent/target try to recive
  UINT64   ActuallyReceivedBytes;    //Actually RecivedBytes
  UINT8   *BaseSendBuffer;       //存放预发送数据
  UINT8   *ActualSendBuffer;
  UINT64   SizeNeedToBeSent;        //需要发送数据的长度
  UINT8   *ControlSetUpBuffer;
  UINT8   *DataSetUpBuffer;
  UINT32   FlashStart;      //起始位置，可能是内存，也可能是flash扇区
  UINT32   FlashSectors;
  UINT32   DramTransferBuffer;
  INT32    LastError;
  SUNXI_USB_EFEX_APP_STATE AppNextState;
}
EFEX_TRANSFER;

typedef struct _SUNXI_EFEX
{

  SUNXI_USB_EFEX_STATE   EfexState;
  SUNXI_USB_EFEX_APP_STATE AppState;
  EFEX_TRANSFER      Transfer;
  SUNXI_FLASH_IO_PROTOCOL *FlashIo;
  EFI_USBFN_IO_PROTOCOL   *UsbIo;
  UINT32           NextAction;      
}
SUNXI_EFEX;

#define  SUNXI_EFEX_DATA_TYPE_MASK    (0x7fff)
#define  SUNXI_EFEX_DRAM_MASK     (0x7f00)
#define  SUNXI_EFEX_DRAM_TAG      (0x7f00)
#define  SUNXI_EFEX_MBR_TAG       (0x7f01)
#define  SUNXI_EFEX_BOOT1_TAG     (0x7f02)
#define  SUNXI_EFEX_BOOT0_TAG     (0x7f03)
#define  SUNXI_EFEX_ERASE_TAG           (0x7f04)
#define  SUNXI_EFEX_PMU_SET             (0x7f05)
#define  SUNXI_EFEX_UNSEQ_MEM_FOR_READ  (0x7f06)
#define  SUNXI_EFEX_UNSEQ_MEM_FOR_WRITE (0x7f07)

#define  SUNXI_EFEX_FLASH_TAG           (0x8000)

#define  SUNXI_EFEX_TRANS_MASK      (0x30000)
#define  SUNXI_EFEX_TRANS_START_TAG   (0x20000)
#define  SUNXI_EFEX_TRANS_FINISH_TAG  (0x10000)

#define  SUNXI_EFEX_VERIFY_STATUS   (0)
#define  SUNXI_EFEX_VERIFY_ADDSUM   (1)
#define  SUNXI_EFEX_VERIFY_CRC32    (2)

#define EFEX_CONTROL_ENDPOINT_INDEX   (0)
#define EFEX_BULK_ENDPOINT_INDEX    (1)

#define _EFEX_USE_BUF_QUEUE_
#define USBFN_EFEX_DEBUG_ENABLE 0

#if (USBFN_EFEX_DEBUG_ENABLE)
#define EFEX_DEBUG_INFO(ARG...)  DEBUG (( EFI_D_INFO,"[Efex Info]:"ARG))
#define EFEX_DEBUG_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[Efex Error]:"ARG))
#else
#define EFEX_DEBUG_INFO(ARG...)  
#define EFEX_DEBUG_ERROR(ARG...) DEBUG (( EFI_D_ERROR,"[Efex Error]:"ARG))
#endif

#define EFEX_INFO(ARG...)  Print (L"[Efex Info]:"ARG)
#define EFEX_ERROR(ARG...) Print (L"[Efex Error]:"ARG)

#endif

