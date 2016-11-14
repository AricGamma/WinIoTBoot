#ifndef _UART_H_
#define _UART_H_

#define NULL ((void*)0)
#define CCM_UART_PORT_OFFSET          16
#define CCM_UART_ADDR_OFFSET          0x400

typedef struct serial_hw
{
	volatile unsigned int rbr;		/* 0 */
	volatile unsigned int ier;		/* 1 */
	volatile unsigned int fcr;		/* 2 */
	volatile unsigned int lcr;		/* 3 */
	volatile unsigned int mcr;		/* 4 */
	volatile unsigned int lsr;		/* 5 */
	volatile unsigned int msr;		/* 6 */
	volatile unsigned int sch;		/* 7 */
}serial_hw_t;


#define   UART_BAUD    115200      // Baud rate for UART
                                   // Compute the divisor factor
// UART Line Control Parameter
#define   PARITY       0           // Parity: 0,2 - no parity; 1 - odd parity; 3 - even parity
#define   STOP         0           // Number of Stop Bit: 0 - 1bit; 1 - 2(or 1.5)bits
#define   DLEN         3           // Data Length: 0 - 5bits; 1 - 6bits; 2 - 7bits; 3 - 8bits

#if DEBUG
void sunxi_serial_init(int uart_port, void *gpio_cfg, int gpio_max);
void sunxi_serial_exit(void);
void sunxi_serial_putc (char c);
char sunxi_serial_getc (void);
int sunxi_serial_tstc (void);
#else
static inline void sunxi_serial_init(int uart_port, void *gpio_cfg, int gpio_max) {}
static inline void sunxi_serial_exit(void) {}
static inline void sunxi_serial_putc(char c) {}
static inline char sunxi_serial_getc(void) { return 0;}
static inline int sunxi_serial_tstc(void) { return 0;}
#endif


#endif    /*  #ifndef _UART_H_  */
