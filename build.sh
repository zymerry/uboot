#!/bin/sh
# 
# 

CPU_NUM=$(cat /proc/cpuinfo | grep processor | wc -l)
UBOOT_CONFIG=TX2440_config

make distclean || return 1;
make ${UBOOT_CONFIG} || return 1;
make -j${CPU_NUM} || return 1;

if [ -f u-boot.bin ]; then
	echo "^_^ u-boot.bin created success."
	exit
else
	echo "*** make error,cann't compile u-boot.bin!"
	exit
fi
exit 0
