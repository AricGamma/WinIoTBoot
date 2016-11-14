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
#define     SUNXI_GPT_COPY_NUM          4    //GPT�ı�������
#define     SUNXI_GPT_RESERVED          (SUNXI_GPT_SIZE - 32 - (SUNXI_GPT_MAX_PART_COUNT * sizeof(gpt_partition)))   //mbr�����Ŀռ�
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
	unsigned  int       addrhi;				//��ʼ��ַ, ������Ϊ��λ
	unsigned  int       addrlo;				//
	unsigned  int       lenhi;				//����
	unsigned  int       lenlo;				//
	unsigned  char      classname[16];		//���豸��
	unsigned  char      name[16];			//���豸��
	unsigned  int       user_type;          //�û�����
	unsigned  int       keydata;            //�ؼ����ݣ�Ҫ����������ʧ
	unsigned  int       ro;                 //��д����
        efi_guid	    type;
	efi_guid	    uniqueguid;
	unsigned  int	    attrhi;				//��������
	unsigned  int	    attrlo;
	unsigned  char      reserved[28];		//�������ݣ�ƥ�������Ϣ128�ֽ�
} //__attribute__ ((packed))sunxi_partition;
gpt_partition;

/* mbr information */
typedef struct sunxi_gpt
{
	unsigned  int       crc32;				        // crc 1k - 4
	unsigned  int       version;			        // �汾��Ϣ�� 0x00000100
	unsigned  char 	    magic[8];			        //"softw311"
	unsigned  int 	    copy;				        //����
	unsigned  int 	    index;				        //�ڼ���gpt����
	unsigned  int       PartCount;			        //��������
	unsigned  int       stamp[1];					//����
	gpt_partition       array[SUNXI_GPT_MAX_PART_COUNT];	//
	unsigned  char      res[SUNXI_GPT_RESERVED];
}//__attribute__ ((packed)) sunxi_mbr_t;
sunxi_gpt_t;

typedef struct tag_one_part_info
{
	unsigned  char      name[16];           //����д���������豸��
	unsigned  int       addrhi;             //����д�����ĸߵ�ַ��������λ
	unsigned  int       addrlo;             //����д�����ĵ͵�ַ��������λ
	unsigned  int       lenhi;				//����д�����ĳ��ȣ���32λ��������λ
	unsigned  int       lenlo;				//����д�����ĳ��ȣ���32λ��������λ
	unsigned  char      dl_filename[16];    //����д�������ļ����ƣ����ȹ̶�16�ֽ�
	unsigned  char      vf_filename[16];    //����д������У���ļ����ƣ����ȹ̶�16�ֽ�
	unsigned  int       encrypt;            //����д�����������Ƿ���м��� 0:����   1��������
	unsigned  int       verify;             //����д�����������Ƿ����У�� 0:��У�� 1��У��
}//__attribute__ ((packed)) dl_one_part_info;
dl_one_part_info;
//������д��Ϣ
typedef struct tag_download_info
{
	unsigned  int       crc32;				        		        //crc
	unsigned  int       version;                                    //�汾��  0x00000101
	unsigned  char 	    magic[8];			        		        //"softw311"
	unsigned  int       download_count;             		        //��Ҫ��д�ķ�������
	unsigned  int       stamp[3];									//����
	dl_one_part_info    one_part_info[SUNXI_GPT_MAX_PART_COUNT];	//��д��������Ϣ
	unsigned  char      res[SUNXI_DL_RESERVED];
}
//__attribute__ ((packed)) sunxi_download_info;
sunxi_download_info;

#endif
