/*
**********************************************************************************************************************
*											        eGon
*						                     the Embedded System
*									       script parser sub-system
*
*						  Copyright(C), 2006-2010, SoftWinners Microelectronic Co., Ltd.
*                                           All Rights Reserved
*
* File    : script.c
*
* By      : Jerry
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include <malloc.h>
#include "types.h"
#include <string.h>
#include "script.h"
#include "crc.h"
#include "sunxi_mbr.h"
#include <ctype.h>
#include <unistd.h>

static  char  *script_mod_buf = NULL;           //ָ���һ������
static  int    script_main_key_count = 0;       //���������ĸ���

static  int   partition_start;
static  int   partition_next;

static  int   _test_str_length(char *str)
{
	int length = 0;

	while(str[length++])
	{
		if(length > 32)
		{
			length = 32;
			break;
		}
	}

	return length;
}
/*
************************************************************************************************************
*
*                                             script_parser_init
*
*    �������ƣ�
*
*    �����б�script_buf: �ű����ݳ�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int script_parser_init(char *script_buf)
{
	script_head_t   *script_head;

	if(script_buf)
	{
		script_mod_buf = script_buf;
		script_head = (script_head_t *)script_mod_buf;

		script_main_key_count = script_head->main_key_count;

		return SCRIPT_PARSER_OK;
	}
	else
	{
		printf("thi input script data buffer is null\n");

		return SCRIPT_PARSER_EMPTY_BUFFER;
	}
}
/*
************************************************************************************************************
*
*                                             script_parser_exit
*
*    �������ƣ�
*
*    �����б�NULL
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int script_parser_exit(void)
{
	script_mod_buf = NULL;
	script_main_key_count = 0;

	return SCRIPT_PARSER_OK;
}

/*
************************************************************************************************************
*
*                                             script_parser_fetch
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    �����ݴ������������Ӽ���ȡ�ö�Ӧ����ֵ
*
*
************************************************************************************************************
*/
int script_parser_fetch(char *main_name, char *sub_name, int value[])
{
	char   main_char[32], sub_char[32];
	script_main_key_t  *main_key;
	script_sub_key_t   *sub_key;
	int    i, j, k;
	int    pattern, word_count;

	//���ű�buffer�Ƿ����
	if(!script_mod_buf)
	{
		printf("the script data buffer is null, unable to parser\n");

		return SCRIPT_PARSER_EMPTY_BUFFER;
	}
	//����������ƺ��Ӽ������Ƿ�Ϊ��
	if((main_name == NULL) || (sub_name == NULL))
	{
		printf("the input main key name or sub key name is null\n");

		return SCRIPT_PARSER_KEYNAME_NULL;
	}
	//�������buffer�Ƿ�Ϊ��
	if(value == NULL)
	{
		printf("the input data value is null\n");

		return SCRIPT_PARSER_DATA_VALUE_NULL;
	}
	//�����������ƺ��Ӽ����ƣ��������16�ֽ����ȡ16�ֽ�
	memset(main_char, 0, sizeof(main_char));
	memset(sub_char, 0, sizeof(sub_char));
	if(_test_str_length(main_name) <= 32)
	{
		strcpy(main_char, main_name);
	}
	else
	{
		strncpy(main_char, main_name, 31);
	}

	if(_test_str_length(sub_name) <= 32)
	{
		strcpy(sub_char, sub_name);
	}
	else
	{
		strncpy(sub_char, sub_name, 31);
	}

	for(i=0;i<script_main_key_count;i++)
	{
		main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
		if(strcmp(main_key->main_name, main_char))    //���������ƥ�䣬Ѱ����һ������
		{
			continue;
		}
		//����ƥ�䣬Ѱ���Ӽ�����ƥ��
		for(j=0;j<main_key->lenth;j++)
		{
			sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
			if(strcmp(sub_key->sub_name, sub_char))    //���������ƥ�䣬Ѱ����һ������
			{
				continue;
			}
			pattern    = (sub_key->pattern>>16) & 0xffff;             //��ȡ���ݵ�����
			word_count = (sub_key->pattern>> 0) & 0xffff;             //��ȡ��ռ�õ�word����
			//ȡ������
			switch(pattern)
			{
				case DATA_TYPE_SINGLE_WORD:                           //��word��������
					value[0] = *(int *)(script_mod_buf + (sub_key->offset<<2));
					break;

				case DATA_TYPE_STRING:     							  //�ַ�����������
					memcpy((char *)value, script_mod_buf + (sub_key->offset<<2), word_count << 2);
					break;

				case DATA_TYPE_GPIO_WORD:							 //��word��������
					k = 0;
					while(k < word_count)
					{
						value[k] = *(int *)(script_mod_buf + (sub_key->offset<<2) + (k<<2));
						k ++;
					}
					break;
			}

			return SCRIPT_PARSER_OK;
		}
	}

	return SCRIPT_PARSER_KEY_NOT_FIND;
}

/*
************************************************************************************************************
*
*                                             script_parser_fetch
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    �����ݴ������������Ӽ���ȡ�ö�Ӧ����ֵ
*
*
************************************************************************************************************
*/
int script_parser_fetch_partition(void)
{
	script_main_key_t  *main_key;
	int  i;

	//���ű�buffer�Ƿ����
	if(!partition_start)		//����partition_start��Ȼ����������partition
	{
		for(i=0;i<script_main_key_count;i++)
		{
			main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + i * sizeof(script_main_key_t));
			if(strcmp(main_key->main_name, "partition_start"))    //���������ƥ�䣬Ѱ����һ������
			{
				continue;
			}
			else
			{
				partition_start = i;
				partition_next = partition_start;
				break;
			}
		}
		if(!partition_start)
		{
			printf("unable to find key partition_start\n");

			return -1;
		}
	}
	partition_next  ++;
	main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + partition_next * sizeof(script_main_key_t));
	if(strcmp(main_key->main_name, "partition"))
	{
		printf("this is not a partition key\n");

		return 0;
	}

	return partition_next;
}


/*
************************************************************************************************************
*
*                                             script_parser_fetch
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    �����ݴ������������Ӽ���ȡ�ö�Ӧ����ֵ
*
*
************************************************************************************************************
*/
int script_parser_fetch_mainkey_sub(char *sub_name, int index, int *value)
{
	char   sub_char[32];
	script_main_key_t  *main_key;
	script_sub_key_t  *sub_key;
	int    j, k;
	int    pattern, word_count;

	//���ű�buffer�Ƿ����
	if(!script_mod_buf)
	{
		printf("the script data buffer is null, unable to parser\n");

		return SCRIPT_PARSER_EMPTY_BUFFER;
	}
	//����������ƺ��Ӽ������Ƿ�Ϊ��
	if(sub_name == NULL)
	{
		printf("the input sub key name is null\n");

		return SCRIPT_PARSER_KEYNAME_NULL;
	}
	//�����������ƺ��Ӽ����ƣ��������16�ֽ����ȡ16�ֽ�
	memset(sub_char, 0, sizeof(sub_char));
	if(_test_str_length(sub_name) <= 32)
	{
		strcpy(sub_char, sub_name);
	}

	main_key = (script_main_key_t *)(script_mod_buf + (sizeof(script_head_t)) + index * sizeof(script_main_key_t));
	if(strcmp(main_key->main_name, "partition"))    //���������ƥ�䣬Ѱ����һ������
	{
		printf("this is not a good partition key\n");

		return -1;
	}
	//����ƥ�䣬Ѱ���Ӽ�����ƥ��
	for(j=0;j<main_key->lenth;j++)
	{
		sub_key = (script_sub_key_t *)(script_mod_buf + (main_key->offset<<2) + (j * sizeof(script_sub_key_t)));
		if(strcmp(sub_key->sub_name, sub_char))    //���������ƥ�䣬Ѱ����һ������
		{
			continue;
		}
		pattern    = (sub_key->pattern>>16) & 0xffff;             //��ȡ���ݵ�����
		word_count = (sub_key->pattern>> 0) & 0xffff;             //��ȡ��ռ�õ�word����
		//ȡ������
		switch(pattern)
		{
			case DATA_TYPE_SINGLE_WORD:                           //��word��������
				value[0] = *(int *)(script_mod_buf + (sub_key->offset<<2));
				break;

			case DATA_TYPE_STRING:     							  //�ַ�����������
				if(!word_count)
				{
					return 1;
				}
				memcpy((char *)value, script_mod_buf + (sub_key->offset<<2), word_count << 2);
				break;

			case DATA_TYPE_GPIO_WORD:							 //��word��������
				k = 0;
				while(k < word_count)
				{
					value[k] = *(int *)(script_mod_buf + (sub_key->offset<<2) + (k<<2));
					k ++;
				}
				break;
		}

		return SCRIPT_PARSER_OK;
	}

	return SCRIPT_PARSER_KEY_NOT_FIND;
}

