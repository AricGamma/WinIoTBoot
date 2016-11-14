// update.cpp : Defines the entry point for the console application.
//

#include <malloc.h>
#include <string.h>
#include "types.h"
#include "spare_head.h"
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#define  MAX_PATH             (260)
int  script_length;
int  align_size;

void *script_file_decode(char *script_name);
int merge_uefi(char *source_uefi_name, char *script_name);
int genchecksum_for_uefi(char *uefi_name);
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
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
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
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
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void Usage(char* path)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "%s  <uefi_fd_image> \n",path);
	fprintf(stderr, "\n");
  exit(-1);
}

int main(int argc, char* argv[])
{
	char   str1[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\eGon\\boot1.bin";
	char   str2[] = "D:\\winners\\eBase\\eGON\\EGON2_TRUNK\\boot_23\\workspace\\wboot\\bootfs\\script.bin";
	char   source_uefi_name[MAX_PATH];
//	int    source_length, script_length;
//	int    total_length;
//	char   *pbuf_source, *pbuf_script;
	char   *script_buf = NULL;

#if 1
	if(argc == 2)
	{
		if(argv[1] == NULL)
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
#else
	strcpy(source_uefi_name, str1);
#endif

	printf("\n");
	printf("uefi file Path=%s\n", source_uefi_name);
	printf("\n");
	//读取原始uefi
	if(genchecksum_for_uefi(source_uefi_name))
	{
		printf("update uefi error: update error\n");

		return -1;
	}

	return 0;

}

int genchecksum_for_uefi(char *uefi_name)
{
	FILE *uefi_file = NULL;
	struct spare_boot_head_t  *uefi_head;
	char *uefi_buf = NULL;
	char *uefi_head_buf =NULL;
//	int   i;
	int   ret = -1;
	int   align_size, uefi_length;
	int   origin_length;
	char*  buffer = NULL;

	uefi_file = fopen(uefi_name, "rb+");
	if(uefi_file == NULL)
	{
		printf("genchecksum uefi error : unable to open uefi file\n");
		goto _err_uefi_out;
	}
	fseek(uefi_file, 0, SEEK_END);
	origin_length = ftell(uefi_file);
	fseek(uefi_file, 0, SEEK_SET);
	if(!origin_length)
	{
		printf("update uefi error : uefi length is zero\n");
		goto _err_uefi_out;
	}
	buffer = malloc(origin_length);
	if(!buffer)
	{
		printf("malloc %d bytes for UEFI failed!\n",origin_length);	
		goto _err_uefi_out;
	}
	//获取对齐尺寸
	fread(buffer, origin_length, 1, uefi_file);
	uefi_head = get_spare_head(buffer);

	align_size = uefi_head->boot_head.align_size;
	printf("origin_length=%d\n",origin_length);
	printf("align_size=%d\n",align_size);
	uefi_length = origin_length;
	if(origin_length & (align_size - 1))
	{
		uefi_length = (uefi_length + align_size) & (~(align_size - 1));
	}
	//为对齐重新开辟空间
	uefi_buf = (char *)realloc((void*)buffer,uefi_length);
	if(!uefi_buf)
	{
		printf("update uefi error : fail to malloc memory for uefi\n");
		goto _err_uefi_out;
	}
	memset(uefi_buf, 0xff, uefi_length);
	rewind(uefi_file);
	
	fread(uefi_buf, origin_length, 1, uefi_file);
	//还原文件大小
	uefi_head = get_spare_head(uefi_buf);;
	uefi_head->boot_head.length = uefi_length;
	uefi_head->boot_head.uefi_length = uefi_length;
    //重新生成校验和
    gen_check_sum(uefi_buf);
	//写入文件
	rewind(uefi_file);
	fwrite(uefi_buf, uefi_length, 1, uefi_file);
	ret = 0;

_err_uefi_out:
	if(buffer)
	{
		free(buffer);	
	}
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




