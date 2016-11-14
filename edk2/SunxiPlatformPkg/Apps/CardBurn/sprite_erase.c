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
#include "sprite_verify.h"

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
int sunxi_sprite_erase_flash(void  *img_mbr_buffer)
{
  int need_erase_flag = 0;
  //char buf[SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM];
  char* buf = NULL;
  int  ret;

  if(sunxi_sprite_erase(0, img_mbr_buffer) > 0)
  {
    printf("flash already erased\n");
    return 0;
  }
  //获取擦除信息，查看是否需要擦除flash
  ret = script_parser_fetch("platform", "eraseflag", &need_erase_flag, 1);
  if((!ret) && (need_erase_flag))
  {
    printf("do need erase flash\n");
  }
  else
  {
    printf("do not need erase flash\n");
    return 0;
  }
  //当要求强制擦除，不处理私有数据
  if(need_erase_flag == 0x11)
  {
    printf("force erase flash\n");
    sunxi_sprite_erase(1, img_mbr_buffer);
    return 0;
  }
  //检测不到private分区，即不用保护用户数据
  if(SunxiSpriteProbePrivatePartition(img_mbr_buffer) == FALSE)
  {
    printf("no part need to protect user data\n");
    sunxi_sprite_erase(need_erase_flag, img_mbr_buffer);
    return 0;
  }
  //当初始化失败的时候，直接擦除，不处理私有数据
  if(sunxi_sprite_init(1))
  {
    debug("sunxi sprite pre init fail, we have to erase it\n");
    sunxi_sprite_exit(1);
    sunxi_sprite_erase(need_erase_flag, img_mbr_buffer);
    return 0;
  }
  debug("nand pre init ok\n");
  buf = malloc(SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM);
  if(buf == NULL)
  {
    printf("malloc mbr buffer fail\n");
    return -1;
  }
  //读出量产介质上的MBR
  if(!sunxi_sprite_read(0, (SUNXI_MBR_SIZE * SUNXI_MBR_COPY_NUM)/512, buf))
  {
    printf("read local mbr on flash failed\n");
    sunxi_sprite_exit(1);
    sunxi_sprite_erase(need_erase_flag, img_mbr_buffer);
    free(buf);
    return 0;
  }
  //校验MBR
  if(SunxiSpriteVerifyMbr(buf) == FALSE)
  {
    printf("the mbr on flash is bad\n");
    sunxi_sprite_exit(1);
    sunxi_sprite_erase(need_erase_flag, img_mbr_buffer);
    free(buf);
    return 0;
  }
  printf("begin to store data\n");
  if(SunxiSpriteStorePrivateData(buf) == FALSE)
  {
    sunxi_sprite_exit(1);
    free(buf);
    return -1;
  }
  free(buf); buf = NULL;
    
  sunxi_sprite_exit(1);
  printf("need_erase_flag = %d\n", need_erase_flag);
  //开始擦除
  printf("begin to erase\n");
  sunxi_sprite_erase(need_erase_flag, img_mbr_buffer);
  //开始回写private
  printf("finish erase\n");
  sunxi_sprite_init(0);
  printf("rewrite\n");
  if(SunxiSpriteRestorePrivateData(img_mbr_buffer) == FALSE)
  {
    sunxi_sprite_exit(1);
    return -1;
  }
  printf("flash exit\n");
  //sunxi_sprite_exit(0);

  return 0;
}

