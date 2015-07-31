rm *.ko *.mod.c *.mod.o *.o modules.* Module.*
make ARCH=arm CROSS_COMPILE=arm-raspi-linux-gnueabi- -C /work/raspberrypi/linux M=$PWD modules
cp *.ko /var/www/html/
