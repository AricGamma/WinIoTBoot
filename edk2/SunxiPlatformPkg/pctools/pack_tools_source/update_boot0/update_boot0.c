// update.cpp : Defines the entry point for the console application.
//

#include <malloc.h>
#include <string.h>
#include "types.h"
#include "boot0_v2.h"
#include "check.h"
#include "script.h"
#include <ctype.h>
#include <unistd.h>

#define  MAX_PATH             (260)

void *script_file_decode(char *script_name);
int update_for_boot0(char *boot0_name, int storage_type);
//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------
int IsFullName(const char *FilePath)
{
    if (isalpha(FilePath[0]) && ':' == FilePath[1])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------
void GetFullPath(char *dName, const char *sName)
{
    char Buffer[MAX_PATH];

	if(IsFullName(sName))
	{
	    strcpy(dName, sName);
		return ;
	}

   /* Get the current working directory: */
   if(getcwd(Buffer, MAX_PATH ) == NULL)
   {
        perror( "getcwd error" );
        return ;
   }
   sprintf(dName, "%s/%s", Buffer, sName);
}

//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------
void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("update.exe script file path para file path\n\n");
}

int main(int argc, char* argv[])
{
	char   str1[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\eGon\\boot1.bin";
	char   str2[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\wboot\\bootfs\\script.bin";
	char   source_boot0_name[MAX_PATH];
	char   script_file_name[MAX_PATH];
	FILE   *src_file = NULL;
//	FILE   *script_file;
//	int    source_length, script_length;
//	int    total_length;
//	char   *pbuf_source, *pbuf_script;
	int    storage_type = 0;
	char   *script_buf = NULL;


#if 1
	if(argc == 4)
	{
		if((argv[1] == NULL) || (argv[2] == NULL) || (argv[3] == NULL))
		{
			printf("update error: one of the input file names is empty\n");

			return __LINE__;
		}
		if((!strcmp(argv[3], "sdmmc_card")) || (!strcmp(argv[3], "SDMMC_CARD")))
		{
			storage_type = 1;
		}
		else if((!strcmp(argv[3], "spinor_flash")) || (!strcmp(argv[3], "SPINOR_FLASH")))
		{
			storage_type = 2;
		}
	}
	else
	{
		Usage();

		return __LINE__;
	}
	GetFullPath(source_boot0_name,  argv[1]);
	GetFullPath(script_file_name,   argv[2]);
#else
	strcpy(source_boot0_name, str1);
	strcpy(script_file_name, str2);
#endif

	printf("\n");
	printf("boot0 file Path=%s\n", source_boot0_name);
	printf("script file Path=%s\n", script_file_name);
	printf("\n");
	//��ʼ�����ýű�
	script_buf = (char *)script_file_decode(script_file_name);
	if(!script_buf)
	{
		printf("update boot0 error: unable to get script data\n");

		goto _err_out;
	}
	script_parser_init(script_buf);
	//��ȡԭʼboot0
	update_for_boot0(source_boot0_name, storage_type);
    //��ȡԭʼ�ű�����
	printf("script update boot0 ok\n");
_err_out:
	if(script_buf)
	{
		free(script_buf);
	}

	return 0;
}


int update_sdcard_info(char  *buf)
{
	int    i, value[8];
	int    card_no;
	char  *tmp_buf = buf;
	script_gpio_set_t       gpio_set;
	normal_gpio_cfg        *storage_gpio;
	boot_sdcard_info_t     *card_info;

	card_info = (boot_sdcard_info_t *)(tmp_buf + 32 * sizeof(normal_gpio_cfg));
	//��дSDCARD����
	for(i=0;i<4;i++)
	{
		char  card_str[32];

		card_info->card_no[i] = -1;
		card_no = i;
		memset(card_str, 0, 32);
		strcpy(card_str, "card0_boot_para");
		card_str[4] = '0' + card_no;
		storage_gpio = (normal_gpio_cfg *)tmp_buf + i * 8;

		if(!script_parser_fetch(card_str, "card_ctrl", value))
		{
			card_info->card_no[i] = value[0];
		}
		else
		{
			card_info->card_no[i] = -1;
			continue;
		}
		if(!script_parser_fetch(card_str, "card_high_speed", value))
		{
			card_info->speed_mode[i] = value[0];
		}
		if(!script_parser_fetch(card_str, "card_line", value))
		{
			card_info->line_sel[i] = value[0];
		}
		//��ȡCLK
		memset(&gpio_set, 0, sizeof(script_gpio_set_t));
		if(!script_parser_fetch(card_str, "SDC_CLK", (int *)&gpio_set))
		{
			storage_gpio[0].port      = gpio_set.port;
			storage_gpio[0].port_num  = gpio_set.port_num;
			storage_gpio[0].mul_sel   = gpio_set.mul_sel;
			storage_gpio[0].pull      = gpio_set.pull;
			storage_gpio[0].drv_level = gpio_set.drv_level;
			storage_gpio[0].data      = gpio_set.data;
		}
		else if(!script_parser_fetch(card_str, "sdc_clk", (int *)&gpio_set))
		{
			storage_gpio[0].port      = gpio_set.port;
			storage_gpio[0].port_num  = gpio_set.port_num;
			storage_gpio[0].mul_sel   = gpio_set.mul_sel;
			storage_gpio[0].pull      = gpio_set.pull;
			storage_gpio[0].drv_level = gpio_set.drv_level;
			storage_gpio[0].data      = gpio_set.data;
		}
		else
		{
			printf("update error: unable to find SDC%d CLOCK PIN\n", i);

			return -1;
		}
		//��ȡCMD
		memset(&gpio_set, 0, sizeof(script_gpio_set_t));
		if(!script_parser_fetch(card_str, "SDC_CMD", (int *)&gpio_set))
		{
			storage_gpio[1].port      = gpio_set.port;
			storage_gpio[1].port_num  = gpio_set.port_num;
			storage_gpio[1].mul_sel   = gpio_set.mul_sel;
			storage_gpio[1].pull      = gpio_set.pull;
			storage_gpio[1].drv_level = gpio_set.drv_level;
			storage_gpio[1].data      = gpio_set.data;
		}
		else if(!script_parser_fetch(card_str, "sdc_cmd", (int *)&gpio_set))
		{
			storage_gpio[1].port      = gpio_set.port;
			storage_gpio[1].port_num  = gpio_set.port_num;
			storage_gpio[1].mul_sel   = gpio_set.mul_sel;
			storage_gpio[1].pull      = gpio_set.pull;
			storage_gpio[1].drv_level = gpio_set.drv_level;
			storage_gpio[1].data      = gpio_set.data;
		}
		else
		{
			printf("update error: unable to find SDC%d CLOCK CMD\n", i);

			return -1;
		}
		//��ȡDATA0
		memset(&gpio_set, 0, sizeof(script_gpio_set_t));
		if(!script_parser_fetch(card_str, "SDC_D0", (int *)&gpio_set))
		{
			storage_gpio[2].port      = gpio_set.port;
			storage_gpio[2].port_num  = gpio_set.port_num;
			storage_gpio[2].mul_sel   = gpio_set.mul_sel;
			storage_gpio[2].pull      = gpio_set.pull;
			storage_gpio[2].drv_level = gpio_set.drv_level;
			storage_gpio[2].data      = gpio_set.data;
		}
		if(!script_parser_fetch(card_str, "sdc_d0", (int *)&gpio_set))
		{
			storage_gpio[2].port      = gpio_set.port;
			storage_gpio[2].port_num  = gpio_set.port_num;
			storage_gpio[2].mul_sel   = gpio_set.mul_sel;
			storage_gpio[2].pull      = gpio_set.pull;
			storage_gpio[2].drv_level = gpio_set.drv_level;
			storage_gpio[2].data      = gpio_set.data;
		}
		else
		{
			printf("update error: unable to find SDC%d CLOCK DATA0\n", i);

			return -1;
		}
		if(1 != card_info->line_sel[i])
		{
			//��ȡDATA1
			memset(&gpio_set, 0, sizeof(script_gpio_set_t));
			if(!script_parser_fetch(card_str, "SDC_D1", (int *)&gpio_set))
			{
				storage_gpio[3].port      = gpio_set.port;
				storage_gpio[3].port_num  = gpio_set.port_num;
				storage_gpio[3].mul_sel   = gpio_set.mul_sel;
				storage_gpio[3].pull      = gpio_set.pull;
				storage_gpio[3].drv_level = gpio_set.drv_level;
				storage_gpio[3].data      = gpio_set.data;
			}
			else if(!script_parser_fetch(card_str, "sdc_d1", (int *)&gpio_set))
			{
				storage_gpio[3].port      = gpio_set.port;
				storage_gpio[3].port_num  = gpio_set.port_num;
				storage_gpio[3].mul_sel   = gpio_set.mul_sel;
				storage_gpio[3].pull      = gpio_set.pull;
				storage_gpio[3].drv_level = gpio_set.drv_level;
				storage_gpio[3].data      = gpio_set.data;
			}
			else
			{
				printf("update error: unable to find SDC%d CLOCK DATA1\n", i);

				return -1;
			}
			//��ȡDATA2
			memset(&gpio_set, 0, sizeof(script_gpio_set_t));
			if(!script_parser_fetch(card_str, "SDC_D2", (int *)&gpio_set))
			{
				storage_gpio[4].port      = gpio_set.port;
				storage_gpio[4].port_num  = gpio_set.port_num;
				storage_gpio[4].mul_sel   = gpio_set.mul_sel;
				storage_gpio[4].pull      = gpio_set.pull;
				storage_gpio[4].drv_level = gpio_set.drv_level;
				storage_gpio[4].data      = gpio_set.data;
			}
			else if(!script_parser_fetch(card_str, "sdc_d2", (int *)&gpio_set))
			{
				storage_gpio[4].port      = gpio_set.port;
				storage_gpio[4].port_num  = gpio_set.port_num;
				storage_gpio[4].mul_sel   = gpio_set.mul_sel;
				storage_gpio[4].pull      = gpio_set.pull;
				storage_gpio[4].drv_level = gpio_set.drv_level;
				storage_gpio[4].data      = gpio_set.data;
			}
			else
			{
				printf("update error: unable to find SDC%d CLOCK DATA2\n", i);

				return -1;
			}
			//��ȡDATA3
			memset(&gpio_set, 0, sizeof(script_gpio_set_t));
			if(!script_parser_fetch(card_str, "SDC_D3", (int *)&gpio_set))
			{
				storage_gpio[5].port      = gpio_set.port;
				storage_gpio[5].port_num  = gpio_set.port_num;
				storage_gpio[5].mul_sel   = gpio_set.mul_sel;
				storage_gpio[5].pull      = gpio_set.pull;
				storage_gpio[5].drv_level = gpio_set.drv_level;
				storage_gpio[5].data      = gpio_set.data;
			}
			else if(!script_parser_fetch(card_str, "sdc_d3", (int *)&gpio_set))
			{
				storage_gpio[5].port      = gpio_set.port;
				storage_gpio[5].port_num  = gpio_set.port_num;
				storage_gpio[5].mul_sel   = gpio_set.mul_sel;
				storage_gpio[5].pull      = gpio_set.pull;
				storage_gpio[5].drv_level = gpio_set.drv_level;
				storage_gpio[5].data      = gpio_set.data;
			}
			else
			{
				printf("update error: unable to find SDC%d CLOCK DATA3\n", i);

				return -1;
			}
		}
	}

	return 0;
}


int update_for_boot0(char *boot0_name, int storage_type)
{
	FILE *boot0_file = NULL;
	boot0_file_head_t  *boot0_head;
	char *boot0_buf = NULL;
	int   length = 0;
	int   i;
	int   ret = -1;
	int   value[8];
    script_gpio_set_t   gpio_set[32];

	boot0_file = fopen(boot0_name, "rb+");
	if(boot0_file == NULL)
	{
		printf("update:unable to open boot0 file\n");
		goto _err_boot0_out;
	}
	fseek(boot0_file, 0, SEEK_END);
	length = ftell(boot0_file);
	fseek(boot0_file, 0, SEEK_SET);
	if(!length)
	{
		goto _err_boot0_out;
	}
	boot0_buf = (char *)malloc(length);
	if(!boot0_buf)
	{
		goto _err_boot0_out;
	}
	fread(boot0_buf, length, 1, boot0_file);
	rewind(boot0_file);

	boot0_head = (boot0_file_head_t *)boot0_buf;
	//���boot0�����ݽṹ�Ƿ�����
    ret = check_file( (unsigned int *)boot0_buf, boot0_head->boot_head.length, BOOT0_MAGIC );
    if( ret != CHECK_IS_CORRECT )
    {
		goto _err_boot0_out;
	}
	//ȡ�����ݽ�������,DRAM����
	if(script_parser_sunkey_all("dram_para", (void *)boot0_head->prvt_head.dram_para))
	{
		printf("script fetch dram para failed\n");
		goto _err_boot0_out;
	}
	//ȡ�����ݽ�������,UART����
	if(!script_parser_fetch("uart_para", "uart_debug_port", value))
	{
		boot0_head->prvt_head.uart_port = value[0];
	}
	if(!script_parser_mainkey_get_gpio_cfg("uart_para", gpio_set, 32))
	{
		for(i=0;i<32;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			boot0_head->prvt_head.uart_ctrl[i].port      = gpio_set[i].port;
			boot0_head->prvt_head.uart_ctrl[i].port_num  = gpio_set[i].port_num;
			boot0_head->prvt_head.uart_ctrl[i].mul_sel   = gpio_set[i].mul_sel;
			boot0_head->prvt_head.uart_ctrl[i].pull      = gpio_set[i].pull;
			boot0_head->prvt_head.uart_ctrl[i].drv_level = gpio_set[i].drv_level;
			boot0_head->prvt_head.uart_ctrl[i].data      = gpio_set[i].data;
		}
	}
	//ȡ�����ݽ�������,debugenable����
	if(!script_parser_fetch("jtag_para", "jtag_enable", value))
	{
		boot0_head->prvt_head.enable_jtag = value[0];
	}
	if(!script_parser_mainkey_get_gpio_cfg("jtag_para", gpio_set, 32))
	{
		for(i=0;i<32;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			boot0_head->prvt_head.jtag_gpio[i].port      = gpio_set[i].port;
			boot0_head->prvt_head.jtag_gpio[i].port_num  = gpio_set[i].port_num;
			boot0_head->prvt_head.jtag_gpio[i].mul_sel   = gpio_set[i].mul_sel;
			boot0_head->prvt_head.jtag_gpio[i].pull      = gpio_set[i].pull;
			boot0_head->prvt_head.jtag_gpio[i].drv_level = gpio_set[i].drv_level;
			boot0_head->prvt_head.jtag_gpio[i].data      = gpio_set[i].data;
		}
	}
	//ȡ�����ݽ���������NAND����
	if(!storage_type)
	{
		if(!script_parser_mainkey_get_gpio_cfg("nand_para", gpio_set, 32))
		{
			for(i=0;i<32;i++)
			{
				if(!gpio_set[i].port)
				{
					break;
				}
				boot0_head->prvt_head.storage_gpio[i].port      = gpio_set[i].port;
				boot0_head->prvt_head.storage_gpio[i].port_num  = gpio_set[i].port_num;
				boot0_head->prvt_head.storage_gpio[i].mul_sel   = gpio_set[i].mul_sel;
				boot0_head->prvt_head.storage_gpio[i].pull      = gpio_set[i].pull;
				boot0_head->prvt_head.storage_gpio[i].drv_level = gpio_set[i].drv_level;
				boot0_head->prvt_head.storage_gpio[i].data      = gpio_set[i].data;
			}
		}
	}
	else if(1 == storage_type) //ȡ�ÿ�����
	{
		if(update_sdcard_info((char *)boot0_head->prvt_head.storage_gpio))
		{
			goto _err_boot0_out;
		}
	}
	else if(2 == storage_type)
	{
		if(!script_parser_mainkey_get_gpio_cfg("spi0_para", gpio_set, 32))
		{
			for(i=0;i<32;i++)
			{
				if(!gpio_set[i].port)
				{
					break;
				}
				boot0_head->prvt_head.storage_gpio[i].port      = gpio_set[i].port;
				boot0_head->prvt_head.storage_gpio[i].port_num  = gpio_set[i].port_num;
				boot0_head->prvt_head.storage_gpio[i].mul_sel   = gpio_set[i].mul_sel;
				boot0_head->prvt_head.storage_gpio[i].pull      = gpio_set[i].pull;
				boot0_head->prvt_head.storage_gpio[i].drv_level = gpio_set[i].drv_level;
				boot0_head->prvt_head.storage_gpio[i].data      = gpio_set[i].data;
			}
		}
	}
	//�����������
	//���¼���У���
	gen_check_sum( (void *)boot0_buf );
	//�ټ��һ��
    ret = check_file( (unsigned int *)boot0_buf, boot0_head->boot_head.length, BOOT0_MAGIC );
    if( ret != CHECK_IS_CORRECT )
    {
		goto _err_boot0_out;
	}
	fwrite(boot0_buf, length, 1, boot0_file);

_err_boot0_out:
	if(boot0_buf)
	{
		free(boot0_buf);
	}
	if(boot0_file)
	{
		fclose(boot0_file);
	}

	return ret;
}


void *script_file_decode(char *script_file_name)
{
	FILE  *script_file;
	void  *script_buf = NULL;
	int    script_length;
	//��ȡԭʼ�ű�
	script_file = fopen(script_file_name, "rb");
	if(!script_file)
	{
        printf("update error:unable to open script file\n");
		return NULL;
	}
    //��ȡԭʼ�ű�����
    fseek(script_file, 0, SEEK_END);
	script_length = ftell(script_file);
	if(!script_length)
	{
		fclose(script_file);
		printf("the length of script is zero\n");

		return NULL;
	}
	//��ȡԭʼ�ű�
	script_buf = (char *)malloc(script_length);
	if(!script_buf)
	{
		fclose(script_file);
		printf("unable malloc memory for script\n");

		return NULL;;
	}
    fseek(script_file, 0, SEEK_SET);
	fread(script_buf, script_length, 1, script_file);
	fclose(script_file);

	return script_buf;
}
