/*
 * (C) Copyright 2012
 *     wangflord@allwinnertech.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 */
#ifndef  __SUNXI_GPT_H__
#define  __SUNXI_GPT_H__

#define     DOWNLOAD_MAP_NAME   "dlinfo.fex"
#define     SUNXI_GPT_NAME      "sunxi_mbr.fex"
/* MBR       */
#define     SUNXI_GPT_SIZE			    (16 * 1024)
#define     SUNXI_DL_SIZE				(16 * 1024)
#define     SUNXI_GPT_MAGIC			    "softw411"
#define     SUNXI_GPT_MAX_PART_COUNT	120
#define     SUNXI_GPT_COPY_NUM          4    //GPT的备份数量
#define     SUNXI_GPT_RESERVED          (SUNXI_GPT_SIZE - 32 - (SUNXI_GPT_MAX_PART_COUNT * sizeof(gpt_partition)))   //mbr保留的空间
#define     SUNXI_DL_RESERVED           (SUNXI_DL_SIZE - 32 - (SUNXI_GPT_MAX_PART_COUNT * sizeof(dl_one_part_info)))

/* efi guid type */
typedef struct {
  unsigned int    data1;
  unsigned short  data2;
  unsigned short  data3;
  unsigned char   data4[8];
} efi_guid;

/* partition information */
typedef struct gpt_partition_t
{
	unsigned  int       addrhi;				//起始地址, 以扇区为单位
	unsigned  int       addrlo;				//
	unsigned  int       lenhi;				//长度
	unsigned  int       lenlo;				//
	unsigned  char      classname[16];		//次设备名
	unsigned  char      name[16];			//主设备名
	unsigned  int       user_type;          //用户类型
	unsigned  int       keydata;            //关键数据，要求量产不丢失
	unsigned  int       ro;                 //读写属性
        efi_guid	    type;
	efi_guid	    uniqueguid;
	unsigned  int	    attrhi;				//分区属性
	unsigned  int	    attrlo;
	unsigned  char      reserved[28];		//保留数据，匹配分区信息128字节
} //__attribute__ ((packed))sunxi_partition;
gpt_partition;

/* mbr information */
typedef struct sunxi_gpt
{
	unsigned  int       crc32;				        // crc 1k - 4
	unsigned  int       version;			        // 版本信息， 0x00000100
	unsigned  char 	    magic[8];			        //"softw311"
	unsigned  int 	    copy;				        //分数
	unsigned  int 	    index;				        //第几个gpt备份
	unsigned  int       PartCount;			        //分区个数
	unsigned  int       stamp[1];					//对齐
	gpt_partition       array[SUNXI_GPT_MAX_PART_COUNT];	//
	unsigned  char      res[SUNXI_GPT_RESERVED];
}//__attribute__ ((packed)) sunxi_mbr_t;
sunxi_gpt_t;

typedef struct tag_one_part_info
{
	unsigned  char      name[16];           //所烧写分区的主设备名
	unsigned  int       addrhi;             //所烧写分区的高地址，扇区单位
	unsigned  int       addrlo;             //所烧写分区的低地址，扇区单位
	unsigned  int       lenhi;				//所烧写分区的长度，高32位，扇区单位
	unsigned  int       lenlo;				//所烧写分区的长度，低32位，扇区单位
	unsigned  char      dl_filename[16];    //所烧写分区的文件名称，长度固定16字节
	unsigned  char      vf_filename[16];    //所烧写分区的校验文件名称，长度固定16字节
	unsigned  int       encrypt;            //所烧写分区的数据是否进行加密 0:加密   1：不加密
	unsigned  int       verify;             //所烧写分区的数据是否进行校验 0:不校验 1：校验
}//__attribute__ ((packed)) dl_one_part_info;
dl_one_part_info;
//分区烧写信息
typedef struct tag_download_info
{
	unsigned  int       crc32;				        		        //crc
	unsigned  int       version;                                    //版本号  0x00000101
	unsigned  char 	    magic[8];			        		        //"softw311"
	unsigned  int       download_count;             		        //需要烧写的分区个数
	unsigned  int       stamp[3];									//对齐
	dl_one_part_info    one_part_info[SUNXI_GPT_MAX_PART_COUNT];	//烧写分区的信息
	unsigned  char      res[SUNXI_DL_RESERVED];
}
//__attribute__ ((packed)) sunxi_download_info;
sunxi_download_info;

#endif
