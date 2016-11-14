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

#ifndef _BOOT0_HELPER_H_
#define _BOOT0_HELPER_H_

//#define readl(addr)     MmioRead32(addr)
//#define writel(value,addr)  MmioWrite32(addr,value)
#define readl(addr) (*(volatile unsigned int *) (addr))
#define writel(val, addr) ((*(volatile unsigned int *) (addr)) = (val))

extern void MmuSetup(unsigned int dram_size);
extern void  MmuTurnOff( void );
extern int LoadBoot1(void);
extern void SetDramPara(void *dram_addr , unsigned int dram_size, unsigned int boot_cpu);
//extern void Boot0Jump(unsigned int addr);
extern void Boot0JmpBoot1(unsigned int addr);
extern void Boot0JmpOther(unsigned int addr);
extern void Boot0JmpMonitor(void);
extern void ResetPll( void );
extern int LoadFip(int *use_monitor);
extern void SetDebugmodeFlag(void);
extern void SetPll( void );
extern void UpdateFlashPara(void);

extern int timer_init(void);
extern void __msdelay(unsigned long ms);
extern void __usdelay(unsigned long us);

extern void sunxi_serial_init(int uart_port, void *gpio_cfg, int gpio_max);
extern int boot_set_gpio(void  *user_gpio_list, unsigned int group_count_max, int set_gpio);
extern char sunxi_serial_getc (void);
extern int sunxi_serial_tstc (void);
extern unsigned int rtc_region_probe_fel_flag(void);
extern void rtc_region_clear_fel_flag(void);


extern int printf(const char *fmt, ...);
extern void * memcpy(void * dest,const void *src,unsigned int count);
extern void * memset(void * s, int c, unsigned int count);
extern void * memcpy_align16(void * dest,const void *src,unsigned int count);
extern int strncmp(const char * cs,const char * ct,unsigned int count);
extern char * strcpy(char * dest,const char *src);


#endif
