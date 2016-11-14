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

#ifndef _MMC_OP_H_
#define _MMC_OP_H_

//物理操作
extern INT32 SDMMC_PhyInit(UINT32 card_no);
extern INT32 SDMMC_PhyExit(UINT32 card_no);

extern INT32 SDMMC_PhyRead     (UINT32 start_sector, UINT32 nsector, void *buf, UINT32 card_no);
extern INT32 SDMMC_PhyWrite    (UINT32 start_sector, UINT32 nsector, void *buf, UINT32 card_no);

//逻辑操作
extern INT32 SDMMC_LogicalInit(UINT32 card_no, UINT32 card_offset);
extern INT32 SDMMC_LogicalExit(UINT32 card_no);

extern INT32 SDMMC_LogicalRead (UINT32 start_sector, UINT32 nsector, void *buf, UINT32 card_no);
extern INT32 SDMMC_LogicalWrite(UINT32 start_sector, UINT32 nsector, void *buf, UINT32 card_no);
extern INT32 SDMMC_BootFileWrite(UINT32 start_sector, UINT32 nblock,void *buf, UINT32 card_no);
extern lbaint_t SDMMC_LogicalLBA(UINT32 card_no);

#endif /* _MMC_H_ */
