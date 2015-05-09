/*
 * emio.c
 *
 *  Created on: Apr 8, 2015
 *      Author: root
 */

#include "em_io.h"

typedef em_uint32 (*initialize_func)(em_uint32);
typedef em_uint32  (*gpio_direction_func)(em_uint8,em_uint8);
typedef em_uint32 (*gpio_read_func)(em_uint8,em_uint8 *);
typedef em_uint32 (*gpio_write_func)(em_uint8 ,em_uint8);
typedef em_uint32 (*gpio_pull_func)(em_uint8,em_uint8);
typedef em_uint32 (*gpio_set_event_func)(em_uint8 ,em_uint8,em_uint8);
typedef em_uint32 (*gpio_read_event_func)(em_uint8 ,em_uint8 * );
typedef em_uint32 (*current_time_func)(em_uint64 *);
typedef em_uint32  (*busy_wait_func)(em_uint64);
typedef em_uint32 (*mini_uart_start_func)(em_uint32 ,em_uint32,em_uint32 );
typedef em_uint32 (*mini_uart_stop_func)();
typedef em_uint32 (*mini_uart_write_func)(em_uint8);
typedef em_uint32 (*mini_uart_read_func)(em_uint8 *);
typedef em_uint32 (*uart_start_func)(em_uint32 ,em_uint32);
typedef em_uint32 (*uart_stop_func)();
typedef em_uint32 (*uart_write_func)(em_uint8);
typedef em_uint32 (*uart_read_func)(em_uint8 *);
typedef em_uint32 (*test_func)();
typedef em_uint32 (*pwm_start_func)(em_uint32 ,em_uint32 ,em_uint32 ,em_uint16 ,em_uint16);
typedef em_uint32 (*pwm_write_func)(em_uint32,em_uint32);
typedef em_uint32 (*i2c_start_func)(em_uint8 channel,em_uint16 divider,em_uint16 timeout);
typedef em_uint32 (*i2c_stop_func)(em_uint8 channel);
typedef em_uint32 (*i2c_write_func)(em_uint8 channel,em_uint16 address,const em_uint8 * const data,em_uint32 data_lenght);
typedef em_uint32 (*i2c_read_func)(em_uint8 channel,em_uint16 address,em_uint8 *data,em_uint32 *data_lenght);

typedef struct {

	initialize_func initialize;
	gpio_direction_func gpio_mode;
	gpio_read_func gpio_read;
    gpio_write_func gpio_write;
    gpio_pull_func gpio_pull;
    gpio_set_event_func gpio_set_event;
    gpio_read_event_func gpio_read_event;
    current_time_func current_time;
    busy_wait_func busy_wait;
    mini_uart_start_func mini_uart_start;
    mini_uart_stop_func mini_uart_stop;
    mini_uart_write_func mini_uart_write;
    mini_uart_read_func mini_uart_read;
    uart_start_func uart_start;
    uart_stop_func uart_stop;
    uart_write_func uart_write;
    uart_read_func uart_read;
    pwm_start_func pwm_start;
    pwm_write_func pwm_write;
    i2c_start_func i2c_start;
    i2c_stop_func i2c_stop;
    i2c_write_func i2c_write;
    i2c_read_func i2c_read;

    test_func test;


}em_board;

em_board current_board;

//not thread safe
em_uint32 em_io_initialize(em_uint32 system){
    memset(&current_board,0,sizeof(current_board));
    current_board.initialize=em_raspi_initialize;
    current_board.gpio_read=em_raspi_gpio_read;
    current_board.gpio_write=em_raspi_gpio_write;
    current_board.gpio_mode=em_raspi_gpio_mode;
    current_board.gpio_pull=em_raspi_gpio_pull;
    current_board.gpio_set_event=em_raspi_set_event;
    current_board.gpio_read_event=em_raspi_read_event;
    current_board.current_time=em_raspi_current_time;
    current_board.busy_wait=em_raspi_busy_wait;
    current_board.mini_uart_start=em_raspi_mini_uart_start;
    current_board.mini_uart_stop=em_raspi_mini_uart_stop;
    current_board.mini_uart_read=em_raspi_mini_uart_read;
    current_board.mini_uart_write=em_raspi_mini_uart_write;
    current_board.uart_start=em_raspi_uart_start;
    current_board.uart_stop=em_raspi_uart_stop;
    current_board.uart_read=em_raspi_uart_read;
    current_board.uart_write=em_raspi_uart_write;
    current_board.pwm_start=em_raspi_pwm_start;
    current_board.pwm_write=em_raspi_pwm_write;

    current_board.test=em_raspi_test;
    return current_board.initialize(system);

}


em_uint32 em_io_gpio_mode(em_uint8 number,em_uint8 direction){
   return current_board.gpio_mode(number,direction);
}

em_uint32 em_io_gpio_read(em_uint8 number,em_uint8 *val){
  return current_board.gpio_read(number,val);
}
em_uint32 em_io_gpio_write(em_uint8 number,em_uint8 val){
return current_board.gpio_write(number,val);
}
em_uint32 em_io_gpio_pull(em_uint8 number,em_uint8 mode){
	return current_board.gpio_pull(number,mode);
}

em_uint32 em_io_gpio_set_event(em_uint8 number,em_uint8 event,em_uint8 enable_or_disable){
	em_log(EM_LOG_INFO,0,"executing set event\n");
	return current_board.gpio_set_event(number,event,enable_or_disable);
}
em_uint32 em_io_gpio_read_event(em_uint8 number,em_uint8* val){
	return current_board.gpio_read_event(number,val);
}
em_uint32 em_io_current_time(em_uint64 *time_value){
	return current_board.current_time(time_value);
}
em_uint32 em_io_busy_wait(em_uint64  micro_seconds){
return current_board.busy_wait(micro_seconds);
}
em_uint32 em_io_mini_uart_start(em_uint32 options,em_uint32 baudrate,em_uint32 clock_frequency_mhz){
	return current_board.mini_uart_start(options,baudrate,clock_frequency_mhz);
}
em_uint32 em_io_mini_uart_stop(){
	return current_board.mini_uart_stop();

}
em_uint32 em_io_mini_uart_write(em_uint8 data){
	return current_board.mini_uart_write(data);

}
em_uint32 em_io_mini_uart_read(em_uint8 *data){
	return current_board.mini_uart_read(data);
}

em_uint32 em_io_pwm_start(em_uint32 channel, em_uint32 options,em_uint32 range,em_uint16 divi,em_uint16 divif){
	return current_board.pwm_start(channel,options,range,divi,divif);
}
em_uint32 em_io_pwm_write(em_uint32 channel,em_uint32 data){
	return current_board.pwm_write(channel,data);
}

em_uint32 em_io_i2c_start(em_uint8 channel,em_uint16 divider,em_uint16 timeout){
  return current_board.i2c_start(channel,divider,timeout);
}
em_uint32 em_io_i2c_stop(em_uint8 channel){
	return current_board.i2c_stop(channel);
}
em_uint32 em_io_i2c_write(em_uint8 channel,em_uint16 address,const em_uint8 * const data,em_uint32 data_lenght){
	return current_board.i2c_write(channel,address,data,data_lenght);

}
em_uint32 em_io_i2c_read(em_uint8 channel,em_uint16 address,em_uint8 *data,em_uint32 *data_lenght){
   return current_board.i2c_read(channel,address,data,data_lenght);
}


em_uint32 em_io_test(){
	return current_board.test();
}

