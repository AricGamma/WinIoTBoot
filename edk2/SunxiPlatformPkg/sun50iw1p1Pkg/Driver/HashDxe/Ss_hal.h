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

#ifndef   _SS_HAL_H   
#define   _SS_HAL_H


//#define SS_BASE       0x01C15000

#define SS_CTL        (SS_BASE + 0x00)
#define SS_KEY        (SS_BASE + 0x04)  
#define SS_IV         (SS_BASE + 0x24)
#define SS_CNT        (SS_BASE + 0x34)  
#define SS_FCSR       (SS_BASE + 0x44)  
#define SS_ICSR       (SS_BASE + 0x48)
#define SS_MD       (SS_BASE + 0x4C)
#define SS_CTS_LEN      (SS_BASE + 0x60)
#define SS_RXFIFO     (SS_BASE + 0x200)
#define SS_TXFIFO     (SS_BASE + 0x204)

#define ss_read_w(n)      MmioRead32(n)          
#define ss_write_w(n,c)   MmioWrite32(n,c)

//macro define for SS IO operation
#define SSIO_SET_KEY          0x00
#define SSIO_SET_IV           0x01
#define SSIO_SET_CNT          0x02
#define SSIO_SETUP_AES_DES        0x03
#define SSIO_SETUP_SHA1_MD5       0x04
#define SSIO_START                  0x05
#define SSIO_STOP                   0x06
#define SSIO_ENABLE_DMA             0x07
#define SSIO_DISABLE_DMA            0x08
#define SSIO_TX_INT                 0x09
#define SSIO_RX_INT                 0x0a
#define SSIO_TX_TRIGGER_LEVEL       0x0b
#define SSIO_RX_TRIGGER_LEVEL       0x0c
#define SSIO_SETUP_RANDOM       0x0d
#define SSIO_RANDOM_ONESHOT_START   0x0e
#define SSIO_KEY_SELECT         0x0f
#define SSIO_SET_CTS_LENGTH             0x10
#define SSIO_GET_IV                 0x11




extern void ss_init(void);
extern void ss_exit(void);
extern void ss_start(void);
extern UINT32 ss_io_ctl(UINT32 io_type, UINT32* io_data);
extern UINT32 ss_get_rxfifo_room_size(void);
extern UINT32 ss_get_txfifo_data_cnt(void);
extern void ss_send_data(UINT32 data);
extern void ss_receive_data(UINT32* data);
extern void ss_sha1md5_dataend(void);
extern void ss_get_md(UINT32* md_buf);
extern void ss_send_text_to_sha1md5(UINT32 data_size, UINT32 total_size, UINT32 last, UINT32* text);
extern void ss_sha1md5_text(UINT32 total_size, UINT32* in);
extern UINT32 sha1md5_padding(UINT32 md5_flag, UINT32 data_size, UINT8* text);
extern void ss_debug(void);

extern void ss_check_randomend(void);
extern void ss_get_random(UINT32* random_buf);

#endif  //_SS_HAL_H
