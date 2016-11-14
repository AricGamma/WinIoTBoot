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

#ifndef _SW_UART_H_
#define _SW_UART_H_  1

#define CFG_SW_SERIAL_MAX      6
#define CFG_SW_SERIAL_COM0	   SUNXI_UART0_BASE	      /* uart0 */
#define CFG_SW_SERIAL_COM1	   SUNXI_UART1_BASE	      /* uart1 */
#define CFG_SW_SERIAL_COM2	   SUNXI_UART2_BASE	      /* uart2 */
#define CFG_SW_SERIAL_COM3	   SUNXI_UART3_BASE	      /* uart3 */
#define CFG_SW_SERIAL_COM4	   SUNXI_UART4_BASE	      /* uart4 */


struct sw_uart
{
    volatile unsigned int rbr;       /* RBR, THR, DLL */
    volatile unsigned int dlh;       /* DLH, IER      */
    volatile unsigned int iir;       /* IIR, FCR      */
    volatile unsigned int lcr;       /* LCR           */
    volatile unsigned int mcr;       /* MCR           */
    volatile unsigned int lsr;       /* LSR           */
    volatile unsigned int msr;       /* MSR           */
    volatile unsigned int sch;       /* SCH           */
    volatile unsigned int dump[23];  /* reserved bytes*/
    volatile unsigned int usr;       /* USR           */
    volatile unsigned int tfl;       /* TFL           */
    volatile unsigned int rfl;       /* RFL           */
};

typedef volatile struct sw_uart *sw_uart_t;

#define thr rbr
#define dll rbr
#define ier dlh
#define fcr iir

#define FCR_FIFO_EN     0x01		/* Fifo enable */
#define FCR_RXSR        0x02		/* Receiver soft reset */
#define FCR_TXSR        0x04		/* Transmitter soft reset */

#define MCR_DTR         0x01
#define MCR_RTS         0x02
#define MCR_DMA_EN      0x04
#define MCR_TX_DFR      0x08

#define LCR_WLS_MSK	    0x03		/* character length select mask */
#define LCR_WLS_5	    0x00		/* 5 bit character length */
#define LCR_WLS_6	    0x01		/* 6 bit character length */
#define LCR_WLS_7	    0x02		/* 7 bit character length */
#define LCR_WLS_8	    0x03		/* 8 bit character length */
#define LCR_STB		    0x04		/* Number of stop Bits, off = 1, on = 1.5 or 2) */
#define LCR_PEN		    0x08		/* Parity eneble */
#define LCR_EPS		    0x10		/* Even Parity Select */
#define LCR_STKP	    0x20		/* Stick Parity */
#define LCR_SBRK	    0x40		/* Set Break */
#define LCR_BKSE	    0x80		/* Bank select enable */

#define LSR_DR		    0x01		/* Data ready */
#define LSR_OE		    0x02		/* Overrun */
#define LSR_PE		    0x04		/* Parity error */
#define LSR_FE		    0x08		/* Framing error */
#define LSR_BI		    0x10		/* Break */
#define LSR_THRE	    0x20		/* Xmit holding register empty */
#define LSR_TEMT	    0x40		/* Xmitter empty */
#define LSR_ERR		    0x80		/* Error */

#define RXFIFO_FULL     0x10      /* Rx Fifo check full  */
#define RXFIFO_EMPTY    0x08      /* Rx Fifo check empty */
#define TXFIFO_EMPTY    0x04      /* Tx Fifo check empty */
#define TXFIFO_FULL     0x02      /* Tx Fifo check full  */
#define UART_BUSY       0x01      /* uart control busy   */

#define LCRVAL          0x03					                /* 8 data, 1 stop, no parity */
#define MCRVAL          (MCR_DTR | MCR_RTS)			        /* RTS/DTR */
#define FCRVAL          (FCR_FIFO_EN | FCR_RXSR | FCR_TXSR)	/* Clear & enable FIFOs */


/* useful defaults for LCR */
#define LCR_8N1		0x03

int 	sw_uart_init   (sw_uart_t com_port, int uart_port, void  *uart_ctrl, int baud_divisor);
int     sw_uart_exit   (int       uart_port);
void	sw_uart_putc   (sw_uart_t com_port, char c);
char	sw_uart_getc   (sw_uart_t com_port);
int		sw_uart_tstc   (sw_uart_t com_port);
void	sw_uart_reinit (sw_uart_t com_port, int baud_divisor);

#endif   /* _SW_UART_H_ */

/* end of sw_uart.h */
