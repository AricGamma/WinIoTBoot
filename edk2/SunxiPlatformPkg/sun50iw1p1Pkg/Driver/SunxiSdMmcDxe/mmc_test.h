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

#ifndef _MMC_TEST_H_
#define _MMC_TEST_H_

#ifdef MMC_INTERNAL_TEST
int mmc_t_rwc(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_erase(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_trim(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_discard(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_secure_erase(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_secure_trim(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_sanitize(struct mmc *mmc);
#endif /*MMC_INTERNAL_TEST*/

#endif /*_MMC_TEST_H_*/
