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

#pragma once
#define ACPI_RESERVED       0

#pragma pack(1)

#define ACPI_GAS_ID_SYSTEM_MEMORY              0
#define ACPI_GAS_ID_SYSTEM_IO                  1
#define ACPI_GAS_ID_PCI_CONFIGURATION_SPACE    2
#define ACPI_GAS_ID_EMBEDDED_CONTROLLER        3
#define ACPI_GAS_ID_SMBUS                      4
#define ACPI_GAS_ID_FUNCTIONAL_FIXED_HARDWARE  0x7F

//
// GTDT flag definitions
//

#define GTDT_TIMER_EDGE_TRIGGERED  0x00000001
#define GTDT_TIMER_ACTIVE_LOW      0x00000002
#define GTDT_TIMER_ALWAYS_ON       0x00000004
             
                                                    
#define ACPI_DBG2_SIGNATURE  SIGNATURE_32('D', 'B', 'G', '2')


#pragma pack()
