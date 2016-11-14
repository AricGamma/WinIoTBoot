/*
*********************************************************************************************************
*											        ePDK
*						            the Easy Portable/Player Develop Kits
*									         Decompression For Boot
*
*						        (c) Copyright 2009-2010, Sunny China
*											All	Rights Reserved
*
* File    : DFB_File.h
* By      : sunny
* Version : V2.0
* Date	  : 2009-11-4 10:35:33
*********************************************************************************************************
*/
#ifndef __CAL_CRC_H__
#define __CAL_CRC_H__


typedef struct tag_CRC32_DATA
{
	unsigned CRC;				//��Windows�±�̣�int�Ĵ�С��32λ
	unsigned CRC_32_Tbl[256];	//�����������
}CRC32_DATA_t;



extern  unsigned  calc_crc32(void * buffer, unsigned length);


#endif	/* __CAL_CRC_H__ */
