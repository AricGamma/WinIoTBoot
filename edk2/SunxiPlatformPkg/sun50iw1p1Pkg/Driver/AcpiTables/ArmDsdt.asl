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

// CreatorID=MSFT	CreatorRev=4.0.0
// FileLength=42	FileChkSum=0xff

DefinitionBlock("mindsdt.aml", "DSDT", 0x01, "AWTH", "SUN50IW1P1", 0x00000001)
{
    Scope(_SB_)
    {
    	  //
				// Use "ACPI0007" in _HID for all application processor cores
				// Each processor also requires a unique number (ACPI Unique ID) for use in the MADT (Interrupt Controller) table
				//
				Device(PRC0)
				{
					//
					// Processor 0
					//
					Name(_HID, "ACPI0007")
					Name(_UID, 0x0)		//Must match an entry in the MADT
				}
						
				Device(PRC1)
				{
					//
					// Processor 1
					//
					Name(_HID, "ACPI0007")
					Name(_UID, 0x1)		//Must match an entry in the MADT
				}

				Device(PRC2)
				{
					//
					// Processor 2
					//
					Name(_HID, "ACPI0007")
					Name(_UID, 0x2)		//Must match an entry in the MADT
				}
				
				Device(PRC3)
				{
					//
					// Processor 3
					//
					Name(_HID, "ACPI0007")
					Name(_UID, 0x3)		//Must match an entry in the MADT
				}

        Method (UCRS, 4, NotSerialized)
        {
            Name (RSRC, ResourceTemplate ()
            {
                Memory32Fixed (ReadWrite,
                    0x00000000,         // Address Base
                    0x00000000,         // Address Length
                    _Y02)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, _Y03)
                {
                    0x00000000,
                }
            })
            CreateDWordField (RSRC, \_SB.UCRS._Y02._BAS, MBAS)  // _BAS: Base Address
            CreateDWordField (RSRC, \_SB.UCRS._Y02._LEN, MBLE)  // _LEN: Length
            CreateWordField (RSRC, \_SB.UCRS._Y03._INT, INTN)  // _INT: Interrupts
            CreateField (RSRC, \_SB.UCRS._Y03._SHR, 0x02, SHRN)  // _SHR: Sharable
            Store (Arg0, MBAS)
            Store (Arg1, MBLE)
            Store (Arg2, INTN)
            Store (Arg3, SHRN)
            Return (RSRC)
        }
        
        Device (USB0) {    //USB0
            Name ( _ADR, 0x01c1a000)  // _ADR: Address
            Name (_CID, "ACPI\\PNP0D20")  // _CID: Compatible ID
            Name (_HRV, 0x00)  // _HRV: Hardware Revision
            Name (_UID, Zero)  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return ("PNP0D20")
            }
            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Store (0x01c1a000, Local0)
                Store (0x400, Local1)
                Return (UCRS (Local0, Local1, 104, 0x03))
            }
        } // Device( USB1)
        
        Device (USB1) {    //USB1
            Name ( _ADR, 0x01c1b000)  // _ADR: Address
            Name (_CID, "ACPI\\PNP0D20")  // _CID: Compatible ID
            Name (_HRV, 0x00)  // _HRV: Hardware Revision
            Name (_UID, One)  // _UID: Unique ID
            Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
            {
                Return ("PNP0D20")
            }
            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Store (0x01c1b000, Local0)
                Store (0x400, Local1)
                Return (UCRS (Local0, Local1, 106, 0x03))
            }
        } // Device( USB2)
        

	    	Device(GPI1)
		{
			//
			// GPIO controller # 1, e.g. the only GPIO controller IP on the SoC
			//	NOTE: The namespace path to this device ("\\_SB.GPI1") will appear in the GpioIo() or GpioInt()
			//	macros for all peripherals that are connected to pins on this IP (see the TUCH Device example, below)
			//
			Name(_HID, "AWTH0001")		//Causes Windows to load your driver ('GPIO Class Extension Client' driver)
			Name(_CID, "AWTH0001")
	    Name(_UID, 0)
			// 
			// The number, type and order of system resources reported in _CRS MUST MATCH requirements of the device driver for the device.
			// For the built-in Windows GPIO Class Extension, _CRS MUST be done in the following format:
			//
			Name(_CRS, ResourceTemplate ()
			{

			//dummy bank0(PA) to adapt gpio driver
			MEMORY32FIXED(ReadWrite, 0x01c20800, 0x24, )   
			
			//bank1(PB)
			MEMORY32FIXED(ReadWrite, 0x01c20824, 0x24, )   
			Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, , , ) {43}
			
			//bank2(PC)
			MEMORY32FIXED(ReadWrite, 0x01c20848, 0x24, ) 
    
			//bank3(PD)
			MEMORY32FIXED(ReadWrite, 0x01c2086c, 0x24, )
    
			//bank4(PE)
			MEMORY32FIXED(ReadWrite, 0x01c20890, 0x24, )
    
			//bank5(PF)
			MEMORY32FIXED(ReadWrite, 0x01c208B4, 0x24, )
    
			//bank6(PG)
			MEMORY32FIXED(ReadWrite, 0x01c208D8, 0x24, )			
			Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, , , ) {49}
			
			//bank7(PH)
		  MEMORY32FIXED(ReadWrite, 0x01c208FC, 0x24, )
		  Interrupt(ResourceConsumer, Level, ActiveHigh, Shared, , , ) {53}


			})//End of _CRS		
				
				Method(_GDI, 0, NotSerialized)
				{
					Return (BUffer(0x19)
					{
					/*BaseAddress:0x01c20800*/0x00, 0x08, 0xc2, 0x01,
					/*Addresslength:*/0x00,0x04,
					/*TotalGroup:0x08*/0x08,0x00,
					/*group A:0x01,0*/0x01, 0x00,
					/*group B:0x18,1*/0x0A, 0x01,
					/*group C:0x20,1*/0x13, 0x00,
					/*group D:0x20,0*/0x19, 0x00,
					/*group E:0x28,0*/0x13, 0x00,
					/*group F:0x21,1*/0x07, 0x00,
					/*group G:0x06,0*/0x0E, 0x01,
					/*group H:0x16,1*/0x0C, 0x01
					})
				}
	
		}// End of Device 'GPI1'
		
		Device(GPI2)
		{
			//
			// GPIO controller # 1, e.g. the only GPIO controller IP on the SoC
			//	NOTE: The namespace path to this device ("\\_SB.GPI1") will appear in the GpioIo() or GpioInt()
			//	macros for all peripherals that are connected to pins on this IP (see the TUCH Device example, below)
			//
			Name(_HID, "AWTH0001")		//Causes Windows to load your driver ('GPIO Class Extension Client' driver)
			Name(_CID, "AWTH0001")
	       	Name(_UID, 1)
			// 
			// The number, type and order of system resources reported in _CRS MUST MATCH requirements of the device driver for the device.
			// For the built-in Windows GPIO Class Extension, _CRS MUST be done in the following format:
			//
			Name(_CRS, ResourceTemplate ()
				{
				//bank0(PL)
				MEMORY32FIXED(ReadWrite, 0x01f02c00, 0x24, ) 
				Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {77}  
			
				})//End of _CRS		
				
				Method(_GDI, 0, NotSerialized)
				{
					Return (BUffer(0x0f)
					{
					/*BaseAddress:0x01f02c00*/0x00, 0x2c, 0xf0, 0x01,
					/*Addresslength:*/0x00,0x04,
					/*TotalGroup:0x03*/0x01,0x00,
					/*group L:0x0E,1*/0x0d, 0x01
					})
				}
	
		}// End of Device 'GPI2'
		
		Device(I2C0)
		{
			Name(_HID, "AWTH0002")								
			Name(_UID, 0x0)					
			Name(_CRS, ResourceTemplate ()
				{
					MEMORY32FIXED(ReadWrite, 0x01c2ac00, 0x3ff,	) 
					Interrupt(ResourceConsumer, Edge, ActiveLow, Exclusive, , , ) {38}
				})
				
			Method(_NUM, 0, NotSerialized)
			{
				Return (Buffer(0x2)
				{
					0x0,0x4
				})				
			}
				
			Device(TP02)
			{
			    Name(_ADR, 0)
			    Name(_HID, "AWTP0002") 
			    Name(_UID, 0)
	        Method (_STA, 0, NotSerialized)  
          {
              Return (0x0F)
          }
			    Method(_CRS, 0x0, NotSerialized)
			    {
			        Name (RBUF, ResourceTemplate ()
			        {
                  //gt82x tp address is 0x5d
			            I2CSerialBus(0x5D, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0", , )
			            GpioInt(Edge, ActiveLow, Exclusive, PullDefault, 0, "\\_SB.GPI1") {228}
			            GpioIo(Exclusive, PullDefault, 0, 25, IoRestrictionOutputOnly, "\\_SB.GPI1") {232}
			        })
			        Return(RBUF)
			    }
			}//End of device 'TP02'
			
			Device(TP04)
			{
			    Name(_ADR, 0)
			    Name(_HID, "AWTP0004")
			    Name(_UID, 0)
			    Method (_STA, 0, NotSerialized) 
          {
              Return (0x00)
          }
			    Method(_CRS, 0x0, NotSerialized)
			    {
			        Name (RBUF, ResourceTemplate ()
			        {
			            //silead tp address is 0x40
			            I2CSerialBus(0x40, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0", , )
			            GpioInt(Edge, ActiveLow, Exclusive, PullDefault, 0, "\\_SB.GPI1") {228}
			            GpioIo(Exclusive, PullDefault, 0, 25, IoRestrictionOutputOnly, "\\_SB.GPI1") {232}
			        })
			        Return(RBUF)
			    }
			}//End of device 'TP04'
		}//End of Device 'i2c0'
		
		Device(I2C1)
		{
			Name(_HID, "AWTH0002")								
			Name(_UID, 0x1)					
			Method (_STA, 0, NotSerialized) 
      {
          Return (0x00)
      }
			Name(_CRS, ResourceTemplate ()
				{
					MEMORY32FIXED(ReadWrite, 0x01c2b000, 0x3ff, ) 
					Interrupt(ResourceConsumer, Edge, ActiveLow, Exclusive, , , ) {39}
				})
				
			Method(_NUM, 0, NotSerialized)
			{
				Return (Buffer(0x2)
				{
					0x1,0x2
				})				
			}
			
		}//End of Device 'i2c1'
		
		Device(I2C2)
		{
			Name(_HID, "AWTH0002")								
			Name(_UID, 0x2)					
		  Method (_STA, 0, NotSerialized) 
      {
          Return (0x00)
      }
			Name(_CRS, ResourceTemplate ()
				{
					MEMORY32FIXED(ReadWrite, 0x01c2b400, 0x3ff, ) 
					Interrupt(ResourceConsumer, Edge, ActiveLow, Exclusive, , , ) {40}
				})
				
			Method(_NUM, 0, NotSerialized)
			{
				Return (Buffer(0x2)
				{
					0x2,0x2
				})				
			}
			

		}//End of Device 'i2c2'
		
		Device(BUTT)
	  {
			Name(_HID, "AWTH0004")
			Name(_CRS, ResourceTemplate ()
			{
				MEMORY32FIXED(ReadWrite, 0x01c21800, 0x400, ) 
				Interrupt(ResourceConsumer, Edge, ActiveLow, Exclusive, , , ) {62}
			})
		}
		
        Device(SDM0)
        {
            Name(_HID, "AWTH0005")                                
            Name(_UID, 0x0)              
            Method (_STA, 0, NotSerialized) 
            {
                Return (0x0F)
            }      
            Name(_CRS, ResourceTemplate ()
            {
                MEMORY32FIXED(ReadWrite, 0x01c0F000, 0x1000, ) 
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {92}
            })
            
            Method (_RMV, 0, NotSerialized)  // _RMV: Removal Status
            {
                Return (Zero)
            }
        
        }//End of Device 'SDM0'
        
        Device(SDM1)
        {
            Name(_HID, "AWTH0005")                                
            Name(_UID, 0x1)            
            Method (_STA, 0, NotSerialized) 
            {
                Return (0x00)
            }              
            Name(_CRS, ResourceTemplate ()
            {
                MEMORY32FIXED(ReadWrite, 0x01c10000, 0x1000, ) 
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {93}
            })
            
            Method (_RMV, 0, NotSerialized)  // _RMV: Removal Status
            {
                Return (Zero)
            }
            
            Device (WLAN)
            {
                Name (_S4W, 0x02)
                Name (_S0W, 0x02)
                Name (_ADR, One)  // _ADR: Address
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }
        
                Method (_RMV, 0, NotSerialized)  // _RMV: Removal Status
                {
                    Return (Zero)
                }
        
                Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
                {
                    Name (RBUF, ResourceTemplate ()
                    {
                        GpioInt(Edge, ActiveHigh, ExclusiveAndWake, PullDown, 0, "\\_SB.GPI2") {3}
                        GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly, "\\_SB.GPI2", 0x00, ResourceConsumer, ,) { 2 }
                    })
                    Return (RBUF)
                }
        
            }
        
        }//End of Device 'SDM1'
        
        Device(SDM2)
        {
            Name(_HID, "AWTH0005")                                
            Name(_UID, 0x2)     
            Method (_STA, 0, NotSerialized) 
            {
                Return (0x0F)
            }                  
            Name(_CRS, ResourceTemplate ()
            {
                MEMORY32FIXED(ReadWrite, 0x01c11000, 0x1000, ) 
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {94}
            })
            
            Method (_RMV, 0, NotSerialized)  // _RMV: Removal Status
            {
                Return (Zero)
            }
        
        }//End of Device 'SDM2'
		
     Device(VI2S)
	   {
	 	    Name(_HID, "AWTH0006")
        Name(_UID, 0x1)
        Name(_CRS, ResourceTemplate ()
        {                  
            FixedDMA(0, 0, Width16Bit) // DMA chnnel 0 requstline 0 as VI2S Tx here
            FixedDMA(1, 1, Width16Bit) // DMA chnnel 1 requstline 0 as VI2S Rx here
        })

	 	  }	// End of Device "CODC"
				
     
	 	  Device(CODC)
	    {
	 	  	Name(_HID, "AWTH0007")
	 	  	Name(_CRS, ResourceTemplate ()
	 	  	{
	 	  		MEMORY32FIXED(ReadWrite, 0x01c22c00, 0x800, )  //codec digital register
	 	  		MEMORY32FIXED(ReadWrite, 0x01f015c0, 0x4, )  //codec analog shadow register
	 	  		MEMORY32FIXED(ReadWrite, 0x01c20000, 0x400, )  //ccmu register to control clock
	 	  		Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {61} //audio codec interrupt 
	 	  		Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {60} //earphone detection	
	 	  	})
	 	  }	// End of Device "CODC"
				
			Device(AUDO)
			{
			   Name(_HID, "AWTH0008")
			   Name(_UID, 0x1)                           
			   Name (_DEP, Package(0x2)
         {
                \_SB_.CODC,
                \_SB_.VI2S
         })            
			}


    Device(DMAT)
	   {
	 	   Name(_HID, "AWTH0100")
       Name(_UID, 0x1)
       Method (_STA, 0, NotSerialized) 
       {
          Return (0x00)
       } 
       Name(_CRS, ResourceTemplate ()
       {   
       	  MEMORY32FIXED(ReadWrite, 0x01c20000, 0x400, )  //ccmu register to control clock               
           FixedDMA(6, 6, Width32Bit,) // DMA chnnel 6 requstline 6 as dma test device Tx here
           FixedDMA(7, 7, Width32Bit,) // DMA chnnel 7 requstline 7 as dma test device Rx here
       })
 
	 	  }	// End of Device "CODC"
				
   }//Scope(_SB_)
}
