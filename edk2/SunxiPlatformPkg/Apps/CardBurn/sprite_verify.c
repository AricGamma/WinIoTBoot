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


#include "../CardBurn.h"
#include "sparse/sparse.h"
#include "../SunxiSprite.h"
#include <Library/SunxiMbr.h>
#include <Library/SunxiCheckLib.h>


/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
uint sunxi_sprite_part_sparsedata_verify(void)
{
  return unsparse_checksum();
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_sprite_verify_dlmap(void *buffer)
{
  sunxi_download_info *local_dlmap;
  char        *tmp_buf = (char *)buffer;

  tmp_buf = buffer;
  local_dlmap = (sunxi_download_info *)tmp_buf;
  if(SunxiCrc32((const unsigned char *)(tmp_buf + 4), SUNXI_MBR_SIZE - 4) != local_dlmap->crc32)
  {
    printf("downlaod map is bad\n");
    return -1;
  }

  return 0;
}

