# obj-m is a list of what kernel modules to build.  The .o and other
# objects will be automatically built from the corresponding .c file -
# no need to list the source files explicitly.

obj-m := em_driver.o 

# KDIR is the location of the kernel source.  The current standard is
# to link to the associated source tree from the directory containing
# the compiled modules.
KDIR  := /work/raspberrypi/linux

# PWD is the current working directory and the location of our module
# source files.
PWD   := $(shell pwd)

# default is the default make target.  The rule here says to run make
# with a working directory of the directory containing the kernel
# source and compile only the modules in the PWD (local) directory.
default:
	$(MAKE) -C $(KDIR) M=$(PWD)  ARCH=arm CROSS_COMPILE=arm-raspi-linux-gnueabi- modules

clean:
	rm  *.ko *.mod.c *.mod.o *.o modules.* Module.*

copy:
	cp *.ko /var/www/html
	

 
 
 
