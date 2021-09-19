#!/bin/bash

export ARCH=arm
#export CROSS_COMPILE=/media/work/iTOP-iMAX6/iTOP-iMX6_android4.4.2/myandroid/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-
export CROSS_COMPILE=$(pwd)/../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-

#cp config_for_iTOPiMX6_android .config
cp config_caesar_v01 .config

make zImage -j40

make uImage -j40

make modules

