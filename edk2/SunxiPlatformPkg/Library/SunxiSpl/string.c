/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  tangmanliang <tangmanliang@allwinnertech.com>
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

#include<Uefi.h>
char * strcpy(char * dest,const char *src)
{
  char *tmp = dest;

  while ((*dest++ = *src++) != '\0')
  /* nothing */;
  return tmp;
}

char * strncpy(char * dest,const char *src,unsigned int count)
{
  char *tmp = dest;

  while (count-- && (*dest++ = *src++) != '\0')
  /* nothing */;

  return tmp;
}

char * strcat(char * dest, const char * src)
{
  char *tmp = dest;

  while (*dest)
    dest++;
  while ((*dest++ = *src++) != '\0')
    ;

  return tmp;
}

char * strncat(char *dest, const char *src, unsigned int count)
{
  char *tmp = dest;

  if (count) {
    while (*dest)
      dest++;
    while ((*dest++ = *src++)) {
      if (--count == 0) {
        *dest = '\0';
        break;
      }
    }
  }

  return tmp;
}

int strcmp(const char * cs,const char * ct)
{
  register signed char __res;

  while (1) {
    if ((__res = *cs - *ct++) != 0 || !*cs++)
      break;
  }

  return __res;
}

int strncmp(const char * cs,const char * ct,unsigned int count)
{
  register signed char __res = 0;

  while (count) {
    if ((__res = *cs - *ct++) != 0 || !*cs++)
      break;
    count--;
  }

  return __res;
}

char * strchr(const char * s, int c)
{
  for(; *s != (char) c; ++s)
    if (*s == '\0')
      return NULL;
  return (char *) s;
}

unsigned int strlen(const char * s)
{
  const char *sc;

  for (sc = s; *sc != '\0'; ++sc)
  /* nothing */;
  return sc - s;
}

char * strrchr(const char * s, int c)
{
  const char *p = s + strlen(s);
  do {
    if (*p == (char)c)
      return (char *)p;
  } while (--p >= s);

  return NULL;
}


unsigned int strnlen(const char * s, unsigned int count)
{
  const char *sc;

  for (sc = s; count-- && *sc != '\0'; ++sc)
  /* nothing */;
  return sc - s;
}


unsigned int strspn(const char *s, const char *accept)
{
  const char *p;
  const char *a;
  unsigned int count = 0;

  for (p = s; *p != '\0'; ++p) {
    for (a = accept; *a != '\0'; ++a) {
      if (*p == *a)
        break;
    }
    if (*a == '\0')
      return count;
    ++count;
  }

  return count;
}

void * memset(void * s, int c, unsigned int count)
{
  char *xs = (char *) s;

  while (count--)
    *xs++ = c;

  return s;
}

void * memcpy(void *dest, const void *src, unsigned int count)
{
  unsigned long *dl = (unsigned long *)dest, *sl = (unsigned long *)src;
  char *d8, *s8;

  if (src == dest)
    return dest;

  /* while all data is aligned (common case), copy a word at a time */
  if ( (((unsigned long)dest | (unsigned long)src) & (sizeof(*dl) - 1)) == 0) {
    while (count >= sizeof(*dl)) {
      *dl++ = *sl++;
      count -= sizeof(*dl);
    }
  }
  /* copy the reset one byte at a time */
  d8 = (char *)dl;
  s8 = (char *)sl;
  while (count--)
    *d8++ = *s8++;

  return dest;
}
void * memmove(void * dest,const void *src,unsigned int count)
{
  char *tmp, *s;

  if (dest <= src) {
    tmp = (char *) dest;
    s = (char *) src;
    while (count--)
      *tmp++ = *s++;
  }
  else {
    tmp = (char *) dest + count;
    s = (char *) src + count;
    while (count--)
      *--tmp = *--s;
  }

  return dest;
}

int memcmp(const void * cs,const void * ct,unsigned int count)
{
  const unsigned char *su1, *su2;
  signed char res = 0;

  for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
    if ((res = *su1 - *su2) != 0)
      break;
  return res;
}

/*
 * find the first occurrence of byte 'c', or 1 past the area if none
 */
void * memscan(void * addr, int c, unsigned int size)
{
  unsigned char * p = (unsigned char *) addr;

  while (size) {
    if (*p == c)
      return (void *) p;
    p++;
    size--;
  }
  return (void *) p;
}

char * strstr(const char * s1,const char * s2)
{
  int l1, l2;

  l2 = strlen(s2);
  if (!l2)
    return (char *) s1;
  l1 = strlen(s1);
  while (l1 >= l2) {
    l1--;
    if (!memcmp(s1,s2,l2))
      return (char *) s1;
    s1++;
  }
  return NULL;
}

void *memchr(const void *s, int c, unsigned int n)
{
  const unsigned char *p = s;
  while (n-- != 0) {
    if ((unsigned char)c == *p++) {
      return (void *)(p-1);
    }
  }
  return NULL;
}

