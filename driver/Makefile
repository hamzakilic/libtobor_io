# my optional module name
MODULE=em_gpio_event_driver
 
# this two variables, depends where you have you raspberry kernel source and tools installed
 

 
 
obj-m += ${MODULE}.o
 
module_upload=${MODULE}.ko
 
all: compile
 
compile:
    make ARCH=arm CROSS_COMPILE=arm-raspi-linux-gnueabi- -C ${KERNEL_SRC} M=$(PWD) modules
 
 
 
