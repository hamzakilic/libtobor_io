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
#define PERIPHERALS_MINI_UART_PHYSICAL 0x215000
#define PERIPHERALS_UART_PHYSICAL 0x201000
#define PERIPHERALS_PWM_PHYSICAL 0x20C000
#define PERIPHERALS_PWM_CLOCK_PHYSICAL 0x101000
#define PERIPHERALS_BSC0_PHYSICAL 0x205000
#define PERIPHERALS_BSC1_PHYSICAL 0x804000


static em_uint32 mem_fd;
volatile em_uint32 * gpio;
volatile em_uint32 * timer;
volatile em_uint32 *mini_uart;
volatile em_uint32 *uart;
volatile em_uint32 *pwm;
volatile em_uint32 *pwm_clock;
volatile em_uint32 *bsc0;
volatile em_uint32 *bsc1;


#define GPIO_REGISTERS_SIZE  (0xB0)
#define TIMER_REGISTERS_SIZE (0x18)
#define MINI_UART_REGISTERS_SIZE  (0xD4)
#define UART_REGISTERS_SIZE  (0x8C)
#define PWM_REGISTERS_SIZE (0x28)
#define PWM_CLOCK_REGISTERS_SIZE (0xa8)
#define BSC_REGISTERS_SIZE (0x20)

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
	mini_uart = (volatile em_uint32 *) mmap(NULL, MINI_UART_REGISTERS_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,(PERIPHERALS_BASE_PHYSICAL + PERIPHERALS_MINI_UART_PHYSICAL));
	uart = (volatile em_uint32 *) mmap(NULL, UART_REGISTERS_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,(PERIPHERALS_BASE_PHYSICAL + PERIPHERALS_UART_PHYSICAL));
	pwm = (volatile em_uint32 *) mmap(NULL, PWM_REGISTERS_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,(PERIPHERALS_BASE_PHYSICAL + PERIPHERALS_PWM_PHYSICAL));
	pwm_clock = (volatile em_uint32 *) mmap(NULL, PWM_CLOCK_REGISTERS_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,(PERIPHERALS_BASE_PHYSICAL + PERIPHERALS_PWM_CLOCK_PHYSICAL));
	bsc0 = (volatile em_uint32 *) mmap(NULL, BSC_REGISTERS_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,(PERIPHERALS_BASE_PHYSICAL + PERIPHERALS_BSC0_PHYSICAL));
	bsc1 = (volatile em_uint32 *) mmap(NULL, BSC_REGISTERS_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,(PERIPHERALS_BASE_PHYSICAL + PERIPHERALS_BSC1_PHYSICAL));



	close(mem_fd);
	if (gpio == MAP_FAILED ||  timer == MAP_FAILED || mini_uart==MAP_FAILED || uart==MAP_FAILED || pwm==MAP_FAILED || pwm_clock==MAP_FAILED || bsc0==MAP_FAILED || bsc1==MAP_FAILED) {
		em_log(EM_LOG_FATAL, errno, "map to ram ,gpio or timer or mini_uart or uart  or pwm or pwm_clock failed\n");
		return EM_ERROR_CANNOT_OPEN;
	}




	return EM_SUCCESS;

}

#define PERIPHERALS_GPIO_FUNCTION_SELECT_PHYSICAL (0x00/sizeof(em_uint32))
#define PERIPHERALS_GPIO_OUTPUT_PHYSICAL (0x1C/sizeof(em_uint32))
#define PERIPHERALS_GPIO_CLEAR_PHYSICAL (0x28/sizeof(em_uint32))
#define PERIPHERALS_GPIO_LEVEL_PHYSICAL (0x34/sizeof(em_uint32))



#define PIN_MODE_POSITION(number) (number/10)
#define PIN_MODE_SHIFT(number)   ((number%10)*3)

em_uint32 em_raspi_gpio_mode(em_uint8 number, em_uint8 mode) {

#ifdef EM_DEBUG
	if (number > total_pin_counts)
		return EM_ERROR_PIN_NUMBER_INVALID;
#endif

	number = pin_maps[number];
	volatile em_uint32 *address=gpio + PERIPHERALS_GPIO_FUNCTION_SELECT_PHYSICAL+ PIN_MODE_POSITION(number);
	if (mode & EM_MODE_GPIO_OUT) {
		em_uint32 address_value=arm_read(address);
		address_value &= ~(0x00000007<< PIN_MODE_SHIFT(number));
		address_value |= (0x00000001	<< PIN_MODE_SHIFT(number));
		arm_write(address,address_value);
	} else if (mode & EM_MODE_GPIO_IN){
		em_uint32 address_value=arm_read(address);
		address_value &=~(0x00000007<< PIN_MODE_SHIFT(number));
	    arm_write(address, address_value  );
	}else if (mode & EM_MODE_GPIO_FUNC0){
		em_uint32 address_value=arm_read(address);
		address_value &= ~(0x00000007 <<PIN_MODE_SHIFT(number));
		address_value |= (0x00000004 <<PIN_MODE_SHIFT(number));
	    arm_write(address, address_value);
	}else if (mode & EM_MODE_GPIO_FUNC1){
		em_uint32 address_value=arm_read(address);
		address_value &= ~(0x00000007 <<PIN_MODE_SHIFT(number));
		address_value |= (0x00000005 <<PIN_MODE_SHIFT(number));
	    arm_write(address, address_value);
	}else if (mode & EM_MODE_GPIO_FUNC2){
			em_uint32 address_value=arm_read(address);
			address_value &= ~(0x00000007 <<PIN_MODE_SHIFT(number));
			address_value |= (0x00000006 <<PIN_MODE_SHIFT(number));
		    arm_write(address, address_value);
	}else if (mode & EM_MODE_GPIO_FUNC3){
			em_uint32 address_value=arm_read(address);
			address_value &= ~(0x00000007 <<PIN_MODE_SHIFT(number));
			address_value |= (0x00000007 <<PIN_MODE_SHIFT(number));
		    arm_write(address, address_value);
	}else if (mode & EM_MODE_GPIO_FUNC4){
			em_uint32 address_value=arm_read(address);
			address_value &= ~(0x00000007 <<PIN_MODE_SHIFT(number));
			address_value |= (0x00000003 <<PIN_MODE_SHIFT(number));
		    arm_write(address, address_value);
	}else if (mode & EM_MODE_GPIO_FUNC5){
			em_uint32 address_value=arm_read(address);
			address_value &= ~(0x00000007 <<PIN_MODE_SHIFT(number));
			address_value |= (0x00000002 <<PIN_MODE_SHIFT(number));
		    arm_write(address, address_value);
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

	em_io_delay_loops(150);

	volatile em_uint32 *address_pullclock=gpio + PERIPHERALS_GPIO_PULL_CLOCK_PHYSICAL + PIN_WRITE_POSITION(number);
	em_uint32 address_pullclock_value=arm_read(address_pullclock);
	arm_write(address_pullclock,address_pullclock_value | (1 << PIN_WRITE_SHIFT(number)));
	em_io_delay_loops(150);

	arm_write(address_pull,0x00);
	em_io_delay_loops(150);

	arm_write(address_pullclock,0x00);
	em_io_delay_loops(150);

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

em_uint32 em_raspi_current_time(em_uint64 *time_value){

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

em_uint32 em_raspi_busy_wait(em_uint64 micro_seconds){
    em_uint64 current_val=0;
    em_uint64 temp_val;
    em_uint32 ret_val;

    ret_val=em_raspi_current_time(&current_val);
    if(ret_val)
    	return ret_val;
    while(1){
       ret_val=em_raspi_current_time(&temp_val);
       if(ret_val)
    	   return ret_val;

       if( (temp_val-current_val)>=micro_seconds)
    	   break;
    }
return EM_SUCCESS;
}


//UART FUNCTIONS

#define PERIPHERALS_MINI_UART_INTERRUPT_STATUS_PHYSICAL (0x0)
#define PERIPHERALS_MINI_UART_ENABLES_PHYSICAL (0x4/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_DATA_PHYSICAL (0x40/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_INTERRUPT_ENABLE_PHYSICAL (0x44/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_INTERRUPT_IDENTIFY_PHYSICAL (0x48/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_LINE_CONTROL_PHYSICAL (0x4C/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_MODEM_CONTROL_PHYSICAL (0x50/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_LINE_STATUS_PHYSICAL (0x54/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_MODEM_STATUS_PHYSICAL (0x58/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_STRACH_PHYSICAL (0x5C/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_EXTRA_CONTROL_PHYSICAL (0x60/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_EXTRA_STATUS_PHYSICAL (0x64/sizeof(em_uint32))
#define PERIPHERALS_MINI_UART_BAUDRATE_PHYSICAL (0x68/sizeof(em_uint32))



#define MAKE_BIT_ONE(number,bit)  (number | (0x1<<bit))
#define MAKE_BIT_ZERO(number,bit)  (number & ~(0x1<<bit))



static em_uint32 em_raspi_mini_uart_extra_status(){
	volatile em_uint32 * address=mini_uart+PERIPHERALS_MINI_UART_EXTRA_STATUS_PHYSICAL;
	em_uint32 val=arm_read(address);
	em_log(EM_LOG_INFO,0,"mini_uart extra status %u\n",val);

	if(val & 0x00000001)
			em_log(EM_LOG_INFO,0,"receiver fifo can not accept\n");
		else em_log(EM_LOG_INFO,0,"receiver fifo can  accept\n");

	if(val & 0x00000002)
		em_log(EM_LOG_INFO,0,"transmitter fifo can  accept\n");
	else em_log(EM_LOG_INFO,0,"transmitter fifo can not  accept\n");

	if(val & 0x00000004)
			em_log(EM_LOG_INFO,0,"receiver is idle\n");
		else em_log(EM_LOG_INFO,0,"receiver is not idle\n");
	if(val & 0x00000008)
				em_log(EM_LOG_INFO,0,"transmitter is idle\n");
			else em_log(EM_LOG_INFO,0,"transmitter is not idle\n");

	if(val & 0x00000010)
		em_log(EM_LOG_INFO,0,"receiver is overrrun\n");
	else em_log(EM_LOG_INFO,0,"receiver is not overrun\n");

	if(val & 0x00000020)
			em_log(EM_LOG_INFO,0,"transmit fifo is full\n");
		else em_log(EM_LOG_INFO,0,"transmit fifo is not full\n");

	if(val & 0x00000040)
				em_log(EM_LOG_INFO,0,"rts is 1\n");
			else em_log(EM_LOG_INFO,0,"rts is 0\n");

	if(val & 0x00000080)
					em_log(EM_LOG_INFO,0,"cts is 1\n");
				else em_log(EM_LOG_INFO,0,"cts is 0\n");

	if(val & 0x00000100)
		em_log(EM_LOG_INFO,0,"transmit fifo is empty\n");
		else em_log(EM_LOG_INFO,0,"transmit fifo is not empty\n");

	if(val & 0x00000200)
			em_log(EM_LOG_INFO,0,"transmitter done\n");
			else em_log(EM_LOG_INFO,0,"transmit is not done\n");

	em_uint32 val_temp = (val << 12)>>28;
	em_log(EM_LOG_INFO,0,"receive fifo level %u\n",val_temp);

	val_temp = (val << 4)>>28;
	em_log(EM_LOG_INFO,0,"transmit fifo level %u\n",val_temp);



	volatile em_uint32 * address_baudrate=mini_uart+PERIPHERALS_MINI_UART_BAUDRATE_PHYSICAL;
		em_uint32 val_baudrate=arm_read(address_baudrate);
		em_log(EM_LOG_INFO,0,"baudreate is: %u\n",val_baudrate);
		val_baudrate &=0x0000FFFF;
		em_log(EM_LOG_INFO,0,"baudreate is: %u\n",val_baudrate);


}

em_uint32 em_raspi_mini_uart_start(em_uint32 options,em_uint32 baudrate,em_uint32 clock_frequency_mhz){

	volatile em_uint32 * address_enables=mini_uart+PERIPHERALS_MINI_UART_ENABLES_PHYSICAL;
	volatile em_uint32 * address_extra_control=mini_uart+PERIPHERALS_MINI_UART_EXTRA_CONTROL_PHYSICAL;
	volatile em_uint32 * address_interrupt_enable=mini_uart+PERIPHERALS_MINI_UART_INTERRUPT_ENABLE_PHYSICAL;
	volatile em_uint32 * address_interrupt_identify=mini_uart+PERIPHERALS_MINI_UART_INTERRUPT_IDENTIFY_PHYSICAL;
	volatile em_uint32 * address_line_control=mini_uart+PERIPHERALS_MINI_UART_LINE_CONTROL_PHYSICAL;
	em_uint32 val_enables=arm_read(address_enables);

		arm_write(address_enables,MAKE_BIT_ZERO(val_enables,0));
		arm_write(address_enables,MAKE_BIT_ONE(val_enables,0));
		arm_write(address_extra_control,0x0);
		arm_write(address_interrupt_enable,0x0);
		arm_write(address_line_control,0x0);
		arm_write(address_interrupt_identify,0xC6);

	em_io_gpio_mode(EM_GPIO_3,EM_MODE_GPIO_FUNC5);
	em_io_gpio_pull(EM_GPIO_3,EM_PULL_OFF);

	em_io_gpio_mode(EM_GPIO_4,EM_MODE_GPIO_FUNC5);
	em_io_gpio_pull(EM_GPIO_4,EM_PULL_OFF);

			//arm_write(address_interrupt_enable,0x3);


						      em_uint32 val_line_control=0;

						        if(options & EM_MINI_UART_DATA_7BIT_ENABLE){
						        	val_line_control =0x0;
						        }
						        if(options & EM_MINI_UART_DATA_8BIT_ENABLE){
						                	val_line_control=0x3;

						                }
					        	//val_line_control=0x3;
						        arm_write(address_line_control,val_line_control);


        volatile em_uint32 * address_baudrate=mini_uart+PERIPHERALS_MINI_UART_BAUDRATE_PHYSICAL;
        arm_write(address_baudrate, (clock_frequency_mhz*1000000/(8*baudrate))-1);




        if(options & EM_MINI_UART_RECEIVE_ENABLE){
        	em_uint32 val_extra_control=arm_read(address_extra_control);
               	arm_write(address_extra_control, MAKE_BIT_ONE(val_extra_control,0));
        }
               else{
            	   em_uint32 val_extra_control=arm_read(address_extra_control);
            	   arm_write(address_extra_control, MAKE_BIT_ZERO(val_extra_control,0));

               }

               if(options & EM_MINI_UART_TRANSMIT_ENABLE){
            	   em_uint32 val_extra_control=arm_read(address_extra_control);
                       	arm_write(address_extra_control, MAKE_BIT_ONE(val_extra_control,1));
               }
                       else {
                    	   em_uint32 val_extra_control=arm_read(address_extra_control);
                    	   arm_write(address_extra_control, MAKE_BIT_ZERO(val_extra_control,1));
               }




}

em_uint32 em_raspi_mini_uart_stop(){
	volatile em_uint32 * address_enables=mini_uart+PERIPHERALS_MINI_UART_ENABLES_PHYSICAL;
		volatile em_uint32 * address_extra_control=mini_uart+PERIPHERALS_MINI_UART_EXTRA_CONTROL_PHYSICAL;
		volatile em_uint32 * address_interrupt_enable=mini_uart+PERIPHERALS_MINI_UART_INTERRUPT_ENABLE_PHYSICAL;
		volatile em_uint32 * address_interrupt_identify=mini_uart+PERIPHERALS_MINI_UART_INTERRUPT_IDENTIFY_PHYSICAL;
		volatile em_uint32 * address_line_control=mini_uart+PERIPHERALS_MINI_UART_LINE_CONTROL_PHYSICAL;
		em_uint32 val_enables=arm_read(address_enables);

			arm_write(address_enables,MAKE_BIT_ZERO(val_enables,0));
			arm_write(address_extra_control,0x0);
			arm_write(address_interrupt_enable,0x0);
			arm_write(address_line_control,0x0);
			arm_write(address_interrupt_identify,0xC6);
}
em_uint32 em_raspi_mini_uart_write_available(){

	volatile em_uint32 * address_line_status=mini_uart+PERIPHERALS_MINI_UART_LINE_STATUS_PHYSICAL;
				em_uint32 val_line_status=arm_read(address_line_status);
				if(val_line_status &   (0x1<<5))
					return EM_SUCCESS ;
				else{

					return EM_ERROR_IO_READ;
				}
}


em_uint32 em_raspi_mini_uart_write(em_uint8 data){
		if(em_raspi_mini_uart_write_available()== EM_SUCCESS){
	volatile em_uint32 * address=mini_uart+PERIPHERALS_MINI_UART_DATA_PHYSICAL;
	arm_write(address,data);
	return EM_SUCCESS;
		}

	return EM_ERROR_IO_WRITE;
}


em_uint32 em_raspi_mini_uart_read_available(){

	volatile em_uint32 * address_line_status=mini_uart+PERIPHERALS_MINI_UART_LINE_STATUS_PHYSICAL;
					em_uint32 val_line_status=arm_read(address_line_status);
					if(val_line_status &   (0x1))return EM_SUCCESS ;else{

						return EM_ERROR_IO_READ;
					}
}

em_uint32 em_raspi_mini_uart_read(em_uint8 *data){

			if(em_raspi_mini_uart_read_available()== EM_SUCCESS){
	volatile em_uint32 * address=mini_uart+PERIPHERALS_MINI_UART_DATA_PHYSICAL;
	*data=arm_read(address);

	return EM_SUCCESS;
			}
			return EM_ERROR_IO_READ;
}



#define PERIPHERALS_UART_DATA_PHYSICAL (0x0)
#define PERIPHERALS_UART_RECEIVE_STATUS_PHYSICAL (0x4/(sizeof(em_uint32)))
#define PERIPHERALS_UART_FLAG_PHYSICAL (0x18/(sizeof(em_uint32)))
#define PERIPHERALS_UART_INTEGER_BAUDRATE_PHYSICAL (0x24/(sizeof(em_uint32)))
#define PERIPHERALS_UART_FRACTIONAL_BAUDRATE_PHYSICAL (0x28/(sizeof(em_uint32)))
#define PERIPHERALS_UART_LINE_CONTROL_PHYSICAL (0x2C/(sizeof(em_uint32)))
#define PERIPHERALS_UART_CONTROL_PHYSICAL (0x30/(sizeof(em_uint32)))
#define PERIPHERALS_UART_INTERRUPT_FIFO_LEVEL_PHYSICAL (0x34/(sizeof(em_uint32)))
#define PERIPHERALS_UART_INTTERRUPT_MASK_PHYSICAL (0x38/(sizeof(em_uint32)))
#define PERIPHERALS_UART_INTERRUPT_RAW_STATUS_PHYSICAL (0x3C/(sizeof(em_uint32)))
#define PERIPHERALS_UART_INTERRUPT_MASK_STATUS_PHYSICAL (0x40/(sizeof(em_uint32)))
#define PERIPHERALS_UART_INTERRUPT_CLEAR_PHYSICAL (0x44/(sizeof(em_uint32)))
#define PERIPHERALS_UART_DMA_CONTROL_PHYSICAL (0x48/(sizeof(em_uint32)))






em_uint32 em_raspi_uart_stop(){
	    volatile em_uint32 * address_control=uart+PERIPHERALS_UART_CONTROL_PHYSICAL;
	    em_uint32 val_control=arm_read(address_control);
		arm_write(address_control,MAKE_BIT_ZERO(val_control,0));//disable uart

		em_io_delay_loops(150);//wait for finish everything

		volatile em_uint32 *address_line_control=uart+PERIPHERALS_UART_LINE_CONTROL_PHYSICAL;
		em_uint32 val_line=arm_read(address_control);
		arm_write(address_line_control,MAKE_BIT_ZERO(val_line,4));//clear fifos
}

em_uint32 em_raspi_uart_start(em_uint32 options,em_uint32 baudrate){

	em_raspi_mini_uart_stop();
	volatile em_uint32 *address_data=uart+PERIPHERALS_UART_DATA_PHYSICAL;

    volatile em_uint32 *address_receive_status=uart+PERIPHERALS_UART_RECEIVE_STATUS_PHYSICAL;
    volatile em_uint32 *address_flag=uart+PERIPHERALS_UART_FLAG_PHYSICAL;
   	arm_write(address_flag,0x90);
   	 volatile em_uint32 *address_integer_baud=uart+PERIPHERALS_UART_INTEGER_BAUDRATE_PHYSICAL;
   	 volatile em_uint32 *address_fractional_baud=uart+PERIPHERALS_UART_FRACTIONAL_BAUDRATE_PHYSICAL;
   	 volatile em_uint32 *address_interrupt_clear=uart+PERIPHERALS_UART_INTERRUPT_CLEAR_PHYSICAL;
   	 arm_write(address_interrupt_clear,0);//clear interrupt mask status we dont need them
   	 volatile em_uint32 *address_interrupt_fifo_level=uart+PERIPHERALS_UART_INTERRUPT_FIFO_LEVEL_PHYSICAL;
   	 //	arm_write(address_interrupt_fifo_level,0);//clear interrupt fifo level we dont need them

	volatile em_uint32 *address_interrupt_mask=uart+PERIPHERALS_UART_INTERRUPT_MASK_STATUS_PHYSICAL;
	arm_write(address_interrupt_mask,0);//clear interrupt mask we dont need them


	volatile em_uint32 *address_interrupt_raw_status=uart+PERIPHERALS_UART_INTERRUPT_RAW_STATUS_PHYSICAL;
	arm_write(address_interrupt_raw_status,0);//clear interrupt raw status we dont need them



	em_io_gpio_mode(EM_GPIO_3,EM_MODE_GPIO_FUNC0);
	em_io_gpio_pull(EM_GPIO_3,EM_PULL_OFF);

	em_io_gpio_mode(EM_GPIO_4,EM_MODE_GPIO_FUNC0);
	em_io_gpio_pull(EM_GPIO_4,EM_PULL_OFF);



     em_uint32 val_line_control=0;
     if(options & EM_UART_PARITY_ENABLE)
    	 val_line_control |= 0x1<<1;
     if(options & EM_UART_PARITY_EVEN_ENABLE)
    	 val_line_control |= 0x1<<2;
     if(options & EM_UART_TWO_STOPBITS_ENABLE)
         val_line_control |= 0x1<<3;
     if(options & EM_UART_FIF0_ENABLE)
         val_line_control |= 0x1<<4;

     if(options & EM_UART_DATA_5BIT_ENABLE)
         val_line_control |= 0x00;
     else if(options & EM_UART_DATA_6BIT_ENABLE)
     	 val_line_control |= 0x1<<5;
     else
     if(options & EM_UART_DATA_7BIT_ENABLE)
          val_line_control |= 0x2<<5;
     else if(options & EM_UART_DATA_8BIT_ENABLE)
      	 val_line_control |= 0x3<<5;

     if(options & EM_UART_SEND_BREAK_ENABLE)
         val_line_control |= 0x1;

     volatile em_uint32 *address_line_control=uart+PERIPHERALS_UART_LINE_CONTROL_PHYSICAL;

     		arm_write(address_line_control,val_line_control);

    em_double64 float_baud=3000000.0/(16*baudrate);
    em_uint32 integer_baud=float_baud;
    em_uint32 fractional_baud=((float_baud-integer_baud*1.0)*64)+0.5f;


    //em_log(EM_LOG_INFO,0,"integer rate %u,%u\n", arm_read(address_integer_baud),integer_baud);
    arm_write(address_integer_baud,integer_baud);

    //em_log(EM_LOG_INFO,0,"fractional rate %u,%u\n",arm_read(address_fractional_baud),fractional_baud);
      arm_write(address_fractional_baud,fractional_baud);

        em_io_delay_loops(150);

        volatile em_uint32 * address_control=uart+PERIPHERALS_UART_CONTROL_PHYSICAL;
     		 em_uint32 val_control=arm_read(address_control);
     		val_control |= 0x1;
     		          if(options & EM_UART_TRANSMIT_ENABLE)
     		         	 val_control |= 0x1<<8;
     		          if(options & EM_UART_RECEIVE_ENABLE)
     		         	 val_control |= 0x1<<9;


     		          arm_write(address_control,val_control);


	return EM_SUCCESS;
}
em_uint32 em_raspi_uart_read_available(){
	 volatile em_uint32 *address_flag=uart+PERIPHERALS_UART_FLAG_PHYSICAL;
	 em_uint32 val_flag=arm_read(address_flag);

	 if(val_flag & (0x1<<4))
		 return EM_ERROR_IO_READ;
	 return EM_SUCCESS;


}

em_uint32 em_raspi_uart_read(em_uint8 *val){
    if(em_raspi_uart_read_available()==EM_SUCCESS)
    {
    volatile em_uint32 *address_data=uart+PERIPHERALS_UART_DATA_PHYSICAL;
    	 em_uint32 temp_val=arm_read(address_data);
    	 *val=temp_val;

    	 temp_val = (temp_val >> 8);

         if(temp_val)
        	 return EM_ERROR_IO_READ;

   return EM_SUCCESS;
    }return EM_ERROR_IO_READ;

}

em_uint32 em_raspi_uart_write_available(){
	 volatile em_uint32 *address_flag=uart+PERIPHERALS_UART_FLAG_PHYSICAL;
	 em_uint32 val_flag=arm_read(address_flag);

	 if(val_flag & (0x1<<7))
		 return EM_SUCCESS;
	 return EM_ERROR_IO_WRITE;


}

em_uint32 em_raspi_uart_write(em_uint8 val){
    if(em_raspi_uart_write_available()==EM_SUCCESS)
    {

    volatile em_uint32 *address_data=uart+PERIPHERALS_UART_DATA_PHYSICAL;
    arm_write(address_data,val);
      return EM_SUCCESS;
    }
    return EM_ERROR_IO_WRITE;

}

#define PERIPHERALS_PWM_CONTROL_PHYSICAL (0x0)
#define PERIPHERALS_PWM_STATUS_PHYSICAL (0x4/sizeof(em_uint32))
#define PERIPHERALS_PWM_DMA_PHYSICAL (0x8/sizeof(em_uint32))
#define PERIPHERALS_PWM_CHANNEL1_RANGE_PHYSICAL (0x10/sizeof(em_uint32))
#define PERIPHERALS_PWM_CHANNEL1_DATA_PHYSICAL (0x14/sizeof(em_uint32))
#define PERIPHERALS_PWM_FIFO_INPUT_PHYSICAL (0x18/sizeof(em_uint32))
#define PERIPHERALS_PWM_CHANNEL2_RANGE_PHYSICAL (0x20/sizeof(em_uint32))
#define PERIPHERALS_PWM_CHANNEL2_DATA_PHYSICAL (0x24/sizeof(em_uint32))
#define PERIPHERALS_PWM_CLOCK_OFFSET_PHYSICAL (0xa0/sizeof(em_uint32))
#define PERIPHERALS_PWM_CLOCK_OFFSET_DIV_PHYSICAL (0xa4/sizeof(em_uint32))



em_uint32 em_raspi_pwm_status(){
	volatile * address_stat=pwm+PERIPHERALS_PWM_STATUS_PHYSICAL;
		em_uint32 val_stat=arm_read(address_stat);
		if(val_stat & (0x1<<0))
			em_log(EM_LOG_INFO,0,"pwm fifo is full\n");
		if(val_stat & (0x1<<1))
					em_log(EM_LOG_INFO,0,"pwm fifo is empty\n");
		if(val_stat & (0x1<<2))
					em_log(EM_LOG_INFO,0,"pwm fifo write error\n");
		if(val_stat & (0x1<<3))
					em_log(EM_LOG_INFO,0,"pwm fifo read error\n");

		em_log(EM_LOG_INFO,0,"pwm status is %u \n",val_stat);
       *address_stat=val_stat;
       val_stat=arm_read(address_stat);
       em_log(EM_LOG_INFO,0,"pwm status is2 %u \n",val_stat);
       return EM_SUCCESS;

}

static em_uint32 pwm_channel_0_use_fifo=0;

em_uint32 em_raspi_pwm_start(em_uint32 channel, em_uint32 options,em_uint32 range,em_uint16 divi,em_uint16 divif){

	if(channel !=0)
		return EM_ERROR_NOT_IMPLEMENTED;

	volatile * address_control=pwm+PERIPHERALS_PWM_CONTROL_PHYSICAL;
	        em_uint32 val_control_old=arm_read(address_control);
			*address_control=0;
	        volatile *address_pwm_clock=pwm_clock+PERIPHERALS_PWM_CLOCK_OFFSET_PHYSICAL;

	        		*address_pwm_clock=((0x5a<<24) | 0x01);
	        		em_raspi_busy_wait(110);
	        		while(((*address_pwm_clock) & 0x80)!=0){
	        			//is_busy
	        			em_io_delay_loops(10);
	        		}
	        		volatile * address_range=pwm+PERIPHERALS_PWM_CHANNEL1_RANGE_PHYSICAL;
	        			*address_range=range;
	        			em_io_delay_microseconds(150);

	        		volatile *address_pwm_clock_div=pwm_clock+PERIPHERALS_PWM_CLOCK_OFFSET_DIV_PHYSICAL;
	        				em_uint32 val_clock_div=0x5a<<24;
	        				val_clock_div |= ((divi) << 12);
	        				val_clock_div |= (divif);
	        				*address_pwm_clock_div=val_clock_div;

	        				em_uint32 val_pwm_clock = 0x5a << 24;
	        				val_pwm_clock |=0x11;
	        				*address_pwm_clock=val_pwm_clock;

  em_io_delay_microseconds(150);

	em_uint32 val_control=0;

	em_io_gpio_mode(EM_GPIO_6,EM_MODE_GPIO_FUNC5);//setting gpio to pwm mode

    if(options & EM_PWM_MODE_DEFAULT)
    	val_control=MAKE_BIT_ZERO(val_control,1);
    else val_control=MAKE_BIT_ONE(val_control,1);

	val_control=MAKE_BIT_ZERO(val_control,2);//
	val_control=MAKE_BIT_ZERO(val_control,3);//
	val_control=MAKE_BIT_ZERO(val_control,4);//
	if(options & EM_PWM_USE_FIF0){
		val_control=MAKE_BIT_ONE(val_control,5);//
		pwm_channel_0_use_fifo=1;
	}
	else val_control=MAKE_BIT_ZERO(val_control,5);//


    val_control=MAKE_BIT_ONE(val_control,0);//enable pwm

		*address_control=val_control;//set value to control
		em_io_delay_microseconds(150);
		em_raspi_pwm_status();




	return EM_SUCCESS;

}


em_uint32 em_raspi_pwm_write(em_uint32 channel,em_uint32 data){


	if(pwm_channel_0_use_fifo){
		volatile * address_fifo=pwm+PERIPHERALS_PWM_FIFO_INPUT_PHYSICAL;
				*address_fifo=data;
				volatile * address_stat=pwm+PERIPHERALS_PWM_STATUS_PHYSICAL;
				em_uint32 val_stat=arm_read(address_stat);
				if(val_stat & (0x1<<2)){
					*address_stat=val_stat;
					 return EM_ERROR_IO_WRITE;
				}



	}else{
	volatile * address_data=pwm+PERIPHERALS_PWM_CHANNEL1_DATA_PHYSICAL;
	*address_data=data;
	}



	return EM_SUCCESS;
}

#define PERIPHERALS_BSC_CONTROL_PHYSICAL (0x0)
#define PERIPHERALS_BSC_STATUS_PHYSICAL (0x04/sizeof(em_uint32))
#define PERIPHERALS_BSC_DATA_LENGHT_PHYSICAL (0x08/sizeof(em_uint32))
#define PERIPHERALS_BSC_SLAVE_ADDRESS_PHYSICAL (0x0c/sizeof(em_uint32))
#define PERIPHERALS_BSC_DATA_FIFO_PHYSICAL (0x010/sizeof(em_uint32))
#define PERIPHERALS_BSC_CLOCK_DIVIDER_PHYSICAL (0x14/sizeof(em_uint32))
#define PERIPHERALS_BSC_DATA_DELAY_PHYSICAL (0x18/sizeof(em_uint32))
#define PERIPHERALS_BSC_CLOCK_STRETCH_PHYSICAL (0x1c/sizeof(em_uint32))




em_uint32 em_raspi_i2c_start(em_uint8 channel,em_uint16 divider,em_uint16 timeout){

	volatile *address_div=0;
	volatile *address_timeout=0;
	volatile *address_delay=0;
		if(channel & EM_USE_BSC0)
		{
			return EM_ERROR_NOT_IMPLEMENTED;
		}
		else if(channel & EM_USE_BSC1)
			{
			 em_raspi_gpio_mode(EM_GPIO_0,EM_MODE_GPIO_FUNC0);
			 em_raspi_gpio_mode(EM_GPIO_1,EM_MODE_GPIO_FUNC0);
			 address_div=(bsc1+PERIPHERALS_BSC_CLOCK_DIVIDER_PHYSICAL);
			 address_timeout=(bsc1+PERIPHERALS_BSC_CLOCK_STRETCH_PHYSICAL);
			 address_delay=(bsc1+PERIPHERALS_BSC_DATA_DELAY_PHYSICAL);
			 if(divider>0){
			 arm_write(address_div,divider);
			 arm_write(address_delay,divider/3);
			 }
			 if(timeout>0)
				 arm_write(address_timeout,timeout);
			}
		else return EM_ERROR_NOT_IMPLEMENTED;




  return EM_SUCCESS;
}

em_uint32 em_raspi_i2c_write(em_uint8 channel,em_uint16 address,const em_uint8 * const data,em_uint32 data_lenght){

	if(channel & EM_USE_BSC0)
			{
				return EM_ERROR_NOT_IMPLEMENTED;
			}
	volatile * address_control=bsc1+PERIPHERALS_BSC_CONTROL_PHYSICAL;
	volatile * address_length=bsc1+PERIPHERALS_BSC_DATA_LENGHT_PHYSICAL;
	volatile * address_status=bsc1+PERIPHERALS_BSC_STATUS_PHYSICAL;
    volatile * address_slave=bsc1+PERIPHERALS_BSC_SLAVE_ADDRESS_PHYSICAL;
    volatile * address_fifo=bsc1+PERIPHERALS_BSC_DATA_FIFO_PHYSICAL;
   // volatile * address_div=(bsc1+PERIPHERALS_BSC_CLOCK_DIVIDER_PHYSICAL);
   // em_uint32 divider=arm_read(address_div);
   // em_uint32 wait_one_byte_transfer_clock=((divider*1.0)/250000000)*1000000*9;
    arm_write(address_length,data_lenght);
    arm_write(address_slave,address);
    arm_write(address_status,(0x1<<9) | (0x1<<8) | (0x1 << 1));//clock strecth,ACK error,Done clear
    arm_write(address_control, (0x1<<4));//clear fifo and done bit
    arm_write(address_fifo,data[0]);
    arm_write(address_control,(0x1<<7)|(0x1<<15));
    //em_io_delay_loops(wait_one_byte_transfer_clock);




    em_uint32 index=1;
    while(1){
    	em_uint32 stat=arm_read(address_status);

    	if(stat & (0x1<<1))//transfer done
    		break;

    	while((index < data_lenght) &&  (stat & (0x1<<4)))//txd fifo can accept data
    	{

          *address_fifo=data[index++];
          //em_io_delay_loops(wait_one_byte_transfer_clock*10000);

    	}

    }


    	em_uint32 stat=arm_read(address_status);

    	    	if(stat & ( (0x1<<8) | (0x1 <<9)) ){
    	    		//em_log(EM_LOG_INFO,0,"write stat error is %u\n",stat);
    	    		return EM_ERROR_IO_WRITE;
    	    	}
    	    	if(index<data_lenght){
    	    		//em_log(EM_LOG_INFO,0,"write stat index  is %u\n",index);
    	    		return EM_ERROR_IO_WRITE;
    	    	}


    return EM_SUCCESS;


}

em_uint32 em_raspi_i2c_read(em_uint8 channel,em_uint16 address,em_uint8 *data,em_uint32 *data_lenght){

	if(channel & EM_USE_BSC0)
			{
				return EM_ERROR_NOT_IMPLEMENTED;
			}
	volatile * address_control=bsc1+PERIPHERALS_BSC_CONTROL_PHYSICAL;
	volatile * address_length=bsc1+PERIPHERALS_BSC_DATA_LENGHT_PHYSICAL;
	volatile * address_status=bsc1+PERIPHERALS_BSC_STATUS_PHYSICAL;
    volatile * address_slave=bsc1+PERIPHERALS_BSC_SLAVE_ADDRESS_PHYSICAL;
    volatile * address_fifo=bsc1+PERIPHERALS_BSC_DATA_FIFO_PHYSICAL;
    arm_write(address_length,*data_lenght);
    arm_write(address_control, (0x1<<4));//clear fifo
    arm_write(address_slave,address);
    arm_write(address_status,(0x1<<9) | (0x1<<8) | (0x1 << 1));
    arm_write(address_control,(0x1<<7) | (0x1<<15) | (0x01));

    em_uint32 index=0;
    while(1){


    	while(arm_read(address_status) & (0x1<<5)){
    		if(index<*data_lenght)
        data[index]= arm_read(address_fifo);
       // em_log(EM_LOG_INFO,0,"read data is %u\n",data[index]);
        index++;
    	continue;
    	}
    	em_uint32 stat=arm_read(address_status);
    	if(stat & (0x1<<1))//transfer done
    		break;
    }
    em_uint32 stat=arm_read(address_status);

        	    	if(stat & ( (0x1<<8) | (0x1 <<9)) ){
        	    		//em_log(EM_LOG_INFO,0,"write stat error is %u\n",stat);
        	    		*data_lenght=index;
        	    		return EM_ERROR_IO_READ;
        	    	}

        	    	if(index>*data_lenght){
        	    		//em_log(EM_LOG_INFO,0,"write stat index  is %u\n",index);
        	    		*data_lenght=index;
        	    		return EM_ERROR_IO_READ;
        	    	}
        	    	*data_lenght=index;

    return EM_SUCCESS;


}


em_uint32 em_raspi_i2c_stop(em_uint8 channel){
	volatile *address=0;
	if(channel & EM_USE_BSC0)
		address=(bsc0+PERIPHERALS_BSC_CONTROL_PHYSICAL);
	else if(channel & EM_USE_BSC1)
		address=(bsc1+PERIPHERALS_BSC_CONTROL_PHYSICAL);
	else return EM_ERROR_NOT_IMPLEMENTED;
	arm_write(address,0);
	return EM_SUCCESS;
}





em_uint32 em_raspi_test(){




	return EM_SUCCESS;
}









