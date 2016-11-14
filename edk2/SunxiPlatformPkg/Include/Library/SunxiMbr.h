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


#ifndef  __SUNXI_MBR_H__
#define  __SUNXI_MBR_H__

#include <IndustryStandard/Mbr.h>
#include <Uefi/UefiBaseType.h>

#define  SUNXI_MBR_OFFSET         (40960)
#define  GET_BLOCK_ADDR(addr)     (SUNXI_MBR_OFFSET + (addr))

#define     DOWNLOAD_MAP_NAME   "dlinfo.fex"
#define     SUNXI_MBR_NAME      "sunxi_mbr.fex"
/* MBR       */
#define     SUNXI_MBR_SIZE          (16 * 1024)
#define     SUNXI_DL_SIZE       (16 * 1024)
#define     SUNXI_MBR_MAGIC         "softw411"
#define     SUNXI_MBR_MAX_PART_COUNT  120
#define     SUNXI_MBR_COPY_NUM          4    //mbrµÄ±¸·ÝÊýÁ¿
#define     SUNXI_MBR_RESERVED          (SUNXI_MBR_SIZE - 32 - 4 - (SUNXI_MBR_MAX_PART_COUNT * sizeof(sunxi_partition)))   //mbr±£ÁôµÄ¿Õ¼ä
#define     SUNXI_DL_RESERVED           (SUNXI_DL_SIZE - 32 - (SUNXI_MBR_MAX_PART_COUNT * sizeof(dl_one_part_info)))

#define     SUNXI_NOLOCK                (0)
#define     SUNXI_LOCKING               (0xAA)
#define     SUNXI_RELOCKING             (0xA0)
#define     SUNXI_UNLOCK                (0xA5)

/* GPT */
#define     SUNXI_GPT_SIZE          (16 * 1024)
#define     SUNXI_GPT_MAGIC         "softw411"
#define     SUNXI_GPT_MAX_PART_COUNT  120
#define     SUNXI_GPT_COPY_NUM          4    //GPT¿¿¿¿¿
#define     SUNXI_GPT_RESERVED          (SUNXI_GPT_SIZE - 32 - (SUNXI_GPT_MAX_PART_COUNT * sizeof(gpt_partition)))   //mbr¿¿¿¿¿


/* partition information */
typedef struct gpt_partition_t
{
  unsigned  int       addrhi;       //¿¿¿¿, ¿¿¿¿¿¿
  unsigned  int       addrlo;       //
  unsigned  int       lenhi;        //¿¿
  unsigned  int       lenlo;        //
  unsigned  char      classname[16];    //¿¿¿¿
  unsigned  char      name[16];     //¿¿¿¿
  unsigned  int       user_type;          //¿¿¿¿
  unsigned  int       keydata;            //¿¿¿¿¿¿¿¿¿¿¿¿
  unsigned  int       ro;                 //¿¿¿¿
        EFI_GUID      type;
  EFI_GUID      uniqueguid;
  unsigned  int     attrhi;       //¿¿¿¿
  unsigned  int     attrlo;
  unsigned  char      reserved[28];   //¿¿¿¿¿¿¿¿¿¿¿128¿¿
} //__attribute__ ((packed))sunxi_partition;
gpt_partition;

/* gpt information */
typedef struct sunxi_gpt
{
  unsigned  int       crc32;                // crc 1k - 4
  unsigned  int       version;              // ¿¿¿¿¿ 0x00000100
  unsigned  char      magic[8];             //"softw311"
  unsigned  int       copy;               //¿¿
  unsigned  int       index;                //¿¿¿gpt¿¿
  unsigned  int       PartCount;              //¿¿¿¿
  unsigned  int       stamp[1];         //¿¿
  gpt_partition       array[SUNXI_GPT_MAX_PART_COUNT];  //
  unsigned  char      res[SUNXI_GPT_RESERVED];
}//__attribute__ ((packed)) sunxi_mbr_t;
sunxi_gpt_t;

/* GPT table info */
typedef struct gpt_table_info
{
  EFI_LBA MyLBA;  
  EFI_LBA AlternateLBA; 
  EFI_LBA FirstLBA;
  EFI_LBA LastLBA;  
  EFI_LBA EntryLBA;   
  UINT32 EntryNum;   
  UINT32 EntrySize;  
  EFI_GUID Guid;
}gpt_table_info_t;

/* partition information */
typedef struct sunxi_partition_t
{
  unsigned  int       addrhi;       //ÆðÊ¼µØÖ·, ÒÔÉÈÇøÎªµ¥Î»
  unsigned  int       addrlo;       //
  unsigned  int       lenhi;        //³¤¶È
  unsigned  int       lenlo;        //
  unsigned  char      classname[16];    //´ÎÉè±¸Ãû
  unsigned  char      name[16];     //Ö÷Éè±¸Ãû
  unsigned  int       user_type;          //ÓÃ»§ÀàÐÍ
  unsigned  int       keydata;            //¹Ø¼üÊý¾Ý£¬ÒªÇóÁ¿²ú²»¶ªÊ§
  unsigned  int       ro;                 //¶ÁÐ´ÊôÐÔ
  unsigned  int       sig_verify;     //Ç©ÃûÑéÖ¤ÊôÐÔ
  unsigned  int       sig_erase;          //Ç©Ãû²Á³ýÊôÐÔ
  unsigned  int       sig_value[4];
  unsigned  int       sig_pubkey;
  unsigned  int       sig_pbumode;
  unsigned  char      reserved2[36];    //±£ÁôÊý¾Ý£¬Æ¥Åä·ÖÇøÐÅÏ¢128×Ö½Ú
}__attribute__ ((packed))sunxi_partition;

/* mbr information */
typedef struct sunxi_mbr
{
  unsigned  int       crc32;                // crc 1k - 4
  unsigned  int       version;              // °æ±¾ÐÅÏ¢£¬ 0x00000100
  unsigned  char      magic[8];             //"softw311"
  unsigned  int       copy;               //·ÖÊý
  unsigned  int       index;                //µÚ¼¸¸öMBR±¸·Ý
  unsigned  int       PartCount;              //·ÖÇø¸öÊý
  unsigned  int       stamp[1];         //¶ÔÆë
  sunxi_partition     array[SUNXI_MBR_MAX_PART_COUNT];  //
  unsigned  int       lockflag;
  unsigned  char      res[SUNXI_MBR_RESERVED];
}__attribute__ ((packed)) sunxi_mbr_t;

typedef struct tag_one_part_info
{
  unsigned  char      name[16];           //ËùÉÕÐ´·ÖÇøµÄÖ÷Éè±¸Ãû
  unsigned  int       addrhi;             //ËùÉÕÐ´·ÖÇøµÄ¸ßµØÖ·£¬ÉÈÇøµ¥Î»
  unsigned  int       addrlo;             //ËùÉÕÐ´·ÖÇøµÄµÍµØÖ·£¬ÉÈÇøµ¥Î»
  unsigned  int       lenhi;        //ËùÉÕÐ´·ÖÇøµÄ³¤¶È£¬¸ß32Î»£¬ÉÈÇøµ¥Î»
  unsigned  int       lenlo;        //ËùÉÕÐ´·ÖÇøµÄ³¤¶È£¬µÍ32Î»£¬ÉÈÇøµ¥Î»
  unsigned  char      dl_filename[16];    //ËùÉÕÐ´·ÖÇøµÄÎÄ¼þÃû³Æ£¬³¤¶È¹Ì¶¨16×Ö½Ú
  unsigned  char      vf_filename[16];    //ËùÉÕÐ´·ÖÇøµÄÐ£ÑéÎÄ¼þÃû³Æ£¬³¤¶È¹Ì¶¨16×Ö½Ú
  unsigned  int       encrypt;            //ËùÉÕÐ´·ÖÇøµÄÊý¾ÝÊÇ·ñ½øÐÐ¼ÓÃÜ 0:¼ÓÃÜ   1£º²»¼ÓÃÜ
  unsigned  int       verify;             //ËùÉÕÐ´·ÖÇøµÄÊý¾ÝÊÇ·ñ½øÐÐÐ£Ñé 0:²»Ð£Ñé 1£ºÐ£Ñé
}__attribute__ ((packed)) dl_one_part_info;

//·ÖÇøÉÕÐ´ÐÅÏ¢
typedef struct tag_download_info
{
  unsigned  int       crc32;                            //crc
  unsigned  int       version;                                    //°æ±¾ºÅ  0x00000101
  unsigned  char      magic[8];                         //"softw311"
  unsigned  int       download_count;                         //ÐèÒªÉÕÐ´µÄ·ÖÇø¸öÊý
  unsigned  int       stamp[3];                 //¶ÔÆë
  dl_one_part_info  one_part_info[SUNXI_MBR_MAX_PART_COUNT];  //ÉÕÐ´·ÖÇøµÄÐÅÏ¢
  unsigned  char      res[SUNXI_DL_RESERVED];
}
__attribute__ ((packed)) sunxi_download_info;


#endif
