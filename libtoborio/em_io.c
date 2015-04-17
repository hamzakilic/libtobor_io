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

typedef struct {

	initialize_func initialize;
	gpio_direction_func gpio_mode;
	gpio_read_func gpio_read;
    gpio_write_func gpio_write;
    gpio_pull_func gpio_pull;
    gpio_set_event_func gpio_set_event;
    gpio_read_event_func gpio_read_event;


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


