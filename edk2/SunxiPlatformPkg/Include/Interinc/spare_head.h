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


#ifndef  __spare_head_h__
#define  __spare_head_h__

/* work mode */
#define WORK_MODE_PRODUCT      (1<<4)
#define WORK_MODE_UPDATE       (1<<5)

#define WORK_MODE_BOOT      0x00  //Õý³£Æô¶¯
#define WORK_MODE_USB_PRODUCT 0x10  //ÓÃÓÚUSBÁ¿²ú
#define WORK_MODE_CARD_PRODUCT  0x11  //ÓÃÓÚ¿¨Á¿²ú
#define WORK_MODE_USB_DEBUG     0x12    //ÀûÓÃusbÁ¿²úÐ­ÒéÍê³ÉµÄ²âÊÔ
#define WORK_MODE_USB_UPDATE  0x20  //ÓÃÓÚUSBÉý¼¶
#define WORK_MODE_OUTER_UPDATE  0x21  //ÓÃÓÚÍâ²¿ÅÌÉý¼¶

#define WORK_MODE_USB_TOOL_PRODUCT  0x04  //ÓÃÓÚÁ¿²ú
#define WORK_MODE_USB_TOOL_UPDATE 0x08  //ÓÃÓÚÉý¼¶

#define UEFI_VERSION      "2.3.1"
#define UEFI_PLATFORM       "sunxi"
/* Mike update, 05/17/2016 */
//#define UEFI_FD_BASE_ADDR     0x2A000000
#define UEFI_FD_BASE_ADDR     0x4A000000

#define UEFI_MAGIC        "uboot"
#define STAMP_VALUE             0x5F0A6C39
#define ALIGN_SIZE        16 * 1024
#define MAGIC_SIZE              8
#define STORAGE_BUFFER_SIZE     (256)

#define SUNXI_UPDATE_NEXT_ACTION_NORMAL     (1)
#define SUNXI_UPDATE_NEXT_ACTION_REBOOT     (2)
#define SUNXI_UPDATE_NEXT_ACTION_SHUTDOWN   (3)
#define SUNXI_UPDATE_NEXT_ACTION_REUPDATE   (4)
#define SUNXI_UPDATE_NEXT_ACTION_BOOT     (5)

//#define BOOT0_SDMMC_START_ADDR                  (16)
#define BOOT0_SDMMC_START_ADDR                  (256)
//#define UBOOT_START_SECTOR_IN_SDMMC             (38192)
#define UBOOT_START_SECTOR_IN_SDMMC             (512)
#define BOOT0_EMMC3_START_ADDR                  (384)
#define BOOT0_EMMC3_BACKUP_START_ADDR           (512)
//#define CONFIG_MMC_LOGICAL_OFFSET               (20 * 1024 * 1024/512)
#define CONFIG_MMC_LOGICAL_OFFSET               (0)
#define MMC_LOGICAL_OFFSET                      CONFIG_MMC_LOGICAL_OFFSET

#define SUNXI_USB_MASS_PART_NUM 2
#define SUNXI_USB_MASS_GPT_PART_NUM 3
#define SUNXI_GPT_PRIMARY_HEADER_LBA 1

typedef struct _normal_gpio_cfg
{
  char      port;                       //¶Ë¿ÚºÅ
  char      port_num;                   //¶Ë¿ÚÄÚ±àºÅ
  char      mul_sel;                    //¹¦ÄÜ±àºÅ
  char      pull;                       //µç×è×´Ì¬
  char      drv_level;                  //Çý¶¯Çý¶¯ÄÜÁ¦
  char      data;                       //Êä³öµçÆ½
  char      reserved[2];                //±£ÁôÎ»£¬±£Ö¤¶ÔÆë
}
normal_gpio_cfg;

typedef struct _special_gpio_cfg
{
  unsigned char   port;       //¿¿¿¿?
  unsigned char   port_num;     //¿¿¿¿¿¿¿?
  char        mul_sel;      //¿¿¿¿¿¿
  char        data;       //¿¿¿¿¿¿
}special_gpio_cfg;

//SD¿¨Ïà¹ØÊý¾Ý½á¹¹
typedef struct sdcard_spare_info_t
{
  int       card_no[4];                   //µ±Ç°Æô¶¯µÄ¿¨¿ØÖÆÆ÷±àºÅ
  int       speed_mode[4];                //¿¨µÄËÙ¶ÈÄ£Ê½£¬0£ºµÍËÙ£¬ÆäËü£º¸ßËÙ
  int       line_sel[4];                  //¿¨µÄÏßÖÆ£¬0: 1Ïß£¬ÆäËü£¬4Ïß
  int       line_count[4];                //¿¨Ê¹ÓÃÏßµÄ¸öÊý
}
sdcard_spare_info;

typedef enum
{
  STORAGE_NAND =0,
  STORAGE_SD,
  STORAGE_EMMC,
  STORAGE_NOR,
        STORAGE_EMMC3
}SUNXI_BOOT_STORAGE;

#endif


