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


#include "Sha256.h"
#include <Library/BaseMemoryLib.h>


#define SHFR(x, n)    (x >> n)
#define ROTR(x, n)   ((x >> n) | (x << (32 - n)))
#define ROTL(x, n)   ((x << n) | (x >> (32 - n)))
#define CH(x, y, z)  ((x & y) ^ (~x & z))
#define MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))

#define SHA256_F1(x) (ROTR(x,  2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SHA256_F2(x) (ROTR(x,  6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SHA256_F3(x) (ROTR(x,  7) ^ ROTR(x, 18) ^ SHFR(x,  3))
#define SHA256_F4(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHFR(x, 10))

uint32 sha256_k[64] =
            {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
             0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
             0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
             0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
             0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
             0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
             0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
             0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
             0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
             0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
             0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
             0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
             0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
             0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
             0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
             0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};



#define UNPACK32(x, str)                      \
{                                             \
    *((str) + 3) = (uint8) ((x)      );       \
    *((str) + 2) = (uint8) ((x) >>  8);       \
    *((str) + 1) = (uint8) ((x) >> 16);       \
    *((str) + 0) = (uint8) ((x) >> 24);       \
}

#define PACK32(str, x)                        \
{                                             \
    *(x) =   ((uint32) *((str) + 3)      )    \
           | ((uint32) *((str) + 2) <<  8)    \
           | ((uint32) *((str) + 1) << 16)    \
           | ((uint32) *((str) + 0) << 24);   \
}



#define SHA256_sequence(i)                    \
{                                             \
    w[i] =  SHA256_F4(w[i -  2]) + w[i -  7]  \
          + SHA256_F3(w[i - 15]) + w[i - 16]; \
}



#define SHA256_EXP(a, b, c, d, e, f, g, h, j)               \
{                                                           \
    t1 = wv[h] + SHA256_F2(wv[e]) + CH(wv[e], wv[f], wv[g]) \
         + sha256_k[j] + w[j];                              \
    t2 = SHA256_F1(wv[a]) + MAJ(wv[a], wv[b], wv[c]);       \
    wv[d] += t1;                                            \
    wv[h] = t1 + t2;                                        \
}



uint32 sha256_h0[8] =
            {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
             0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};



void sha256_transf(sha256_ctx *ctx, const unsigned char *message,
                   unsigned int block_nb)
{
  uint32 w[64];
  uint32 wv[8];
  uint32 t1, t2;
  const unsigned char *sub_block;
  int i;
  int j;


  for (i = 0; i < (int) block_nb; i++) {
    sub_block = message + (i << 6);

    for (j = 0; j < 16; j++) {
      PACK32(&sub_block[j << 2], &w[j]);
    }

    for (j = 16; j < 64; j++) {
      SHA256_sequence(j);
    }

    for (j = 0; j < 8; j++) {
      wv[j] = ctx->h[j];
    }

    for (j = 0; j < 64; j++) {
      t1 = wv[7] + SHA256_F2(wv[4]) + CH(wv[4], wv[5], wv[6])
          + sha256_k[j] + w[j];
      t2 = SHA256_F1(wv[0]) + MAJ(wv[0], wv[1], wv[2]);
      wv[7] = wv[6];
      wv[6] = wv[5];
      wv[5] = wv[4];
      wv[4] = wv[3] + t1;
      wv[3] = wv[2];
      wv[2] = wv[1];
      wv[1] = wv[0];
      wv[0] = t1 + t2;
    }

    for (j = 0; j < 8; j++) {
      ctx->h[j] += wv[j];
    }

  }
}


void sha256_init(sha256_ctx *ctx)
{
  int i;
  for (i = 0; i < 8; i++) {
    ctx->h[i] = sha256_h0[i];
  }
  ctx->len = 0;
  ctx->tot_len = 0;
}

void sha256_update(sha256_ctx *ctx, const unsigned char *message,
                   unsigned int len)
{
  unsigned int block_nb;
  unsigned int new_len, rem_len, tmp_len;
  const unsigned char *shifted_message;

  tmp_len = SHA256_BLOCK_SIZE - ctx->len;
  rem_len = len < tmp_len ? len : tmp_len;

  CopyMem(&ctx->block[ctx->len], message, rem_len);

  if (ctx->len + len < SHA256_BLOCK_SIZE) {
    ctx->len += len;
    return;
  }

  new_len = len - rem_len;
  block_nb = new_len / SHA256_BLOCK_SIZE;

  shifted_message = message + rem_len;

  sha256_transf(ctx, ctx->block, 1);
  sha256_transf(ctx, shifted_message, block_nb);

  rem_len = new_len % SHA256_BLOCK_SIZE;

  CopyMem(ctx->block, &shifted_message[block_nb << 6],
         rem_len);

  ctx->len = rem_len;
  ctx->tot_len += (block_nb + 1) << 6;
}

void sha256_final(sha256_ctx *ctx, unsigned char *digest)
{
  unsigned int block_nb;
  unsigned int pm_len;
  unsigned int len_b;

  //int i;

  block_nb = (1 + ((SHA256_BLOCK_SIZE - 9)
                   < (ctx->len % SHA256_BLOCK_SIZE)));

  len_b = (ctx->tot_len + ctx->len) << 3;
  pm_len = block_nb << 6;

  SetMem(ctx->block + ctx->len, pm_len - ctx->len,0);
  ctx->block[ctx->len] = 0x80;
  UNPACK32(len_b, ctx->block + pm_len - 4);

  sha256_transf(ctx, ctx->block, block_nb);

  //  for (i = 0 ; i < 8; i++) {
  //      UNPACK32(ctx->h[i], &digest[i << 2]);
  //  }
  CopyMem(digest,&ctx->h[0],sizeof(int)*8);
}

void sha256(const unsigned char *message, unsigned int len, unsigned char *digest)
{
  sha256_ctx ctx;

  sha256_init(&ctx);
  sha256_update(&ctx, message, len);
  sha256_final(&ctx, digest);
}

