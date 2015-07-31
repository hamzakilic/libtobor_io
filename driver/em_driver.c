/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 */

#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#include <linux/list.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <mach/platform.h>
#include <mach/gpio.h>
#include <linux/time.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
/*
 *  Prototypes - this would normally go in a .h file
 */

MODULE_AUTHOR("Hamza <hamza@hamzakilic.com>");
MODULE_DESCRIPTION("BCM2835 em_io lib driver");
MODULE_LICENSE("GPL");


static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);


struct em_io_irq{
   unsigned int irq;
   const char * device_name;
   unsigned int physical_address;
   void * map_address;
   wait_queue_head_t process_wait;
   dev_t device; // Global variable for the first device number
   struct cdev c_dev; // Global variable for the character device structure
   struct class *cl; // Global variable for the device class
   int interrupt_occured;
   spinlock_t lock;
};

#define SIZE_OF_ITEMS 1

static struct em_io_irq items[SIZE_OF_ITEMS];



void init_em_io_items(void){

#define PERIPHERALS_BASE_PHYSICAL  0x20000000
#define PERIPHERALS_UART_PHYSICAL 0x201000
#define PERIPHERALS_UART_INTERRUPT_MASK_STATUS_PHYSICAL 0x40
#define PERIPHERALS_UART_INTERRUPT_CLEAR_PHYSICAL 0x44


	items[0].irq=83;
	items[0].device_name="em_uart";
	items[0].physical_address=PERIPHERALS_BASE_PHYSICAL+PERIPHERALS_UART_PHYSICAL+PERIPHERALS_UART_INTERRUPT_CLEAR_PHYSICAL;
	items[0].interrupt_occured=0;

}





static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};


static irqreturn_t irq_handler(int irq, void *dev_id)
{
	 unsigned long interrupt_flags;
	 int index=(int)dev_id;
	 spin_lock_irqsave(&items[index].lock,interrupt_flags);
	 spin_lock(&items[index].lock);
	// printk(KERN_INFO "irq mask handled before: %d \n",ioread32(items[index].map_address));
     iowrite32(0xFFFFFFFF,items[index].map_address);
    // ioread32(items[index].map_address+4);
     //ioread32(items[index].map_address+4);

    // printk(KERN_INFO "irq mask handled after: %d \n",ioread32(items[index].map_address));
    // printk(KERN_INFO "irq handled: %d \n",irq);

     items[index].interrupt_occured=1;
     wake_up_interruptible(&items[index].process_wait);
     spin_unlock(&items[index].lock);
     spin_unlock_irqrestore(&items[index].lock,interrupt_flags);


	 return IRQ_HANDLED;

}

/*
 * This function is called when the module is loaded
 */

  int __init  em_init_module(void)
{
	  int index;
	  init_em_io_items();
	  for(index=0;index<SIZE_OF_ITEMS;++index){
		  if (alloc_chrdev_region(&items[index].device, 0, 1, items[index].device_name) < 0)
		    {
		      printk(KERN_INFO "registering chrdev failed stage1 %s",items[index].device_name);
		      continue;
		    }
		  if ((items[index].cl = class_create(THIS_MODULE, "chardrv")) == NULL)
		    {

			  printk(KERN_INFO "registering chrdev failed stage2 %s",items[index].device_name);
			  		      continue;
		    }
		  if (device_create(items[index].cl, NULL, items[index].device, NULL, items[index].device_name) == NULL)
		    {
			  printk(KERN_INFO "registering chrdev failed stage3 %s",items[index].device_name);
			  			  		      continue;

		    }
		      cdev_init(&items[index].c_dev, &fops);
		      if (cdev_add(&items[index].c_dev, items[index].device, 1) == -1)
		    {
		    	  printk(KERN_INFO "registering chrdev failed stage4 %s",items[index].device_name);
		    	 			  			  		      continue;

		    }


     init_waitqueue_head (&(items[index].process_wait));
     spin_lock_init(&items[index].lock);

    items[index].map_address= ioremap(items[index].physical_address,4);



    if(request_irq(items[index].irq,irq_handler,0 ,items[index].device_name,(void*)index))
            	 printk(KERN_INFO "uart: can't get assigned irq %i\n",items[index].irq);

	  }

	return 0;
}


  void __exit em_cleanup_module(void)
{


}

 module_init(em_init_module);
 module_exit(em_cleanup_module);



/*
 * Methods
 */

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "device opened %s\n",file->f_path.dentry->d_iname);
	try_module_get(THIS_MODULE);

	return 0;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{


	/*
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module.
	 */
	printk(KERN_INFO "device closed %s\n",file->f_path.dentry->d_iname);
	module_put(THIS_MODULE);

	return 0;
}
//wake_up(wait_queue_head_t *queue);

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	int index;
	//printk(KERN_INFO "device is reading %s\n",filp->f_path.dentry->d_iname);

	for(index=0;index<SIZE_OF_ITEMS;++index){
		if(strcmp(items[index].device_name,filp->f_path.dentry->d_iname)==0){
	      // printk(KERN_INFO "waiting interrupts for filename is %s\n",filp->f_path.dentry->d_iname);
	       //
	       spin_lock(&items[index].lock);
	       if(items[index].interrupt_occured==0){

	       wait_event_interruptible(items[index].process_wait,(items[index].interrupt_occured == 1));

	       }
	       items[index].interrupt_occured=0;
	       spin_unlock(&items[index].lock);
	       break;
		}
	}

	return 0;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
	return -EINVAL;
}
