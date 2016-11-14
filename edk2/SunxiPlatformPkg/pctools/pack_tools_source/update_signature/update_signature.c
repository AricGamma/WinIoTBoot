// update_uboot.cpp : Defines the entry point for the console application.
//

#include "signature.h"
#include "superblock.h"
#include "imgheader.h"

#define  MAX_PATH             (260)

#define  HASH_BUFFER_BYTES                (32 * 1024)

#define  PARTITION_BOOT_VERIFY_OFFSET     (4 * 1024)
#define  PARTITION_BOOT_VERIFY_COUNT      (20)
#define  PARTITION_BOOT_VERIFY_STEP       (256 * 1024)

#define  PARTITION_SYSTEM_VERIFY_OFFSET   (5 * 1024 * 1024)
#define  PARTITION_SYSTEM_VERIFY_COUNT    (40)
#define  PARTITION_SYSTEM_VERIFY_STEP     (5 * 1024 * 1024)

unsigned int signature_raw_file_hash(char *file_name);
unsigned int signature_sparse_file_hash(char *sfile_name, char *vfile_name);
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
void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("signature file_name\n\n");
}


int main(int argc, char* argv[])
{
	char   signature_file_name[MAX_PATH];
	char   verify_file_name[MAX_PATH];
#if 1
	if(argc == 2)
	{
		if(argv[1] == NULL)
		{
			printf("update error: one of the input file names is empty\n");

			return __LINE__;
		}
		memset(signature_file_name, 0, MAX_PATH);
		GetFullPath(signature_file_name, argv[1]);
		if(!strcmp(argv[1], "boot.fex"))
		{
			signature_raw_file_hash(signature_file_name);
		}
		else
		{
			Usage();

			printf("Invalid file name to signature\n");
		}
	}
	else if(argc == 3)
	{
		if((argv[1] == NULL) || (argv[2] == NULL))
		{
			printf("update error: one of the input file names is empty\n");

			return __LINE__;
		}
		memset(signature_file_name, 0, MAX_PATH);
		memset(verify_file_name, 0, MAX_PATH);
		GetFullPath(signature_file_name, argv[1]);
		GetFullPath(verify_file_name, argv[2]);

		if((!strcmp(argv[1], "system.fex")) && (!strcmp(argv[2], "system.img")) )
		{
			debug("begin to parser\n");
			signature_sparse_file_hash(signature_file_name, verify_file_name);
		}
		else
		{
			Usage();

			printf("Invalid file name to signature\n");
		}
	}
	else
	{
		Usage();

		return __LINE__;
	}
#else
	strcpy(signature_file_name, str1);
#endif

	return 0;
}


unsigned int signature_raw_file_hash(char *file_name)
{
	FILE *signature_file = NULL;
	char  buffer[1 * 1024];
	char  *data = NULL;
	unsigned int hash_value, signature;
	unsigned int h_value[4], s_value[4];
	unsigned int length, read_bytes;
	int   i, ret = -1;

	signature_file = fopen(file_name, "rb+");
	if(!signature_file)
	{
		printf("signature error : unable to open signature file\n");

		goto signature_raw_hash_err;
	}
	fseek(signature_file, 0, SEEK_END);
	length = ftell(signature_file);
	fseek(signature_file, 0, SEEK_SET);
	if(!length)
	{
		printf("signature error : signature file length is zero\n");

		goto signature_raw_hash_err;
	}
	printf("file size  %d bytes\n", length);
	//读出前1k
	fread(buffer, 1 * 1024, 1, signature_file);
	rewind(signature_file);

	data = (unsigned char *)malloc(HASH_BUFFER_BYTES * 2);
	if(!data)
	{
		printf("signature error : unable to malloc memory to store data\n");

		goto signature_raw_hash_err;
	}
	memset(data, 0, HASH_BUFFER_BYTES * 2);

	HashString_init();

	read_bytes = sizeof(struct fastboot_boot_img_hdr);
	fseek(signature_file, 0, SEEK_SET);
	fread(data, read_bytes, 1, signature_file);
	hash_value = HashString(data, 1, read_bytes);	//1类hash

	read_bytes = sizeof(struct image_header);
	fseek(signature_file, CFG_FASTBOOT_MKBOOTIMAGE_SECTOR*512, SEEK_SET);
	fread(data, read_bytes, 1, signature_file);
	hash_value = HashString(data, 1, read_bytes);	//1类hash

	rsa_init();
	h_value[0] = (hash_value>>0) & 0xff;
	h_value[1] = (hash_value>>8) & 0xff;
	h_value[2] = (hash_value>>16) & 0xff;
	h_value[3] = (hash_value>>24) & 0xff;

	rsa_encrypt( h_value, 4, s_value);
	*(unsigned int *)(buffer + 608) = s_value[0];
	*(unsigned int *)(buffer + 612) = s_value[1];
	*(unsigned int *)(buffer + 616) = s_value[2];
	*(unsigned int *)(buffer + 620) = s_value[3];

	debug("hash value %x %x %x %x\n", h_value[0], h_value[1], h_value[2], h_value[3]);
	debug("rsa value %x %x %x %x\n", s_value[0], s_value[1], s_value[2], s_value[3]);

	rewind(signature_file);
	fwrite(buffer, 1024, 1, signature_file);

	ret = 0;

signature_raw_hash_err:
	if(data)
	{
		free(data);
	}
	if(signature_file)
	{
		fclose(signature_file);
	}

	return ret;
}


unsigned int signature_sparse_file_hash(char *sfile_name, char *vfile_name)
{
	FILE *signature_file = NULL;
	FILE *verified_file = NULL;
	char  buffer[1 * 1024];
	char  *data = NULL;
	unsigned int hash_value, signature;
	unsigned int h_value[4], s_value[4];
	unsigned int length, read_bytes;
	int   i, ret = -1;

	verified_file = fopen(vfile_name, "rb");
	if(!verified_file)
	{
		printf("signature error : unable to open signature file\n");

		goto signature_sparse_hash_err;
	}
	debug("open %s ok\n", vfile_name);
	fseek(verified_file, 0, SEEK_END);
	length = ftell(verified_file);
	fseek(verified_file, 0, SEEK_SET);
	if(!length)
	{
		printf("signature error : signature file length is zero\n");

		goto signature_sparse_hash_err;
	}
	printf("file size %d bytes\n", length);

	data = (unsigned char *)malloc(HASH_BUFFER_BYTES * 2);
	if(!data)
	{
		printf("signature error : unable to malloc memory to store data\n");

		goto signature_sparse_hash_err;
	}
	memset(data, 0, HASH_BUFFER_BYTES * 2);

	HashString_init();

	debug("begin to load data\n");

	{
		struct ext4_super_block  *sblock;

		read_bytes = sizeof(struct ext4_super_block);
		fseek(verified_file, CFG_SUPER_BLOCK_SECTOR * 512, SEEK_SET);
		fread(data, read_bytes, 1, verified_file);
		sblock = (struct ext4_super_block *)data;
		sblock->s_mtime     = CFG_SUPER_BLOCK_STAMP_VALUE;
		sblock->s_mnt_count = CFG_SUPER_BLOCK_STAMP_VALUE & 0xffff;
		memset(sblock->s_last_mounted, 0, 64);
		hash_value = HashString(data, 1, (unsigned int)&(((struct ext4_super_block *)0)->s_snapshot_list));	//1类hash
	}

//	for(i=0;i<PARTITION_SYSTEM_VERIFY_COUNT;i++)
//	{
//		fread(data, HASH_BUFFER_BYTES, 1, verified_file);
//		hash_value = HashString(data, 1, HASH_BUFFER_BYTES);	//1类hash
//		fseek(verified_file, PARTITION_SYSTEM_VERIFY_STEP - HASH_BUFFER_BYTES, SEEK_CUR);
//	}
	fclose(verified_file);
	verified_file = NULL;
	debug("data load ok\n");

	debug("hash value %x\n", hash_value);
	rsa_init();

	h_value[0] = (hash_value>>0) & 0xff;
	h_value[1] = (hash_value>>8) & 0xff;
	h_value[2] = (hash_value>>16) & 0xff;
	h_value[3] = (hash_value>>24) & 0xff;

	rsa_encrypt( h_value, 4, s_value);

	debug("hash value %x %x %x %x\n", h_value[0], h_value[1], h_value[2], h_value[3]);
	debug("rsa value %x %x %x %x\n", s_value[0], s_value[1], s_value[2], s_value[3]);

	debug("begin to open file %s\n", sfile_name);
	signature_file = fopen(sfile_name, "rb+");
	if(!signature_file)
	{
		printf("signature error : unable to open signature file\n");

		goto signature_sparse_hash_err;
	}
	fseek(signature_file, 0, SEEK_END);
	length = ftell(signature_file);
	fseek(signature_file, 0, SEEK_SET);
	if(!length)
	{
		printf("signature error : signature file length is zero\n");

		goto signature_sparse_hash_err;
	}
	//读出前1k
	fread(buffer, 1 * 1024, 1, signature_file);
	rewind(signature_file);
	*(unsigned int *)(buffer + 1000) = s_value[0];
	*(unsigned int *)(buffer + 1004) = s_value[1];
	*(unsigned int *)(buffer + 1008) = s_value[2];
	*(unsigned int *)(buffer + 1012) = s_value[3];

	fwrite(buffer, 1 * 1024, 1, signature_file);

	ret = 0;

signature_sparse_hash_err:
	if(data)
	{
		free(data);
	}
	if(verified_file)
	{
		fclose(verified_file);
	}
	if(signature_file)
	{
		fclose(signature_file);
	}

	return ret;
}


