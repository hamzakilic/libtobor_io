/*
 * io_raspi.h
 *
 *  Created on: Apr 7, 2015
 *      Author: root
 */

#ifndef EM_IO_RASPI_H_
#define EM_IO_RASPI_H_

#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


#include "em_error.h"
#include "em_log.h"
#include "em_common.h"
#include "em_pin.h"
#include "em_types.h"




em_uint32 em_raspi_initialize(em_uint32 system);

em_uint32 em_raspi_gpio_mode(em_uint8 number,em_uint8 mode);

em_uint32 em_raspi_gpio_read(em_uint8 number,em_uint8 *val);
em_uint32 em_raspi_gpio_write(em_uint8 number,em_uint8 val);

em_uint32 em_raspi_gpio_pull(em_uint8 number,em_uint8 mode);
em_uint32 em_raspi_set_event(em_uint8 number,em_uint8 event,em_uint8 enable_or_disable);
em_uint32 em_raspi_read_event(em_uint8 number,em_uint8* val);
em_uint32 em_raspi_current_time(em_uint64 *time_value);
em_uint32 em_raspi_busy_wait(em_uint64  micro_seconds);
em_uint32 em_raspi_mini_uart_start(em_uint32 options,em_uint32 baudrate);
em_uint32 em_raspi_mini_uart_stop();
em_uint32 em_raspi_mini_uart_write(em_uint8 data);
em_uint32 em_raspi_mini_uart_read(em_uint8 *data);





#endif /* EM_IO_RASPI_H_ */
