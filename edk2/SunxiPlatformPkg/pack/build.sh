#!/bin/bash

LICHEE_ROOT=$PWD
PACK_ROOT=pack
TARGET_CHIP="a31"
TARGET_PLATFORM="linux"
TARGET_BOARD="evb"
PACK_DEBUG="uart0"
count=0


while getopts hc:p:b:B:d:f: OPTION
do
    case $OPTION in
    h) show_help
    exit 0
    ;;
		d) PACK_DEBUG=$OPTARG 
		;;
    *) show_help
    exit 0
    ;;
	  esac
done

select_chips()
{
    count=0

    printf "All valid chips:\n"

    for chip in $(cd chips/; find -mindepth 1 -maxdepth 1 -type d |sort); do
        chips[$count]=`basename $PACK_ROOT/chips/$chip`
        printf "$count. ${chips[$count]}\n"
        let count=$count+1
    done

    while true; do
        read -p "Please select a chip:"
        RES=`expr match "$REPLY" '[0-9][0-9]*$'`
		if [ -z "$REPLY" ]; then
			echo "Must choice a chip"
			continue
		fi
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

    TARGET_CHIP=${chips[$REPLY]}
}

select_platform()
{
    count=0
    chip=$1

    printf "All valid platforms:\n"

    for platform in $(cd chips/$chip/configs/; find -mindepth 1 -maxdepth 1 -type d |sort); do
        platforms[$count]=`basename $PACK_ROOT/chips/$chip/configs/$platform`
        printf "$count. ${platforms[$count]}\n"
        let count=$count+1
    done

    while true; do
        read -p "Please select a platform:"
        RES=`expr match "$REPLY" '[0-9][0-9]*$'`
        if [ -z "$REPLY" ]; then
			echo "Must choice a platform"
			continue
		fi
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

    TARGET_PLATFORM=${platforms[$REPLY]}
}

select_boards()
{
    count=0
    chip=$1
    platform=$2

    printf "All valid boards:\n"

    for board in $(cd chips/$chip/configs/$platform/; find -mindepth 1 -maxdepth 1 -type d |grep -v default|sort); do
        boards[$count]=`basename $PACK_ROOT/chips/$chip/configs/$platform/$board`
        printf "$count. ${boards[$count]}\n"
        let count=$count+1
    done

    while true; do
        read -p "Please select a board:"
        RES=`expr match "$REPLY" '[0-9][0-9]*$'`
        if [ -z "$REPLY" ]; then
			echo "Must choice a board"
			continue
		fi
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

    TARGET_BOARD=${boards[$REPLY]}
}

printf "Start packing system\n\n"

select_chips

select_platform $TARGET_CHIP

select_boards $TARGET_CHIP $TARGET_PLATFORM

echo "$TARGET_CHIP $TARGET_PLATFORM $TARGET_BOARD"

if [ "$TARGET_PLATFORM" = "crane" ]; then
    if [ -z "$CRANE_IMAGE_OUT" ]; then
        echo "You need to export CRANE_IMAGE_OUT var to env"
        exit 1
    fi

    if [ ! -f "$CRANE_IMAGE_OUT/system.img" ]; then
        echo "You have wrong CRANE_IMAGE_OUT env"
        exit 1
    fi
fi

./pack -c $TARGET_CHIP -p $TARGET_PLATFORM -b $TARGET_BOARD -d $PACK_DEBUG


