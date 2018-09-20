################################################################################

1. How to Build
	- get Toolchain
		From android git server, codesourcery and etc ..
		 - arm-eabi-4.6

	- edit Makefile
		edit "CROSS_COMPILE" to right toolchain path(You downloaded).
		Ex)   export PATH=$PATH:(android platform directory you download)/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin
		Ex)   export CROSS_COMPILE=arm-eabi-

		$ mkdir -p ../out/drivers/modem/balong_oam_ps
		$ make ARCH=arm O=../out hi6921_v711_e5573cs-609_defconfig
		$ make ARCH=arm O=../out -j8

2. How to Build wifi
	- edit Makefile
		edit "CROSS_COMPILE" to right toolchain path(You downloaded).
		Ex)   export PATH=$PATH:$(android platform directory you download)/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-
		Ex)   export CROSS_COMPILE=arm-eabi-

		$ cd drivers/wifi
		$ ./build_wifi_v7.sh

3. Output files
		- Kernel : out/arch/arm/boot/Image
		- module : out/drivers/*/*.ko
		- wifi : out/system/bin/wifi_rtl

4. How to Clean
		$ make ARCH=arm distclean
		$ rm -rf out
################################################################################
