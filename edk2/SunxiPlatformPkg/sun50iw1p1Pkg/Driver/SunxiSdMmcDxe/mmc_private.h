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


#ifndef _MMC_PRIVATE_H_
#define _MMC_PRIVATE_H_

#include <mmc.h>

extern int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
      struct mmc_data *data);
extern int mmc_send_status(struct mmc *mmc, int timeout);
extern int mmc_set_blocklen(struct mmc *mmc, int len);

#ifndef CONFIG_SPL_BUILD

extern unsigned long mmc_berase(int dev_num, lbaint_t start, lbaint_t blkcnt);

extern ulong mmc_bwrite(int dev_num, lbaint_t start, lbaint_t blkcnt,
    const void *src);

#else /* CONFIG_SPL_BUILD */

/* SPL will never write or erase, declare dummies to reduce code size. */

static inline unsigned long mmc_berase(int dev_num, lbaint_t start,
    lbaint_t blkcnt)
{
  return 0;
}

static inline ulong mmc_bwrite(int dev_num, lbaint_t start, lbaint_t blkcnt,
    const void *src)
{
  return 0;
}

#endif /* CONFIG_SPL_BUILD */

#endif /* _MMC_PRIVATE_H_ */
