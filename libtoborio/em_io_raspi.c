/*
 * io_raspi.c
 *
 *  Created on: Apr 7, 2015
 *      Author: root
 */

#include "em_io_raspi.h"



#define PERIPHERALS_BASE_PHYSICAL  0x20000000
#define PERIPHERALS_GPIO_PHYSICAL 0x200000
#define PERIPHERALS_TIMER_PHYSICAL 0x3000

static em_uint32 mem_fd;
volatile em_uint32 * gpio;
volatile em_uint32 * timer;

#define GPIO_REGISTERS_SIZE  (0x7E2000B0-0x7E200000)
#define TIMER_REGISTERS_SIZE (7*sizeof(em_uint32))

static em_uint8 pin_maps[255] = { 2, 3, 4, 14, 15, 17, 18, 27, 22, 23, 24, 10,
		9, 25, 11, 8, 7 };
const em_uint8 total_pin_counts = 17;


#ifndef barrierdefs
 #define barrierdefs
 //raspi is ARMv6. It does not have ARMv7 DMB/DSB/ISB, so go through CP15.
 #define isb() __asm__ __volatile__ ("mcr p15, 0, %0, c7,  c5, 4" : : "r" (0) : "memory")
 #define dmb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")
 //ARMv6 DSB (DataSynchronizationBarrier): also known as DWB (drain write buffer / data write barrier) on ARMv5
 #define dsb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")
#endif
//use dmb() around the accesses. ("before write, after read")



em_uint32 arm_read(volatile  em_uint32 *address)
{
	em_uint32 val=*address;
	dmb();
	return val;
}

void arm_write(volatile em_uint32 *address,em_uint32 value){
	 dmb();
	*address=value;


}


em_uint32 em_raspi_initialize(em_uint32 system) {

	if (geteuid() != 0) {
		em_log(EM_LOG_FATAL, errno, "you must be root or sudo execute\n");
		return EM_ERROR_CANNOT_OPEN;

	}
	if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
		em_log(EM_LOG_FATAL, errno, "/dev/mem cannot open\n");
		return EM_ERROR_CANNOT_OPEN;
	}
	//em_int32 sys_page_size= sysconf(_SC_PAGE_SIZE);
	//em_log(EM_LOG_INFO,0,"page size is %d \n",sys_page_size);
	gpio = (volatile em_uint32 *) mmap(NULL, GPIO_REGISTERS_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,(PERIPHERALS_BASE_PHYSICAL + PERIPHERALS_GPIO_PHYSICAL));
	timer = (volatile em_uint32 *) mmap(NULL, TIMER_REGISTERS_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,(PERIPHERALS_BASE_PHYSICAL + PERIPHERALS_TIMER_PHYSICAL));


	close(mem_fd);
	if (gpio == MAP_FAILED ||  timer == MAP_FAILED) {
		em_log(EM_LOG_FATAL, errno, "map to ram failed\n");
		return EM_ERROR_CANNOT_OPEN;
	}




	return EM_SUCCESS;

}

#define PERIPHERALS_GPIO_FUNCTION_SELECT_PHYSICAL (0x00/sizeof(em_uint32))
#define PERIPHERALS_GPIO_OUTPUT_PHYSICAL (0x1C/sizeof(em_uint32))
#define PERIPHERALS_GPIO_CLEAR_PHYSICAL (0x28/sizeof(em_uint32))
#define PERIPHERALS_GPIO_LEVEL_PHYSICAL (0x34/sizeof(em_uint32))

#define PIN_INPUT 0x00000000
#define PIN_OUTPUT 0x00000001
#define PIN_ALT_O  0x00000004

#define PIN_MODE_POSITION(number) (number/10)
#define PIN_MODE_SHIFT(number)   ((number%10)*3)

em_uint32 em_raspi_gpio_mode(em_uint8 number, em_uint8 mode) {

#ifdef EM_DEBUG
	if (number > total_pin_counts)
		return EM_ERROR_PIN_NUMBER_INVALID;
#endif

	number = pin_maps[number];
	volatile em_uint32 *address=gpio + PERIPHERALS_GPIO_FUNCTION_SELECT_PHYSICAL+ PIN_MODE_POSITION(number);
	if (mode & EM_DIRECTION_OUT) {
		em_uint32 address_value=arm_read(address);
		arm_write(address, address_value & ~(0x0000007<< PIN_MODE_SHIFT(number)));
		address_value=arm_read(address);
		arm_write(address,address_value | (PIN_OUTPUT	<< PIN_MODE_SHIFT(number)));
	} else if (mode & EM_DIRECTION_IN){

		em_uint32 address_value=arm_read(address);
	    arm_write(address, address_value & ~(0x0000007<< PIN_MODE_SHIFT(number)));
	}

	return EM_SUCCESS;
}

#define PIN_WRITE_POSITION(number)  ((number/32))
#define PIN_WRITE_SHIFT(number)  (number%32)

em_uint32 em_raspi_gpio_read(em_uint8 number, em_uint8 *val) {
#ifdef EM_DEBUG
	if (number > total_pin_counts)
		return EM_ERROR_PIN_NUMBER_INVALID;
#endif
	number = pin_maps[number];
	em_uint32 value = arm_read(gpio + PERIPHERALS_GPIO_LEVEL_PHYSICAL+ PIN_WRITE_POSITION(number));
	em_uint32 pin_position = 1 << PIN_WRITE_SHIFT(number);
	if (value & pin_position)
		*val = EM_GPIO_HIGH;
	else
		*val = EM_GPIO_LOW;

	return EM_SUCCESS;

}

em_uint32 em_raspi_gpio_write(em_uint8 number, em_uint8 val) {

#ifdef EM_DEBUG
	if (number > total_pin_counts)
		return EM_ERROR_PIN_NUMBER_INVALID;
#endif
	number = pin_maps[number];
	if (val & EM_GPIO_HIGH) {
       volatile em_uint32 *address=gpio + PERIPHERALS_GPIO_OUTPUT_PHYSICAL + PIN_WRITE_POSITION(number);
       em_uint32 address_value=arm_read(address);
	   arm_write(address, address_value | (1 << PIN_WRITE_SHIFT(number)));

	} else if (val & EM_GPIO_LOW) {

		volatile em_uint32 *address=gpio + PERIPHERALS_GPIO_CLEAR_PHYSICAL + PIN_WRITE_POSITION(number);
		       em_uint32 address_value=arm_read(address);
			   arm_write(address, address_value | (1 << PIN_WRITE_SHIFT(number)));


	}

	return EM_SUCCESS;
}

#define PERIPHERALS_GPIO_PULL_PHYSICAL (0x94/sizeof(em_uint32))
#define PERIPHERALS_GPIO_PULL_CLOCK_PHYSICAL (0x98/sizeof(em_uint32))

em_uint32 em_raspi_gpio_pull(em_uint8 number, em_uint8 mode) {

#ifdef EM_DEBUG
	if (number > total_pin_counts)
		return EM_ERROR_PIN_NUMBER_INVALID;
#endif
	number = pin_maps[number];
	volatile em_uint32 *address_pull=gpio + PERIPHERALS_GPIO_PULL_PHYSICAL;
	if (mode & EM_PULL_OFF) {

		 arm_write(address_pull,0x00000000);
	} else if (mode & EM_PULL_DOWN) {

		arm_write(address_pull,0x00000001);


	} else if (mode & EM_PULL_UP) {

	 arm_write(address_pull,0x00000002);

	}

	em_io_delay_microseconds(5);

	volatile em_uint32 *address_pullclock=gpio + PERIPHERALS_GPIO_PULL_CLOCK_PHYSICAL + PIN_WRITE_POSITION(number);
	em_uint32 address_pullclock_value=arm_read(address_pullclock);
	arm_write(address_pullclock,address_pullclock_value | (1 << PIN_WRITE_SHIFT(number)));
	em_io_delay_microseconds(5);

	arm_write(address_pull,0x00);
	em_io_delay_microseconds(5);

	arm_write(address_pullclock,0x00);
	em_io_delay_microseconds(5);

	return EM_SUCCESS;
}

#define PERIPHERALS_GPIO_RISING_EDGE_PHYSICAL (0x4C/sizeof(em_uint32))
#define PERIPHERALS_GPIO_FALLING_EDGE_PHYSICAL (0x58/sizeof(em_uint32))
#define PERIPHERALS_GPIO_HIGH_DETECT_PHYSICAL (0x64/sizeof(em_uint32))
#define PERIPHERALS_GPIO_LOW_DETECT_PHYSICAL (0x70/sizeof(em_uint32))

em_uint32 em_raspi_set_event(em_uint8 number, em_uint8 event,
		em_uint8 enable_or_disable) {
	number = pin_maps[number];
	if (event & EM_EVENT_RISING_EDGE_DETECT) {
			if (enable_or_disable & EM_EVENT_ENABLE){
                volatile em_uint32 * address_rising=gpio + PERIPHERALS_GPIO_RISING_EDGE_PHYSICAL+ PIN_WRITE_POSITION(number);
                em_uint32 address_rising_value=arm_read(address_rising);
                arm_write(address_rising,address_rising_value | (1<< PIN_WRITE_SHIFT(number)));





			}
			else if (enable_or_disable & EM_EVENT_DISABLE){

				volatile em_uint32 * address_rising=gpio + PERIPHERALS_GPIO_RISING_EDGE_PHYSICAL+ PIN_WRITE_POSITION(number);
				                em_uint32 address_rising_value=arm_read(address_rising);
				                arm_write(address_rising,address_rising_value & ~(1<< PIN_WRITE_SHIFT(number)));



			}
		}
	if (event & EM_EVENT_FALLING_EDGE_DETECT) {
				if (enable_or_disable & EM_EVENT_ENABLE){

	                volatile em_uint32 * address_falling=gpio + PERIPHERALS_GPIO_FALLING_EDGE_PHYSICAL+ PIN_WRITE_POSITION(number);
	                                em_uint32 address_falling_value=arm_read(address_falling);
	                                arm_write(address_falling,address_falling_value | (1<< PIN_WRITE_SHIFT(number)));



				}
				else if (enable_or_disable & EM_EVENT_DISABLE){




					                volatile em_uint32 * address_falling=gpio + PERIPHERALS_GPIO_FALLING_EDGE_PHYSICAL+ PIN_WRITE_POSITION(number);
					                                em_uint32 address_falling_value=arm_read(address_falling);
					                                arm_write(address_falling,address_falling_value & ~(1<< PIN_WRITE_SHIFT(number)));
				}
			}


	if (event & EM_EVENT_HIGH_DETECT) {
		if (enable_or_disable & EM_EVENT_ENABLE){
			volatile em_uint32 * address=(gpio + PERIPHERALS_GPIO_HIGH_DETECT_PHYSICAL+ PIN_WRITE_POSITION(number));
            em_uint32 address_value=arm_read(address);
            arm_write(address, address_value | (1<< PIN_WRITE_SHIFT(number)));


		}
		else if (enable_or_disable & EM_EVENT_DISABLE){

			volatile em_uint32 * address=(gpio + PERIPHERALS_GPIO_HIGH_DETECT_PHYSICAL+ PIN_WRITE_POSITION(number));
			            em_uint32 address_value=arm_read(address);
			            arm_write(address, address_value & ~(1<< PIN_WRITE_SHIFT(number)));




		}

	}

	if (event & EM_EVENT_LOW_DETECT) {
			if (enable_or_disable & EM_EVENT_ENABLE){

				volatile em_uint32 * address=(gpio + PERIPHERALS_GPIO_LOW_DETECT_PHYSICAL+ PIN_WRITE_POSITION(number));
				            em_uint32 address_value=arm_read(address);
				            arm_write(address,address_value | (1<< PIN_WRITE_SHIFT(number)));


			}
			else if (enable_or_disable & EM_EVENT_DISABLE){

				volatile em_uint32 * address=(gpio + PERIPHERALS_GPIO_LOW_DETECT_PHYSICAL+ PIN_WRITE_POSITION(number));
								            em_uint32 address_value=arm_read(address);
								            arm_write(address,address_value & ~(1<< PIN_WRITE_SHIFT(number)));
			}
		}





	return EM_SUCCESS;
}

#define PERIPHERALS_EVENT_DETECT_PHYSICAL (0x40/sizeof(em_uint32))
em_uint32 em_raspi_read_event(em_uint8 number,em_uint8* val){
	number = pin_maps[number];
	volatile em_uint32 * address =(gpio + PERIPHERALS_EVENT_DETECT_PHYSICAL+ PIN_WRITE_POSITION(number));

		em_uint32 pin_value = 1 << PIN_WRITE_SHIFT(number);
		em_uint32 address_val=arm_read(address);



		if (address_val & pin_value){
			*val=EM_EVENT_DETECTED;
			arm_write(address, address_val | (1 << PIN_WRITE_SHIFT(number)));
		}
		else
			*val=EM_EVENT_NOT_DETECTED;


		return EM_SUCCESS;
}


#define PERIPHERALS_TIMER_COUNTER_LOW_PHYSICAL (0x4/sizeof(em_uint32))
#define PERIPHERALS_TIMER_COUNTER_HIGH_PHYSICAL (0x8/sizeof(em_uint32))

em_uint64 em_raspi_current_time(em_uint64 *time_value){
	volatile em_uint32 * address_low =(timer + PERIPHERALS_TIMER_COUNTER_LOW_PHYSICAL);
	volatile em_uint32 * address_high =(timer +  PERIPHERALS_TIMER_COUNTER_HIGH_PHYSICAL);
	em_uint32 low_value=arm_read(address_low);
	em_uint32 high_value=arm_read(address_high);
	em_uint64 temp=high_value;
	temp = temp << 32;
	temp +=low_value;
	*time_value=temp;


	return EM_SUCCESS;

}





