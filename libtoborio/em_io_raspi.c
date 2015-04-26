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

static em_uint32 mem_fd;
volatile em_uint32 * gpio;
volatile em_uint32 * timer;
volatile em_uint32 *mini_uart;

#define GPIO_REGISTERS_SIZE  (0x7E2000B0-0x7E200000)
#define TIMER_REGISTERS_SIZE (7*sizeof(em_uint32))
#define MINI_UART_REGISTERS_SIZE  (0xD4/sizeof(em_uint32))

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


	close(mem_fd);
	if (gpio == MAP_FAILED ||  timer == MAP_FAILED || mini_uart==MAP_FAILED) {
		em_log(EM_LOG_FATAL, errno, "map to ram or timer or mini_uart failed\n");
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

       if(temp_val-current_val>=micro_seconds)
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

em_uint32 em_raspi_mini_uart_start(em_uint32 options,em_uint32 baudrate){

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

						        if(options & EM_UART_DATA_7BIT_ENABLE){
						        	val_line_control =0x0;
						        }
						        if(options & EM_UART_DATA_8BIT_ENABLE){
						                	val_line_control=0x3;

						                }
					        	//val_line_control=0x3;
						        arm_write(address_line_control,val_line_control);


        volatile em_uint32 * address_baudrate=mini_uart+PERIPHERALS_MINI_UART_BAUDRATE_PHYSICAL;
        arm_write(address_baudrate, (250000000/(8*baudrate))-1);




        if(options & EM_UART_RECEIVE_ENABLE){
        	em_uint32 val_extra_control=arm_read(address_extra_control);
               	arm_write(address_extra_control, MAKE_BIT_ONE(val_extra_control,0));
        }
               else{
            	   em_uint32 val_extra_control=arm_read(address_extra_control);
            	   arm_write(address_extra_control, MAKE_BIT_ZERO(val_extra_control,0));

               }

               if(options & EM_UART_TRANSMIT_ENABLE){
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









