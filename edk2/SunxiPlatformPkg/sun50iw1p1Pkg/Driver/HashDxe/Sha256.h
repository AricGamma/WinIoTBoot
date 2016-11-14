/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Martin.Zheng <martinzheng@allwinnertech.com>
*  
*  This program and the accompanying materials                          
*  are licensed and made available under the terms and conditions of the BSD License         
*  which accompanies this distribution.  The full text of the license may be found at        
*  http://opensource.org/licenses/bsd-license.php                                            
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
*
**/


#ifndef SHA256_H
#define SHA256_H

//#define SHA256_DIGEST_SIZE ( 256/ 8)
#define SHA256_BLOCK_SIZE  ( 512 / 8)
typedef unsigned char uint8;
typedef unsigned int  uint32;


typedef struct sha256_ctx{
    unsigned int tot_len;
    unsigned int len;
    unsigned char block[2 * SHA256_BLOCK_SIZE];
    unsigned int h[8];
} sha256_ctx;


void sha256_init(sha256_ctx * ctx);
void sha256_update(sha256_ctx *ctx, const unsigned char *message,unsigned int len);
void sha256_final(sha256_ctx *ctx, unsigned char *digest);
extern void sha256(const unsigned char *message, unsigned int len,unsigned char *digest);




#endif /* !SHA2_H */


