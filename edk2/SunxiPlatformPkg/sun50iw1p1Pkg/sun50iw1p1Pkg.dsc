#  
#  Copyright (c) 2007-2015, Allwinner Technology Co., Ltd. All rights reserved.
#  http://www.allwinnertech.com
#
#  Martin.Zheng <martinzheng@allwinnertech.com>
#  
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#  
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = sun50iw1p1Pkg
  PLATFORM_GUID                  = 4fe82b83-9315-4ff3-8cc0-ab77ca93cb7f 
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SunxiPlatform/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = ARM|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = SunxiPlatformPkg/$(PLATFORM_NAME)/$(PLATFORM_NAME).fdf
  
!include SunxiPlatformPkg/SunxiPlatformPkg.dsc.inc

[Defines]
  DEFINE FVB_VARIABLE_ENABLE     = FALSE
  #DEFINE FVB_VARIABLE_ENABLE      = TRUE

[LibraryClasses.common]
  ArmPlatformLib|SunxiPlatformPkg/$(PLATFORM_NAME)/Library/Sun50iW1P1Lib/Sun50iW1P1.inf
  
  ArmGicLib|ArmPkg/Drivers/ArmGic/ArmGicLib.inf
  
  SerialPortLib|SunxiPlatformPkg/$(PLATFORM_NAME)/Library/SerialPortLib/SerialPortLib.inf
  SerialPortExtLib|EmbeddedPkg/Library/TemplateSerialPortExtLib/TemplateSerialPortExtLib.inf
  
  #TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf 
  TimerLib|SunxiPlatformPkg/Library/TimerLib/TimerLib.inf
    
  EfiResetSystemLib|SunxiPlatformPkg/$(PLATFORM_NAME)/Library/ResetSystemLib/ResetSystemLib.inf
  RealTimeClockLib|SunxiPlatformPkg/$(PLATFORM_NAME)/Library/RealTimeClockLib/RealTimeClockLib.inf
  SunxiDmaLib|SunxiPlatformPkg/Driver/SunxiDmaDxe/SunxiDmaDxe.inf
  SmBusLib|SunxiPlatformPkg/$(PLATFORM_NAME)/Driver/Rsb/SmBusLib.inf
  SunxiTwiLib|SunxiPlatformPkg/$(PLATFORM_NAME)/Library/Sun50iTwiLib/SunxiTwiLib.inf  

  AxpPowerLib|SunxiPlatformPkg/Driver/AxpPower/Axp81X/Axp81XLib.inf

  SunxiKeyLib|SunxiPlatformPkg/$(PLATFORM_NAME)/Library/KeyLib/SunxiKeyLib.inf
  SunxiCommonLib|SunxiPlatformPkg/$(PLATFORM_NAME)/Library/CommonLib/CommonLib.inf
  GraphicsLib | SunxiPlatformPkg/Library/Graphics/Graphics.inf
  SunxiFileLib | SunxiPlatformPkg/Library/SunxiFileLib/SunxiFileLib.inf
  #Sun50iW1P1HdmiLib | SunxiPlatformPkg/sun50iw1p1Pkg/Library/Sun50iW1P1HdmiLib/Sun50iW1P1Hdmi.inf
  
!if $(TOOLCHAIN) == "ARMLINUXGCC"
  NandLib|SunxiPlatformPkg/sun50iw1p1Pkg/Library/NandLib/NandLib_GCC.inf
  EdpLib|SunxiPlatformPkg/sun50iw1p1Pkg/Library/EdpLib/EdpLib_GCC.inf
  HdcpLib|SunxiPlatformPkg/sun50iw1p1Pkg/Library/HdcpLib/HdcpLib_GCC.inf
  HdmiLib|SunxiPlatformPkg/sun50iw1p1Pkg/Library/HdmiLib/HdmiLib_GCC.inf
!else
  NandLib|SunxiPlatformPkg/sun50iw1p1Pkg/Library/NandLib/NandLib_RVCT.inf
  #EdpLib|SunxiPlatformPkg/sun50iw1p1Pkg/Library/EdpLib/EdpLib_RVCT.inf
  #HdcpLib|SunxiPlatformPkg/sun50iw1p1Pkg/Library/EdpLib/HdcpLib_RVCT.inf
  HdmiLib|SunxiPlatformPkg/sun50iw1p1Pkg/Library/HdmiLib/HdmiLib_RVCT.inf

!endif

  #NandExtLib|SunxiPlatformPkg/sun50iw1p1Pkg/Driver/Nand/NandExtLib.inf

[LibraryClasses.ARM]
  ArmLib|ArmPkg/Library/ArmLib/ArmV7/ArmV7Lib.inf
  CacheMaintenanceLib|ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf

[LibraryClasses.common.SEC]


[LibraryClasses.common.DXE_CORE]


[LibraryClasses.common.DXE_DRIVER]

[LibraryClasses.common.UEFI_APPLICATION]

[LibraryClasses.common.UEFI_DRIVER]

[LibraryClasses.common.DXE_RUNTIME_DRIVER]

[LibraryClasses.ARM]

[BuildOptions]
  XCODE:*_*_ARM_PLATFORM_FLAGS == -arch armv7 -I$(WORKSPACE)/SunxiPlatformPkg/$(PLATFORM_NAME)/Include/Sun50iW1P1

  GCC:*_*_ARM_PLATFORM_FLAGS == -march=armv7-a -mfpu=neon -I$(WORKSPACE)/SunxiPlatformPkg/$(PLATFORM_NAME)/Include/Sun50iW1P1 

  RVCT:*_*_ARM_PLATFORM_FLAGS == --cpu Cortex-A7 -I$(WORKSPACE)/SunxiPlatformPkg/$(PLATFORM_NAME)/Include/Sun50iW1P1 --licretry --diag_suppress=9931,9933 
  

  XCODE:*_*_ARM_ARCHCC_FLAGS = -DCONFIG_ARCH_SUN50IW1P1 
  
  GCC:*_*_ARM_ARCHCC_FLAGS = -DCONFIG_ARCH_SUN50IW1P1 
  
  RVCT:*_*_ARM_ARCHCC_FLAGS = -DCONFIG_ARCH_SUN50IW1P1 

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsFeatureFlag.common]

[PcdsFixedAtBuild.common]
 
  #
  # ARM Pcds
  #
  gArmTokenSpaceGuid.PcdVFPEnabled|1
  gArmTokenSpaceGuid.PcdArmMachineType|4137
      
  # Stack for CPU Cores in Non Secure Mode(128K size)
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x20000
  gArmPlatformTokenSpaceGuid.PcdCPUCoreSecondaryStackSize|0x20000
  
  # Size of the region used by UEFI in permanent memory (Reserved 64MB)
  gArmPlatformTokenSpaceGuid.PcdSystemMemoryUefiRegionSize|0x04000000
  
  gArmPlatformTokenSpaceGuid.PcdCoreCount|4
  gArmPlatformTokenSpaceGuid.PcdClusterCount|1
  
  gArmTokenSpaceGuid.PcdArmPrimaryCoreMask|0xf03
  gArmTokenSpaceGuid.PcdArmPrimaryCore|0
  
  gArmTokenSpaceGuid.PcdArmUncachedMemoryMask|0x0000000000000000
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDescription|L"boot windows from HDD"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDevicePath|L"VenHw(B615F1F5-5088-43CD-809C-A16E52487D00)/HD(1,MBR,0x00000000,0x3F,0x19FC0)/EFI/Boot/bootarm.efi"
  #gArmPlatformTokenSpaceGuid.PcdDefaultBootDevicePath|L"PciRoot(0x0)\Pci(0x0,0x0)\USB(0x0,0x0)\HD(1,MBR,0x00000000,0x800,0x1DD9000)\EFI\Boot\bootarm.efi"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootArgument|"console=tty0 console=ttyS2,115200n8 root=UUID=a4af765b-c2b5-48f4-9564-7a4e9104c4f6 rootwait ro earlyprintk"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootType|0
  gArmPlatformTokenSpaceGuid.PcdPlatformBootTimeOut|1
  #gArmPlatformTokenSpaceGuid.PcdFdtDevicePath|L""

  gArmPlatformTokenSpaceGuid.PcdDefaultConOutPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenPcAnsi()"
#  gArmPlatformTokenSpaceGuid.PcdDefaultConOutPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenPcAnsi();VenHw(9042A9DE-23DC-4A38-96FB-7ADED080516A)"

#  gArmPlatformTokenSpaceGuid.PcdDefaultConOutPaths|L"VenHw(9042A9DE-23DC-4A38-96FB-7ADED080516A)"
  gArmPlatformTokenSpaceGuid.PcdDefaultConInPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenPcAnsi()"
  
  #
  # NV Storage PCDs. Use base of 0x0C000000 for NOR1
  #
  
  #gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase|0x24000000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize|0x00100000
  #gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase|0x0FFD0000
  #gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingSize|0x00010000
  #gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase|0x0FFE0000
  #gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareSize|0x00010000
   

  #
  #Sunxi Platform Pcd
  #
  
  #
  #reserve 512MB for framebuffer at the top of the dram.
  #
  
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x41000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0X3DF80000
  
  gSunxiTokenSpaceGuid.PcdMpParkSharedBase|0x7EF80000
  gSunxiTokenSpaceGuid.PcdMpParkSharedSize|0x00080000
  
  gSunxiTokenSpaceGuid.PcdFrameBufferBase|0x7F000000
  gSunxiTokenSpaceGuid.PcdFrameBufferSize|0x1000000
  

  
  gSunxiTokenSpaceGuid.PcdGpioBase|0x01c20800
  gSunxiTokenSpaceGuid.PcdRtcBase|0x01f00000
  
  gSunxiTokenSpaceGuid.PcdScriptEarlyBase|0x43000000
  gSunxiTokenSpaceGuid.PcdCpusGpioBase|0x01f02c00

  # Timers
  gSunxiTokenSpaceGuid.PcdSunxiArchTimer|3
  gSunxiTokenSpaceGuid.PcdSunxiFreeTimer|4
  gEmbeddedTokenSpaceGuid.PcdTimerPeriod|100000
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPerformanceCounterPeriodInNanoseconds|77
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPerformanceCounterFrequencyInHz|13000000
  
  
  #
  # ARM General Interrupt Controller
  #
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x01c81000
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x01c82000
  #gArmTokenSpaceGuid.PcdGicNumInterrupts|160
  
  
  #
  # ARM Architectual Timer Frequency£¬fixed at 24M
  #
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|24000000

################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]
  
  #
  # PEI Phase modules
  #
  SunxiPlatformPkg/PrePi/PeiMPCore.inf

  #
  # DXE
  #
 
  #
  # Architectural Protocols
  #

  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf  
  
  #
  # usb host : ehci + bus + pci_emul + mass_storage
  #
  SunxiPlatformPkg/sun50iw1p1Pkg/Driver/PciEmulation/PciEmulation.inf
  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  MdeModulePkg/Bus/Usb/UsbMouseDxe/UsbMouseDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf

  #
  #soc driver for sun50iw1
  #
  SunxiPlatformPkg/Driver/SunxiDisplayDxe/DisplayV3Dxe.inf
  #SunxiPlatformPkg/Driver/EfiCheckSig/EfiCheckSigDxe.inf
  SunxiPlatformPkg/sun50iw1p1Pkg/Driver/EfiUsbFnIo/EfiUsbFnIoDxe.inf
  SunxiPlatformPkg/sun50iw1p1Pkg/Driver/EthDxe/EthDxe.inf
  #SunxiPlatformPkg/sun50iw1p1Pkg/Driver/EfiSampleWinPhoneIo/EfiSampleWinPhoneIoDxe.inf
  ##SunxiPlatformPkg/sun50iw1p1Pkg/Driver/HashDxe/HashDxe.inf
  #SunxiPlatformPkg/Driver/SunxiDmaDxe/SunxiDmaDxe.inf
  SunxiPlatformPkg/sun50iw1p1Pkg/Driver/Rsb/SmBusDxe.inf
  SunxiPlatformPkg/Driver/AxpPower/Axp81X/Axp81XDxe.inf

  SunxiPlatformPkg/Driver/Variable/RuntimeDxe/VariableRuntimeDxe.inf
  
  #
  # Multimedia Card Interface
  #
  #EmbeddedPkg/Universal/MmcDxe/MmcDxe.inf
  #SunxiPlatformPkg/sun50iw1p1Pkg/Driver/MmcHostDxe/MmcHostDxe.inf
  #SunxiPlatformPkg/Driver/SunxiSdMmcDxe/SdMmcDxe.inf
  SunxiPlatformPkg/sun50iw1p1Pkg/Driver/SunxiSdMmcDxe/SdMmcDxe.inf
  #SunxiPlatformPkg/Driver/FvbDxe/FvbDxe.inf
  #SunxiPlatformPkg/sun50iw1p1Pkg/Driver/Nand/NandDxe.inf
  SunxiPlatformPkg/Driver/BootCommandDxe/BootCommandDxe.inf
  
  #
  # ACPI Support
  #
  SunxiPlatformPkg/sun50iw1p1Pkg/Driver/AcpiTables/AcpiTables.inf
  
  #app
  SunxiPlatformPkg/Apps/LinuxLoader/LinuxLoader.inf
  SunxiPlatformPkg/Apps/Nand/NandIoTest.inf
  SunxiPlatformPkg/Apps/TestApps/SdMmc/SdMmcTest.inf
  
