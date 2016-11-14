@REM build.bat
@REM
@REM (c) Copyright 2014
@REM Allwinner Technology Co., Ltd. <www.allwinnertech.com>
@REM Martin.Zheng <zhengjiewen@allwinnertech.com>
@REM
@REM This program is free software; you can redistribute it and/or modify
@REM it under the terms of the GNU General Public License as published by
@REM the Free Software Foundation; either version 2 of the License, or
@REM (at your option) any later version.

@REM Example usage of this script. default is a DEBUG build
@REM b
@REM b clean
@REM b release 
@REM b release clean
@REM b -v -y build.log

@ECHO OFF
@REM Setup Build environment. Sets WORKSPACE and puts build in path
CALL %CD%\edk2\edksetup.bat

@SET BUILD_ARCH=ARM
@SET BUILD_PLATFORM=SunxiPlatformPkg
@SET BUILD_CHIP=sun50iw1p1
@SET BUILD_TOOL_CHAIN=RVCT
@SET BUILD_TYPE=DEBUG

@if /I "%1"=="RELEASE" (
  @REM If 1st argument is release set TARGET to RELEASE and shift arguments to remove it 
  SET BUILD_TYPE=RELEASE
  shift /1
)

@PUSHD .
@cd ..

@REM Build the SunxiPlatformPkg firmware and creat an FD (FLASH Device) Image.
@if  "%BUILD_TYPE%"=="RELEASE" (

	CALL build -p %WORKSPACE%\%BUILD_PLATFORM%\%BUILD_CHIP%Pkg\%BUILD_CHIP%Pkg.dsc -a %BUILD_ARCH% -t %BUILD_TOOL_CHAIN% -b %BUILD_TYPE% -n 4 -D "DEBUG_TARGET=RELEASE" %1 %2 %3 %4 %5 %6 %7 %8

) else (

	CALL build -p %WORKSPACE%\%BUILD_PLATFORM%\%BUILD_CHIP%Pkg\%BUILD_CHIP%Pkg.dsc -a %BUILD_ARCH% -t %BUILD_TOOL_CHAIN% -b %BUILD_TYPE% -n 4 %1 %2 %3 %4 %5 %6 %7 %8

)
@POPD

@if ERRORLEVEL 1 goto Exit

@SET str=%BUILD_CHIP%

@for %%i in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do @call set str=%%str:%%i=%%i%%

copy /B /Y %WORKSPACE%\Build\%BUILD_PLATFORM:~0,-3%\%BUILD_CHIP%Pkg\%BUILD_TYPE%_%BUILD_TOOL_CHAIN%\FV\%str%_EFI.fd %WORKSPACE%\%BUILD_PLATFORM%\pack\chips\%BUILD_CHIP%\bin\uefi-%BUILD_CHIP%.bin

@if /I "%1"=="CLEAN" goto Clean



:Exit
EXIT /B

:Clean
cd Tools
CALL build cleanall

