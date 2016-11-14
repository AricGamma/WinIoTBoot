/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  tangmanliang <tangmanliang@allwinnertech.com>
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

#ifndef _SUNXI_LINUX_HEAD_H_
#define _SUNXI_LINUX_HEAD_H_

/* Android bootimage file format */
#define FASTBOOT_BOOT_MAGIC "ANDROID!"
#define FASTBOOT_BOOT_MAGIC_SIZE 8
#define FASTBOOT_BOOT_NAME_SIZE 16
#define FASTBOOT_BOOT_ARGS_SIZE 512

struct fastboot_boot_img_hdr {
  unsigned char magic[FASTBOOT_BOOT_MAGIC_SIZE];

  unsigned int kernel_size;  /* size in bytes */
  unsigned int kernel_addr;  /* physical load addr */

  unsigned int ramdisk_size; /* size in bytes */
  unsigned int ramdisk_addr; /* physical load addr */

  unsigned int second_size;  /* size in bytes */
  unsigned int second_addr;  /* physical load addr */

  unsigned int tags_addr;    /* physical addr for kernel tags */
  unsigned int page_size;    /* flash page size we assume */
  unsigned int unused[2];    /* future expansion: should be 0 */

  unsigned char name[FASTBOOT_BOOT_NAME_SIZE]; /* asciiz product name */

  unsigned char cmdline[FASTBOOT_BOOT_ARGS_SIZE];

  unsigned int id[8]; /* timestamp / checksum / sha1 / etc */
};


EFI_STATUS
SunxiBdsBootLinuxAtag (
  IN EFI_PHYSICAL_ADDRESS       LinuxImage,
  IN  CONST CHAR8*              CommandLineArguments
  );

#endif
