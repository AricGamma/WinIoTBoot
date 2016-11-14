/** @file
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

#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/SerialPortLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Interinc/sunxi_uefi.h>
#include <Library/SysConfigLib.h>
#include <Include/Sun50iW1P1/ccmu.h>
#include <Include/Sun50iW1P1/uart.h>
#include "sw_uart.h"

#define   SERIAL_PORT    serial_ports[serial_index]

static sw_uart_t serial_ports[CFG_SW_SERIAL_MAX] = {
  (sw_uart_t)CFG_SW_SERIAL_COM0,
  (sw_uart_t)CFG_SW_SERIAL_COM1,
  (sw_uart_t)CFG_SW_SERIAL_COM2,
  (sw_uart_t)CFG_SW_SERIAL_COM3,
  (sw_uart_t)CFG_SW_SERIAL_COM4

};

static int serial_index = 0;

/*

  Programmed hardware of Serial port.

  @return    Always return EFI_UNSUPPORTED.

**/
RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{ 
  INT32 apb_clk = 24000000;
  INT32 baudrate = 115200;
  INT32 df,i;
  UINT32 Reg;
  normal_gpio_cfg* uart_gpio_cfg;
  UINT32 uart_port;
  
  struct spare_boot_head_t* spare_head =get_spare_head(PcdGet32 (PcdFdBaseAddress));

  uart_gpio_cfg = (normal_gpio_cfg *)spare_head->boot_data.uart_gpio;
  uart_port = spare_head->boot_data.uart_port;
    
  //reset
  Reg = MmioRead32(CCMU_BUS_SOFT_RST_REG4);
  Reg &= ~(1<<(CCM_UART_PORT_OFFSET + uart_port));
  MmioWrite32(CCMU_BUS_SOFT_RST_REG4,Reg);
  for( i = 0; i < 100; i++ );
  Reg |=  (1<<(CCM_UART_PORT_OFFSET + uart_port));
  MmioWrite32(CCMU_BUS_SOFT_RST_REG4,Reg);
  //gate
  Reg = MmioRead32(CCMU_BUS_CLK_GATING_REG3);
  Reg &= ~(1<<(CCM_UART_PORT_OFFSET + uart_port));
  MmioWrite32(CCMU_BUS_CLK_GATING_REG3,Reg);
  for( i = 0; i < 100; i++ );
  Reg |=  (1<<(CCM_UART_PORT_OFFSET + uart_port));
  MmioWrite32(CCMU_BUS_CLK_GATING_REG3,Reg);
  //gpio
  //gpio_request_early(uart_gpio_cfg,2,1);
  
  df =  (( apb_clk) + (baudrate << 3)) / (baudrate << 4) ; // (UART_APBCLK )/(16*UART_BAUD)
  if(sw_uart_init(serial_ports[uart_port], uart_port, (void*)uart_gpio_cfg, df) != -1)
  {
    serial_index = uart_port;
    return RETURN_SUCCESS;
  }
  else
  {
    return RETURN_DEVICE_ERROR;
  }
  
}

/**
  Write data to serial device.

  @param  Buffer           Point of data buffer which need to be writed.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Write data failed.
  @retval !0               Actual number of bytes writed to serial device.

**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
)
{
  UINTN   Count;
  for (Count = 0; Count < NumberOfBytes; Count++, Buffer++) {
    if( *Buffer == '\n' )                      // if current character is '\n', insert and output '\r'
    sw_uart_putc( SERIAL_PORT, '\r' );
    sw_uart_putc( SERIAL_PORT,*Buffer);
  }

  return NumberOfBytes;
}


/**
  Read data from serial device and save the datas in buffer.

  @param  Buffer           Point of data buffer which need to be writed.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Read data failed.
  @retval !0               Aactual number of bytes read from serial device.

**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes
)
{

  UINTN   Count;
    
  for (Count = 0; Count < NumberOfBytes; Count++, Buffer++) {
    *Buffer = sw_uart_getc(SERIAL_PORT);
  }
  return NumberOfBytes;
}


/**
  Check to see if any data is avaiable to be read from the debug device.

  @retval EFI_SUCCESS       At least one byte of data is avaiable to be read
  @retval EFI_NOT_READY     No data is avaiable to be read
  @retval EFI_DEVICE_ERROR  The serial device is not functioning properly

**/
BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  return sw_uart_tstc(SERIAL_PORT);
}

