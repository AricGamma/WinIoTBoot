// update_uefi.cpp : Defines the entry point for the console application.
//

#include <malloc.h>
#include <string.h>
#include "types.h"
#include "spare_head.h"
#include "check.h"
#include "script.h"
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#define  MAX_PATH             (260)


//int  script_length;
//int  align_size;

void *script_file_decode(char *script_name);
int merge_uefi(char *source_uefi_name, char *script_name);

int update_for_uefi(char *uefi_name);
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
void Usage(char* path)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "%s  <uefi_fd_image> <script_file> \n",path);
	fprintf(stderr, "\n");
  exit(-1);
}




int main(int argc, char* argv[])
{
	char   str1[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\eGon\\boot1.bin";
	char   str2[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\wboot\\bootfs\\script.bin";
	char   source_uefi_name[MAX_PATH];
	char   script_file_name[MAX_PATH];
	FILE   *src_file = NULL;
//	FILE   *script_file;
//	int    source_length;
//	int    script_length;
//	int    total_length;
//	char   *pbuf_source, *pbuf_script;
	char   *script_buf = NULL;

#if 1
	if(argc == 3)
	{
		if((argv[1] == NULL) || (argv[2] == NULL))
		{
			printf("update error: one of the input file names is empty\n");

			return __LINE__;
		}
	}
	else
	{
		Usage(argv[0]);

		return __LINE__;
	}
	GetFullPath(source_uefi_name,  argv[1]);
	GetFullPath(script_file_name,   argv[2]);
#else
	strcpy(source_boot1_name, str1);
	strcpy(script_file_name, str2);
#endif

	printf("\n");
	printf("uefi file Path=%s\n", source_uefi_name);
	printf("script file Path=%s\n", script_file_name);
	printf("\n");
	//��ʼ�����ýű�
	script_buf = (char *)script_file_decode(script_file_name);
	if(!script_buf)
	{
		printf("update uefi error: unable to get script data\n");

		goto _err_out;
	}
	script_parser_init(script_buf);
	//��ȡԭʼuefi
	if(update_for_uefi(source_uefi_name))
	{
		printf("update uefi error: update error\n");

		goto _err_out;
	}
	

	if(align_uefi(source_uefi_name))
	{
		printf("update uefi error: align error\n");
	}

	if(merge_uefi(source_uefi_name, script_file_name))
	{
		printf("update uefi error: merge error\n");

		goto _err_out;
	}
_err_out:
	if(script_buf)
	{
		free(script_buf);
	}

	return 0;
}

int update_sdcard_info(void *gpio_info, void *card_info_buf)
{
	int    i, value[8];
	int    card_no;
	script_gpio_set_t       gpio_set;
	normal_gpio_cfg        *storage_gpio;
	sdcard_spare_info      *card_info;

	card_info = (sdcard_spare_info *)card_info_buf;
	card_info->card_no[0] = -1;
	card_info->card_no[1] = -1;
	//��дSDCARD����
	for(i=0;i<2;i++)
	{
		char  card_str[32];

		card_no = i * 2;
		memset(card_str, 0, 32);
		strcpy(card_str, "card0_boot_para");
		card_str[4] = '0' + card_no;
		storage_gpio = (normal_gpio_cfg *)gpio_info + i * 8;

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
		card_info->line_count[i] = 3;
		if(1 != card_info->line_sel[i])
		{
			card_info->line_count[i] = 6;
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

		card_info ++;
	}

	return 0;
}

int update_nand_info(void *gpio_info)
{
	script_gpio_set_t   gpio_set[32];
    normal_gpio_cfg    *gpio;
	int  i;

	gpio = (normal_gpio_cfg *)gpio_info;
	if(!script_parser_mainkey_get_gpio_cfg("nand_para", gpio_set, 32))
	{
		for(i=0;i<32;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			gpio[i].port      = gpio_set[i].port;
			gpio[i].port_num  = gpio_set[i].port_num;
			gpio[i].mul_sel   = gpio_set[i].mul_sel;
			gpio[i].pull      = gpio_set[i].pull;
			gpio[i].drv_level = gpio_set[i].drv_level;
		}
	}

	return 0;
}

int update_for_uefi(char *uefi_name)
{
	FILE *uefi_file = NULL;
	struct spare_boot_head_t  *uefi_head;
	char *uefi_buf = NULL;
	int   length = 0;
	int   align_size;
	int   i;
	int   ret = -1;
	int   value[8];
	script_gpio_set_t   gpio_set[32];

	uefi_file = fopen(uefi_name, "rb+");
	if(uefi_file == NULL)
	{
		printf("update uefi error : unable to open uefi file\n");
		goto _err_uefi_out;
	}
	fseek(uefi_file, 0, SEEK_END);
	length = ftell(uefi_file);
	fseek(uefi_file, 0, SEEK_SET);
	if(!length)
	{
		printf("update uefi error : uefi length is zero\n");
		goto _err_uefi_out;
	}
	uefi_buf = (char *)malloc(length);
	if(!uefi_buf)
	{
		printf("update uefi error : fail to malloc memory for uefi\n");
		goto _err_uefi_out;
	}
	fread(uefi_buf, length, 1, uefi_file);
	rewind(uefi_file);
	uefi_head = get_spare_head(uefi_buf);
	//���uefi�����ݽṹ�Ƿ�����
	//align_size = uefi_head->boot_head.align_size;
	//�����ж�:�Ƿ���ԭʼuefi
	if((uefi_head->boot_head.length == uefi_head->boot_head.uefi_length))
	{
		//uefi���Ⱥ�ԭʼ���峤��һ�£���ʾ��ԭʼuefi
		//printf("orignal align ok!\n");
		uefi_head->boot_head.length = length;
		uefi_head->boot_head.uefi_length = length;
	}
	//���򣬲���Ҫ�޸��ļ�����
  //ret = check_file(uefi_buf, uefi_head->boot_head.length, UEFI_MAGIC );
  ret = check_magic( uefi_buf, UEFI_MAGIC );
  if( ret != CHECK_IS_CORRECT )
  {
  	printf("update uefi error : uefi pre checksum error\n");
		goto _err_uefi_out;
	}
	//ȡ�����ݽ�������,UART����
	if(!script_parser_fetch("uart_para", "uart_debug_port", value))
	{
		uefi_head->boot_data.uart_port = value[0];
	}
	if(!script_parser_mainkey_get_gpio_cfg("uart_para", gpio_set, 32))
	{
		for(i=0;i<2;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			uefi_head->boot_data.uart_gpio[i].port      = gpio_set[i].port;
			uefi_head->boot_data.uart_gpio[i].port_num  = gpio_set[i].port_num;
			uefi_head->boot_data.uart_gpio[i].mul_sel   = gpio_set[i].mul_sel;
			uefi_head->boot_data.uart_gpio[i].pull      = gpio_set[i].pull;
			uefi_head->boot_data.uart_gpio[i].drv_level = gpio_set[i].drv_level;
			uefi_head->boot_data.uart_gpio[i].data      = gpio_set[i].data;
		}
	}
	//ȡ�����ݽ�������,TWI����
	if(!script_parser_fetch("twi_para", "twi_port", value))
	{
		uefi_head->boot_data.twi_port = value[0];
	}
	if(!script_parser_mainkey_get_gpio_cfg("twi_para", gpio_set, 32))
	{
		for(i=0;i<2;i++)
		{
			if(!gpio_set[i].port)
			{
				break;
			}
			uefi_head->boot_data.twi_gpio[i].port      = gpio_set[i].port;
			uefi_head->boot_data.twi_gpio[i].port_num  = gpio_set[i].port_num;
			uefi_head->boot_data.twi_gpio[i].mul_sel   = gpio_set[i].mul_sel;
			uefi_head->boot_data.twi_gpio[i].pull      = gpio_set[i].pull;
			uefi_head->boot_data.twi_gpio[i].drv_level = gpio_set[i].drv_level;
			uefi_head->boot_data.twi_gpio[i].data      = gpio_set[i].data;
		}
	}
	//�������ݽ�������������target����
	if(!script_parser_fetch("target", "boot_clock", value))
	{
		uefi_head->boot_data.run_clock = value[0];
	}
	if(!script_parser_fetch("target", "dcdc3_vol", value))
	{
		uefi_head->boot_data.run_core_vol = value[0];
	}
	//�����洢�豸��Ϣ
	if(update_sdcard_info((void *)uefi_head->boot_data.sdcard_gpio, (void *)uefi_head->boot_data.sdcard_spare_data))
	{
		goto _err_uefi_out;
	}
	update_nand_info((void *)uefi_head->boot_data.nand_gpio);
	//�����������
	//���¼���У���
	gen_check_sum( (void *)uefi_buf );

    //�ټ��һ��
    //printf("size=%d, magic=%s\n", uefi_head->boot_head.length, UEFI_MAGIC);
    ret = check_file(uefi_buf, uefi_head->boot_head.length, UEFI_MAGIC );
    if( ret != CHECK_IS_CORRECT )
    {
    	printf("update uefi error : uefi after checksum error\n");
		goto _err_uefi_out;
	}
	fwrite(uefi_buf, length, 1, uefi_file);

_err_uefi_out:
	if(uefi_buf)
	{
		free(uefi_buf);
	}
	if(uefi_file)
	{
		fclose(uefi_file);
	}

	return ret;
}


void *script_file_decode(char *script_file_name)
{
	FILE  *script_file;
	void  *script_buf = NULL;
	int script_length = 0;
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


int reconstruct_uefi(char *buf, int length, char *script_buf, int script_length)
{
	struct spare_boot_ctrl_head  *head;
	int  total_length;
	int  script_align;
	int  align_size;
  char *tmp_start;

	head = (struct spare_boot_ctrl_head*)get_spare_head(buf);
	align_size = head->align_size;
	
	script_align = script_length;
	if(script_length & (align_size - 1))
	{
		script_align = (script_length + align_size) & (~(align_size - 1));
	}
    total_length = script_align + length;
    head->uefi_length = length;
	head->length = total_length;
    tmp_start = buf + length;

	memset(tmp_start, 0xff, script_align);
	memcpy(tmp_start, script_buf, script_length);
	if(gen_check_sum(buf))
	{
		return -1;
	}
	printf("UEFI  length = %d\n",head->uefi_length);
	printf("total length = %d\n",head->length);
	printf("checksum=%x\n", head->check_sum);

	return 0;
}


int merge_uefi(char *source_uefi_name, char *script_file_name)
{
	FILE   *src_file = NULL;
	FILE   *script_file;
	int    source_length, script_length;
	int    total_length;
	int    align_size;
	char   *pbuf_source, *pbuf_script;
	char*   buffer = NULL;
	struct spare_boot_ctrl_head   *head;
	int    ret = -1;
	//��ȡԭʼboot1
	src_file = fopen(source_uefi_name, "rb+");
	if(src_file == NULL)
	{
		printf("update uefi error:unable to open uefi file\n");
		goto _err_merge_uefi_out;
	}
	//��ȡԭʼuefi����
	  
  fseek(src_file, 0, SEEK_END);
  source_length = ftell(src_file);
  fseek(src_file, 0, SEEK_SET);
  
  pbuf_source = malloc(source_length);
  if(!pbuf_source)
	{
		printf("malloc %d bytes for UEFI failed!\n",source_length);	
		goto _err_merge_uefi_out;
	}
	//��UEFI�ļ�
  fread(pbuf_source, source_length, 1, src_file);
  
  head =(struct spare_boot_ctrl_head*)get_spare_head(pbuf_source);

	source_length = head->uefi_length;
	align_size    = head->align_size;

	//printf("%s %d:source_length = %d\n", __FUNCTION__,__LINE__,source_length);
	//printf("%s %d:align length = %d\n", __FUNCTION__,__LINE__, align_size);
	fseek(src_file, 0, SEEK_SET);
	if(!source_length)
	{
		printf("update error:the length of boot1 is zero\n");
		goto _err_merge_uefi_out;
	}
	//��ȡԭʼ�ű�
	script_file = fopen(script_file_name, "rb");
	if(!script_file)
	{
        printf("update uefi error:unable to open script file\n");
		goto _err_merge_uefi_out;
	}
    //��ȡԭʼ�ű�����
    fseek(script_file, 0, SEEK_END);
	script_length = ftell(script_file);
	if(!script_length)
	{
		printf("the length of script is zero\n");
		goto _err_merge_uefi_out;
	}
	//��ȡԭʼ�ű�
	pbuf_script = (char *)malloc(script_length);
	if(!pbuf_script)
	{
		printf("unable malloc memory for script\n");

		goto _err_merge_uefi_out;
	}
  fseek(script_file, 0, SEEK_SET);
	fread(pbuf_script, script_length, 1, script_file);
	fclose(script_file);
	script_file = NULL;
	//��ȡԭʼuefi�ڴ�
	total_length = source_length + script_length;
	if(total_length & (align_size - 1))
	{
		total_length = (total_length + align_size) & (~(align_size - 1));
	}

	pbuf_source = (char *)realloc(pbuf_source,total_length);
	if(!pbuf_source)
	{
		goto _err_merge_uefi_out;
	}
	//��ȡboot1����
	//fread(pbuf_source, source_length, 1, src_file);
	fseek(src_file, 0, SEEK_SET);
	//��������
	reconstruct_uefi(pbuf_source, source_length, pbuf_script, script_length);
	//��д�ļ�
	fwrite(pbuf_source, total_length, 1, src_file);
    //�ر��ļ�
	fclose(src_file);
	src_file = NULL;
	ret = 0;
_err_merge_uefi_out:
	if(buffer)
	{
		free(buffer);	
	}
	if(src_file)
	{
		fclose(src_file);

		src_file = NULL;
	}
	if(script_file)
	{
		fclose(script_file);

		script_file = NULL;
	}
	if(pbuf_source)
	{
		free(pbuf_source);

		pbuf_source = NULL;
	}
	if(pbuf_script)
	{
		free(pbuf_script);

		pbuf_script = NULL;
	}

	return ret;
}

int align_uefi(char *source_uefi_name)
{
	FILE   *uefi_file = NULL;
	int    source_length;
	int    total_length;
	int    align_size;
	int    origin_length;
	char   *uefi_buf;
	struct spare_boot_ctrl_head   *head;
	int    ret = -1;

	//��ȡԭʼuefi
	uefi_file = fopen(source_uefi_name, "rb+");
	if(uefi_file == NULL)
	{
		printf("update uefi error : unable to open uefi file\n");
		goto _err_align_uefi_out;
	}
	
  fseek(uefi_file, 0, SEEK_END);
	origin_length = ftell(uefi_file);
	fseek(uefi_file, 0, SEEK_SET);
	uefi_buf = malloc(origin_length);
	if(!uefi_buf)
	{
		printf("%s:%d malloc %d bytes for UEFI failed!\n",__FUNCTION__,__LINE__,origin_length);	
		goto _err_align_uefi_out;
	}
	fread(uefi_buf, origin_length, 1, uefi_file);
	rewind(uefi_file);

	head = (struct spare_boot_ctrl_head*)get_spare_head(uefi_buf);
	source_length = head->uefi_length;
	align_size = head->align_size;
	if(source_length & (align_size - 1))
	{
		total_length = (source_length + align_size) & (~(align_size - 1));
	}
	else
	{
			total_length = source_length;
	}
	//printf("source length = %d\n", source_length);
	//printf("total length = %d\n", total_length);
	uefi_buf = (char *)realloc(uefi_buf,total_length);
	if(!uefi_buf)
	{
		printf("%s:%d update uefi error : fail to malloc memory for uefi\n",__FUNCTION__,__LINE__);
		goto _err_align_uefi_out;
	}
	memset(uefi_buf, 0xff, total_length);
  
	fread(uefi_buf, source_length, 1, uefi_file);
	rewind(uefi_file);
	head = (struct spare_boot_ctrl_head*)get_spare_head(uefi_buf);
	head->uefi_length = total_length;
	head->length = total_length;
	gen_check_sum(uefi_buf );

	fwrite(uefi_buf, total_length, 1, uefi_file);

	ret = 0;
_err_align_uefi_out:
	if(uefi_buf)
	{
		free(uefi_buf);
	}
	if(uefi_file)
	{
		fclose(uefi_file);
	}

	return ret;
}

