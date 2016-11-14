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
unsigned int cryptTable[0x500];
unsigned int seed1 = 0x7FED7FED;
unsigned int seed2 = 0xEEEEEEEE;
int key = 0;

unsigned int verify_sum( void *mem_base, unsigned int size )
{
	unsigned int *buf;
	unsigned int count;
	unsigned int sum;

	count = size >> 2;                         // �� �֣�4bytes��Ϊ��λ����
	sum = 0;
	buf = (unsigned int *)mem_base;
	while(count--)
	{
		sum += *buf++;                         // �����ۼӣ����У���
	};

	switch(size & 0x03)
	{
		case 0:
			return sum;
		case 1:
			sum += (*buf & 0x000000ff);
			break;
		case 2:
			sum += (*buf & 0x0000ffff);
			break;
		case 3:
			sum += (*buf & 0x00ffffff);
			break;
	}

	return sum;
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
*    note          :  ��������
*
*
************************************************************************************************************
*/
void prepareCryptTable(void)
{
//	unsigned int seed = 0x00100001, index1 = 0, index2 = 0, i;
//
//	for( index1 = 0; index1 < 0x100; index1++ )
//	{
//		for( index2 = index1, i = 0; i < 5; i++, index2 += 0x100 )
//		{
//			unsigned long temp1, temp2;
//
//			seed = (seed * 125 + 3) % 0x2AAAAB;
//			temp1 = (seed & 0xFFFF) << 0x10;
//			seed = (seed * 125 + 3) % 0x2AAAAB;
//			temp2 = (seed & 0xFFFF);
//
//			cryptTable[index2] = ( temp1 | temp2 );
//		}
//	}
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
*    note          :  ����hashֵ
*
*
************************************************************************************************************
*/
unsigned int HashString( unsigned char *str, unsigned int dwHashType, unsigned int length )
{
//	unsigned char *key = (unsigned char *)str;
//	int ch;
//
//	while( length > 0 )
//	{
//		ch = *key++;
//		seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
//		seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
//		length --;
//	}
//
//	return seed1;
	key += verify_sum((void *)str, length);

	return (unsigned int)key;
}

void HashString_init(void)
{
	key = 0;
}


