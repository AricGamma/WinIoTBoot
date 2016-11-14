# build.sh
#
# (c) Copyright 2013
# Allwinner Technology Co., Ltd. <www.allwinnertech.com>
# Martin.Zheng <zhengjiewen@allwinnertech.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

BUILD_ARCH="ARM"
BUILD_UEFI_PKG="SunxiPlatformPkg"
BUILD_CHIP="sun8iw1p1"
BUILD_OS_PLATFORM="windows"
BUILD_BOARD="EVB"
BUILD_TYPE="DEBUG"
BUILD_TOOL_CHAIN="ARMLINUXGCC" 
BUILD_CONFIG_FILE=".buildconfig"

PACK_TOOLS_DIR=$PWD

set -e

export PATH=$PATH:$PWD/pctools
export ARMLINUXGCC_TOOLS_PATH=$PWD/pctools/toolchain/gcc-linaro/bin/
export UNIX_IASL_BIN_PATH=$PWD/pctools/acpi

function do_pack()
{

local chip=${BUILD_CHIP}
local platform=${BUILD_OS_PLATFORM}
local board=${BUILD_BOARD}
local debug=uart
local sigmode=none
local securemode=none

while getopts "c:p:b:dsvh" arg;
do
	case $arg in
		c)
			chip=$OPTARG
			;;
		p)
			platform=$OPTARG
			;;
		b)
			board=$OPTARG
			;;
		d)
			debug=card0
			;;
		s)
			sigmode=sig
			;;
		v)
			securemode=secure
			;;
		h)
			usage
			exit 0
			;;
		?)
			exit 1
			;;
	esac
done

cd ${PACK_TOOLS_DIR}/pack

if [ "x${BUILD_CHIP}" = "xsun50iw1p1" ]; then
	./npack -c $chip -p $platform -b $board -d $debug -s $sigmode 
else
	./pack -c $chip -p $platform -b $board -d $debug -s $sigmode -v $securemode
fi
}



build_select_chip()
{
    local cnt=0
    local choice
	  local call=$1

    printf "All available chips:\n"
    for chipdir in ${PACK_TOOLS_DIR}/pack/chips/* ; do
        chips[$cnt]=`basename $chipdir`
        printf "%4d. %s\n" $cnt ${chips[$cnt]}
        ((cnt+=1))
    done

    while true ; do
        read -p "Choice: " choice
        if [ -z "${choice}" ] ; then
            continue
        fi

        if [ -z "${choice//[0-9]/}" ] ; then
            if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
                BUILD_CHIP="${chips[$choice]}"
                break
            fi
        fi
        printf "Invalid input ...\n"
    done

}

build_select_platform()
{

	local cnt=0
	local choice
	local call=$1
	local platform
	local platforms=(
	windows
	linux
	)

	printf "All available platforms:\n"
	for platform in ${platforms[@]} ; do
		printf "%4d. %s\n" $cnt $platform
		((cnt+=1))
	done

	while true ; do
		read -p "Choice: " choice
		if [ -z "${choice}" ] ; then
			continue
		fi

		if [ -z "${choice//[0-9]/}" ] ; then
			if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
					 BUILD_PLATFORM="${platforms[$choice]}"
				break
			fi
		fi
		printf "Invalid input ...\n"
	done
}

build_select_board()
{

	 	local cnt=0
    local choice

    printf "All available boards:\n"
    for boarddir in ${PACK_TOOLS_DIR}/pack/chips/${BUILD_CHIP}/configs/* ; do
        boards[$cnt]=`basename $boarddir`
        if [ "x${boards[$cnt]}" = "xdefault" ] ; then
            continue
        fi
        printf "%4d. %s\n" $cnt ${boards[$cnt]}
        ((cnt+=1))
    done
    
    while true ; do
        read -p "Choice: " choice
        if [ -z "${choice}" ] ; then
            continue
        fi

        if [ -z "${choice//[0-9]/}" ] ; then
            if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
                BUILD_BOARD="${boards[$choice]}"
                break
            fi
        fi
        printf "Invalid input ...\n"
    done
    
}

build_select_tool_chain()
{
  	local count=0
		local length=0

    build_tool_chain=(ARMLINUXGCC RVCT)
    printf "All valid tool_chain:\n"

    length=`expr ${#build_tool_chain[@]} - 1`
    for count in `seq 0 $length`; do
    		printf "$count. ${build_tool_chain[$count]}\n"
    done
    
    let count=$count+1
    
    while true; do
        read -p "Please select a tool_chain:"
        RES=`expr match $REPLY "[0-9][0-9]*$"`
        if [ "$RES" -le 0 ]; then
            echo "please use index number"
            continue
        fi
        echo "REPLY=$REPLY"
        if [ "$REPLY" -ge $count ]; then
            echo "too big"
            continue
        fi
        if [ "$REPLY" -lt "0" ]; then
            echo "too small"
            continue
        fi
        break
    done
    
    BUILD_TOOL_CHAIN=${build_tool_chain[$REPLY]}

}

build_select_build_type()
{
  	local count=0
		local length=0

    build_type=(RELEASE DEBUG)
    printf "All valid build type:\n"

    length=`expr ${#build_type[@]} - 1`
    for count in `seq 0 $length`; do
    		printf "$count. ${build_type[$count]}\n"
    done
    
    let count=$count+1
    
    while true; do
        read -p "Please select a build type:"
        RES=`expr match $REPLY "[0-9][0-9]*$"`
        if [ "$RES" -le 0 ]; then
            echo "please use index number"
            continue
        fi
        if [ "$REPLY" -ge $count ]; then
            echo "too big"
            continue
        fi
        if [ "$REPLY" -lt "0" ]; then
            echo "too small"
            continue
        fi
        break
    done
    
    BUILD_TYPE=${build_type[$REPLY]}

}

build_boot0_config()
{
    INC_DIR=$PACK_TOOLS_DIR/${BUILD_CHIP}Pkg/Include
    for filedir in ${INC_DIR}/* ; do
        if [ -f ${filedir}/${BUILD_CHIP}.h ]
        then
                break
        fi
    done

    cp -f ${filedir}/${BUILD_CHIP}.h $PACK_TOOLS_DIR/Include/boot0_include/config.h
    cp -f ${filedir}/platform.h $PACK_TOOLS_DIR/Include/boot0_include/platform.h
    cp -f ${filedir}/dram.h $PACK_TOOLS_DIR/Include/boot0_include/
    cp -f ${filedir}/archdef.h $PACK_TOOLS_DIR/Include/boot0_include/
}

build_get_config_from_user()
{
	build_select_chip
	build_select_platform
	build_select_board
#	build_select_tool_chain
	build_select_build_type
}

build_write_config_to_file()
{
		rm -f $BUILD_CONFIG_FILE
		echo "BUILD_ARCH       :$BUILD_ARCH" >> $BUILD_CONFIG_FILE
		echo "BUILD_UEFI_PKG   :${BUILD_UEFI_PKG%%Pkg*}" >> $BUILD_CONFIG_FILE
		echo "BUILD_CHIP       :${BUILD_CHIP%%Pkg*}" >> $BUILD_CONFIG_FILE
		echo "BUILD_PLATFORM   :${BUILD_PLATFORM}" >> $BUILD_CONFIG_FILE
		echo "BUILD_BOARD      :${BUILD_BOARD}" >> $BUILD_CONFIG_FILE
		echo "BUILD_TYPE       :$BUILD_TYPE" >> $BUILD_CONFIG_FILE
		echo "BUILD_TOOL_CHAIN :$BUILD_TOOL_CHAIN" >> $BUILD_CONFIG_FILE
}

build_get_config_from_file()
{
		BUILD_ARCH=`cat $BUILD_CONFIG_FILE | awk -F"[:|=]" '(NF&&$1~/^[[:space:]]*BUILD_ARCH/) {printf "%s",$2}'`
		BUILD_UEFI_PKG=`cat $BUILD_CONFIG_FILE | awk -F"[:|=]" '(NF&&$1~/^[[:space:]]*BUILD_UEFI_PKG/) {printf "%sPkg",$2}'`
		BUILD_CHIP=`cat $BUILD_CONFIG_FILE | awk -F"[:|=]" '(NF&&$1~/^[[:space:]]*BUILD_CHIP/) {printf "%s",$2}'`
		BUILD_PLATFORM=`cat $BUILD_CONFIG_FILE | awk -F"[:|=]" '(NF&&$1~/^[[:space:]]*PLATFORM/) {printf "%s",$2}'`
		BUILD_BOARD=`cat $BUILD_CONFIG_FILE | awk -F"[:|=]" '(NF&&$1~/^[[:space:]]*BUILD_BOARD/) {printf "%s",$2}'`
		BUILD_TYPE=`cat $BUILD_CONFIG_FILE | awk -F"[:|=]" '(NF&&$1~/^[[:space:]]*BUILD_TYPE/) {printf "%s",$2}'`
		BUILD_TOOL_CHAIN=`cat $BUILD_CONFIG_FILE | awk -F"[:|=]" '(NF&&$1~/^[[:space:]]*BUILD_TOOL_CHAIN/) {printf "%s",$2}'`

}

build_show_config()
{
printf "\nconfig information is:\n"
echo -e '\033[0;31;36m'
printf "BUILD_ARCH       : $BUILD_ARCH\n"
printf "BUILD_UEFI_PKG   : ${BUILD_UEFI_PKG%%Pkg*}\n"
printf "BUILD_CHIP       : ${BUILD_CHIP%%Pkg*}\n"
printf "BUILD_TYPE       : $BUILD_TYPE\n"
printf "BUILD_TOOL_CHAIN : $BUILD_TOOL_CHAIN\n"
echo -e '\033[0m'
}

build_show_help()
{
printf "
 (c) Copyright 2015
 Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 Martin.Zheng <martinzheng@allwinnertech.com>

NAME
	build - The top level build script to build Sunxi platform UEFI system

SYNOPSIS
	build [-h] | [-m module] | [clean] [cleanall] [cleanconfig] |[showconfig]

OPTIONS
	-h             Display help message

	-m [module]   Use this option when you dont want to build all. [OPTIONAL]
  
    clean		  clean the compile file in the output dir
  
    cleanall	  clean all the compile file
  
    cleanconfig   clean the .config file
  
    showconfig    show the current compile config

"

}


if [ `basename $PWD` != $BUILD_UEFI_PKG ]; then
    echo "you must build form the $BUILD_UEFI_PKG dir."
    exit -1
fi

if [ "$1" == "-h" ]; then
build_show_help
exit
fi


if [ -z "${WORKSPACE:-}" ]
then
  echo Initializing workspace
  cd ../
  export EDK_TOOLS_PATH=`pwd`/BaseTools
  source edksetup.sh BaseTools
else
  echo Building from: $WORKSPACE
fi

for arg in "$@"
do
    if [[ $arg == config ]]; then
       	build_get_config_from_user
		build_write_config_to_file
		build_show_config
        exit
    fi
done

if [ -f $BUILD_CONFIG_FILE ]; then
	build_get_config_from_file
else
	build_get_config_from_user
	build_write_config_to_file
fi

build_show_config

for arg in "$@"
do
    if [[ $arg == showconfig ]]; then
          exit
    fi
done

if  [[ ! -e $EDK_TOOLS_PATH/Source/C/bin ]];
then
  # build the tools if they don't yet exist
  echo Building tools: $EDK_TOOLS_PATH
  make -C $EDK_TOOLS_PATH
else
  echo using prebuilt tools
fi

#
# Build the edk2 SunxiPlatform code
#

for arg in "$@"
do
    if [[ $arg == clean ]]; then
        echo "clean the build..."
    elif [[ $arg == cleanall ]]; then
        echo "cleanall the build..."
        make -C $EDK_TOOLS_PATH clean
        rm -rf $BUILD_CONFIG_FILE
    
    elif [[ $arg == cleanconfig ]]; then
        rm -rf $BUILD_CONFIG_FILE
        exit
    elif [[ $arg == pack ]]; then
        do_pack $2 $3 $4 $5 $6 $7  
        exit
    
    fi
done

cpu_cores=`cat /proc/cpuinfo | grep "processor" | wc -l`
if [ ${cpu_cores} -le 8 ] ; then
    jobs=${cpu_cores}
else
    jobs=`expr ${cpu_cores} / 2`
fi

for arg in "$@"
do
    if [[ $arg == fes ]]; then
        echo "build Fes..."
        build_boot0_config
        if [[ $BUILD_TYPE == RELEASE ]]; then
            build -p $WORKSPACE/${BUILD_UEFI_PKG}/${BUILD_CHIP}Pkg/fesPkg.dsc -a ARM -t $BUILD_TOOL_CHAIN -b $BUILD_TYPE -n $jobs -D DEBUG_TARGET=RELEASE  ${2:-} ${3:-} ${4:-} ${5:-} ${6:-} ${7:-} ${8:-}
        else
            build -p $WORKSPACE/$BUILD_UEFI_PKG/${BUILD_CHIP}Pkg/fesPkg.dsc -a ARM -t $BUILD_TOOL_CHAIN -b $BUILD_TYPE -n $jobs  ${2:-} ${3:-} ${4:-} ${5:-} ${6:-} ${7:-} ${8:-}
        fi
        gen_check_sum $WORKSPACE/Build/${BUILD_UEFI_PKG%%Pkg*}/${BUILD_CHIP}Pkg/${BUILD_TYPE}_${BUILD_TOOL_CHAIN}/FV/`tr '[a-z]' '[A-Z]'<<<${BUILD_CHIP}`_FES.fd\
 $WORKSPACE/${BUILD_UEFI_PKG}/pack/chips/${BUILD_CHIP}/bin/fes1_${BUILD_CHIP}.bin
	exit
    elif [[ $arg == boot0 ]]; then
        echo "build boot0..."
        build_boot0_config
        if [[ $BUILD_TYPE == RELEASE ]]; then
            build -p $WORKSPACE/${BUILD_UEFI_PKG}/${BUILD_CHIP}Pkg/boot0Pkg.dsc -a ARM -t $BUILD_TOOL_CHAIN -b $BUILD_TYPE -n $jobs -D DEBUG_TARGET=RELEASE ${2:-} ${3:-} ${4:-} ${5:-} ${6:-} ${7:-} ${8:-}
        else
            build -p $WORKSPACE/$BUILD_UEFI_PKG/${BUILD_CHIP}Pkg/boot0Pkg.dsc -a ARM -t $BUILD_TOOL_CHAIN -b $BUILD_TYPE -n $jobs ${2:-} ${3:-} ${4:-} ${5:-} ${6:-} ${7:-} ${8:-}
        fi
	gen_check_sum $WORKSPACE/Build/${BUILD_UEFI_PKG%%Pkg*}/${BUILD_CHIP}Pkg/${BUILD_TYPE}_${BUILD_TOOL_CHAIN}/FV/`tr '[a-z]' '[A-Z]'<<<${BUILD_CHIP}`_BOOT0.fd\
 $WORKSPACE/${BUILD_UEFI_PKG}/pack/chips/${BUILD_CHIP}/bin/boot0_sdcard_${BUILD_CHIP}.bin
        exit
    fi
done

# modify UEFI version according to board type
line=`grep -n "PcdFirmwareVersionString" $PACK_TOOLS_DIR/SunxiPlatformPkg.dsc.inc | cut  -d  ":"  -f  1`
sed -i "${line}s/.*$/  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L\"${BUILD_BOARD} UEFI V2.1.0\"/g" $PACK_TOOLS_DIR/SunxiPlatformPkg.dsc.inc

if [[ $BUILD_TYPE == RELEASE ]]; then
#  build -p $WORKSPACE/ShellPkg/ShellPkg.dsc -a ARM -t $BUILD_TOOL_CHAIN -b $BUILD_TYPE -n $jobs -D DEBUG_TARGET=RELEASE ${1:-} ${2:-} ${3:-} ${4:-} ${5:-} ${6:-} ${7:-} ${8:-}
  build -p $WORKSPACE/${BUILD_UEFI_PKG}/${BUILD_CHIP}Pkg/${BUILD_CHIP}Pkg.dsc -a ARM -t $BUILD_TOOL_CHAIN -b $BUILD_TYPE -n $jobs -D DEBUG_TARGET=RELEASE ${1:-} ${2:-} ${3:-} ${4:-} ${5:-} ${6:-} ${7:-} ${8:-}
else
 # build -p $WORKSPACE/ShellPkg/ShellPkg.dsc -a ARM -t $BUILD_TOOL_CHAIN -b $BUILD_TYPE -n $jobs ${1:-} ${2:-} ${3:-} ${4:-} ${5:-} ${6:-} ${7:-} ${8:-}
  build -p $WORKSPACE/$BUILD_UEFI_PKG/${BUILD_CHIP}Pkg/${BUILD_CHIP}Pkg.dsc -a ARM -t $BUILD_TOOL_CHAIN -b $BUILD_TYPE -n $jobs ${1:-} ${2:-} ${3:-} ${4:-} ${5:-} ${6:-} ${7:-} ${8:-}
fi

#genchecksum_uefi edk2/Build/${BUILD_UEFI_PKG%%Pkg*}/${BUILD_CHIP}/${BUILD_TYPE}_${BUILD_TOOL_CHAIN}/FV/`tr '[a-z]' '[A-Z]'<<<${BUILD_CHIP%%Pkg*}`_EFI.fd

#cp -f edk2/Build/${BUILD_UEFI_PKG%%Pkg*}/${BUILD_CHIP}Pkg/${BUILD_TYPE}_${BUILD_TOOL_CHAIN}/FV/`tr '[a-z]' '[A-Z]'<<<${BUILD_CHIP}`_EFI.fd\
# pack/chips/${BUILD_CHIP}/bin/u-boot-${BUILD_CHIP}.bin

cp -f $WORKSPACE/Build/${BUILD_UEFI_PKG%%Pkg*}/${BUILD_CHIP}Pkg/${BUILD_TYPE}_${BUILD_TOOL_CHAIN}/FV/`tr '[a-z]' '[A-Z]'<<<${BUILD_CHIP}`_EFI.fd\
 $WORKSPACE/${BUILD_UEFI_PKG}/pack/chips/${BUILD_CHIP}/bin/uefi-${BUILD_CHIP}.bin

