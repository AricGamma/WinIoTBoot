/*
*********************************************************************************************************
*											        ePDK
*						            the Easy Portable/Player Develop Kits
*									         Decompression For Boot
*
*						        (c) Copyright 2009-2010, Sunny China
*											All	Rights Reserved
*
* File    : calc_crc32.c
* By      : sunny
* Version : V2.0
* Date	  : 2009-11-4 10:34:26
*********************************************************************************************************
*/
#include  "crc.h"
#include  "types.h"

__u32 calc_crc32(void * buffer, u32 length)
{
	__u32 i, j;
	CRC32_DATA_t crc32;		//
	__u32 CRC32 = 0xffffffff; //���ó�ʼֵ

	crc32.CRC = 0;

	for( i = 0; i < 256; ++i)//��++i�����Ч��
	{
		crc32.CRC = i;
		for( j = 0; j < 8 ; ++j)
		{
			//���ѭ��ʵ���Ͼ�����"���㷨"����ȡCRC��У����
			if(crc32.CRC & 1)
				crc32.CRC = (crc32.CRC >> 1) ^ 0xEDB88320;
			else //0xEDB88320����CRC-32������ʽ��ֵ
				crc32.CRC >>= 1;
		}
		crc32.CRC_32_Tbl[i] = crc32.CRC;
	}

	CRC32 = 0xffffffff; //���ó�ʼֵ
    for( i = 0; i < length; ++i)
    {
        CRC32 = crc32.CRC_32_Tbl[(CRC32^((unsigned char*)buffer)[i]) & 0xff] ^ (CRC32>>8);
    }

    //return CRC32;
	return CRC32^0xffffffff;
}

