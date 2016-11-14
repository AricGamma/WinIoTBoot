# AllWinner UEFI Firmware Release Notes
---

## Contents:

- [1. Introduction](#1)
- [2. Source code](#2)
- [3. Build AllWinner UEFI Image](#3)
- [4. Porting guide](#4)
- [5. Flash UEFI Image](#5)

---

## `[TOC]`1. Introduction

This document describes how to build AllWinner UEFI Firmware and bring a AllWinner dev board up with it. 
The AllWinner UEFI Firmware, as the major boot system for AllWinner-Windows platform, 
is based on the EDK2 framework which is open-source and with some AllWinner platform software packages injected.

### Features of this version:
1. This version can only support two boards based on AllWinner A64 platform which is Pine64 and Banana Pi M64;
2. Need to build and pack the firmware via a Linux shell;
3. At present, it can only bring up Windows 10 IoT Core.

About UEFI and EDK2 open source project, please refer to links below:

- UEFI: [http://www.uefi.org/](http://www.uefi.org/)
- EDK2: [http://www.tianocore.org/](http://www.tianocore.org)

## [2. Source code](#$2)

Please get the source code of AllWinner UEFI firmware from [here](http://link).

### Code structure

---

#### Subdirectory of firmware:

        firmware
            |__arm-trusted-firmware-1.0 
            |__edk2

>**arm-trusted-firmware-1.0:** This directory contains firmware related to security of ARM platform. We just integrated the BL31 image into our firmware. 
>   
>**edk2:** This directory contains all of the UEFI code and script used to build a firmware. And all of the AllWinner-related files is located in the *SunxiPlatformPkg* directory.  
  
---
#### AllWinner software packages directory:

        SunxiPlatformPkg
            |__Apps                             Some common app images in UEFI(such as a boot menu)
            |__Bds                              Some function related to booting an OS
            |__build.bat                        Script to build AllWinner UEFI under Windows OS
            |__build.sh                         Script to build AllWinner UEFI under Linux OS
            |__DebuggerScripts                  Scripts related to debugging DS5
            |__Driver                           Some common driver image in UEFI
            |__Include                          Header files
            |__Library                          Some common library
            |__MemoryInitPei                    Memory initialization code during Prepi
            |__pack                             AllWinner packages output directory
            |__pctools                          Some tools used to package
            |__PlatformPei                      Prepi initialization code
            |__PrePi                            Boot program entry code
            |__sun50iw1p1Pkg                    Some code related to AllWinner A64 SoC
            |__SunxiPlatformPkg.dec             DEC description file of AllWinner packages
            |__SunxiPlatformPkg.dsc             DSC description file of AllWinner packages
            |__SunxiPlatformPkg.dsc.inc         DSC description file of AllWinner packages
            |__SunxiSpl                         Boot0 and FES code of AllWinner

>All source codes and configuration files related to AllWinner platform that include applications, drivers and libraries of EDK2 are kept in the *SunxiPlatformPkg* directory. In addition, there are also some configuration files and tools used to package inside. Only the *sun50iw1p1Pkg* subdirectory is related to a specific SoC(sun50iw1p1Pkg represents AllWinner A64).

---

#### Code related to AllWinner platform:

        sun50iw1p1Pkg
            |__boot0Pkg.dsc                 DSC description file of boot0
            |__boot0Pkg.fdf                 FDF description file of boot0
            |__Driver                       Include drivers related to A64 platform
            |__fesPkg.dsc                   DSC description file of FES
            |__fesPkg.fdf                   FDF description of FES
            |__Include                      Include some header files related to A64 platform
            |__Library                      Include some libraries related to A64 platform                                |__sun50iw1p1Pkg.dec            DEC description file of A64 platform
            |__sun50iw1p1Pkg.dsc            DSC description file of A64 platfor
            |__sun50iw1p1Pkg.fdf            FDF description file of A64 platfom


>This directory contains all source and configuration files related to AllWinner A64 platform exactly. You can copy and modify to build for another platform you wanna bring up with UEFI.

---

#### Directory of image generation:

        pack
            |__chips                Some images and configuration files related to a specific SoC
            |__common               Include common images and configuration files
            |__npack                Include scripts of packaging
            |__pctools              Include some tools used to generate a package

>All the files in this directory and its subdirectory are used to generate a package.

---

## [3. Build AllWinner UEFI image]($#3)

There are two steps to build a UEFI image. Firstly, build a FD image via the EDK2 framework and copy that image to *./edk2/SunxiPlatformPkg/pack/chips*. Secondly, during packaging stage, after configured the FD image by the configuration files of a specific platform under *./edk2/SunxiPlatformPkg/pack/chips/[$chip_alias]/configs/[$board_alias]* directory, we combine the FD image with arm trusted firmware to generate a *boot_package.fex*.

### **Steps:**

>**Note:** At present, the build.bat script is not available. So you need to build the firmware under a Linux OS.
#### 1. Stage of build
a. Navigate to *./edk2/SunxiPlatformPkg* directory under a Shell and type `./build.sh` to start the build process;  
b. Select the platfom of chip and board.


#### 2. Stage of package
a. Navigate to *./edk2/SunxiPlatformPkg* directory and type `./build.sh pack` to generate the firmware;  
b. Get the *boot_package.fex* generated under *./edk2/SunxiPlatformPkg/pack/out*.  
       
>**PS:** If you have modified the code of boot0 or FES, you need to rebuild boot0 or FES and pack.  
>Please follow steps below:  
>1. Build boot0:  
>       a. Navigate to `SunxiPlatformPkg` directory and execute `./build.sh boot0` ;  
>       b. Execute package command `./build.sh pack` ;  
>       c. Get boot0 file named *boot0_sdcard.fex* under the *./pack/out/* directory.  
>2. Build FES:  
>       Just the same as "build boot0" except the param of step a need to be "fes"(./build.sh fes).


 ## [4. Porting guide]($#4) 

 ### 1. How to modify configurations of specific board    
 
 Navigate to "./edk2/SunxiPlatformPkg/pack/chips/[*SOC_NAME*]/configs/[*BOARD_ALIAS*]/" directory and you can find a file named `sys_config_windows.fex`. Modify this file to customize your board and you don't need to rebuild the project. Just re-package a new UEFI image by executing `./build.sh pack`.    


 ### 2. Configure GPT partition table  

 Navigate to "./edk2/SunxiPlatformPkg/pack/chips/[*SOC_NAME*]/configs/[*BOARD_ALIAS*]/" directory and get the file named `sys_gpt_windows.fex` to add, delete or modify the partitions. Also you don't need to rebuild all the project but just re-package.  


 ### 3. Configure SMBIOS

 SMBIOS configurations can be found in `sys_config_windows.fex` and please refer to 1st item.  


 ### 4. Add a new module to UEFI  

 If you want to add an additional module (e.g. App, Driver or Library) to AllWinner UEFI Firmware, just follow the next steps:
 
 1. Create a new directory for your module under "SunxiPlatformPkg" or "sun50iw1p1Pkg" directory;

 2. Add your source code and configuration files into your module directory;

 3. Add your configuration files to the configurations(.dsc and .fdf) of the specific platfom.

 A example of creating an additional app module:  

 1. Create a directory named "HelloWorld" under `SunxiPlatformPkg/Apps`;

 2. Create a source file named "hello.c" under "HelloWorld" directory, and the source code like this:  

  ```C
  #include <Uefi.h>
  
  EFI_STATUS UefiMain(
          IN EFI_HANDLE ImageHandle,
          IN EFI_SYSTEM_TABLE *SystemTable)
  {
          SystemTable
                ->ConOut
                ->OutputString(
                        SystemTable->ConOut, 
                        L"\nHello World\n");
          return EFI_SUCCESS;
  }
  ```

  3. Create a configuration file named "hello.inf" and fill it by code below:

  ```
  [Defines]
  INF_VERSION    = 0x00010005
  BASE_NAME      = HelloTest
  FILE_GUID      = 25C1528D-4197-4E2C-855C-500BA175F303
  MODULE_TYPE    = UEFI_APPLICATION
  VERSION_STRING = 1.0
  ENTRY_POINT    = UefiMain
  
  [Sources]
  Hello.c
  
  [Packages]
  MdePkg/MdePkg.dec
  
  [LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib
  ```

  4. Add "hello.inf" into configurations of "sun50iw1p1Pkg" project.

        a. Open "sun50iw1p1Pkg.dsc" and add `SunxiPlatformPkg/Apps/Hello/Hello.inf` under `[Components.common]`;  

        b. Open "sun50iw1p1Pkg.fdf" and add `INF SunxiPlatformPkg/Apps/Hello/Hello.inf` under `[FV.FvMain]`.  


## [5. Flash UEFI Image]($#5)

Please refer to this [doc](https://github.com/Leeway213/Win10-IoT-for-A64-Release-Notes/blob/master/doc/How%20to%20flash%20UEFI%20image.md).



