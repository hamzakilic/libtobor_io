/*
 * emio.h
 *
 *  Created on: Apr 8, 2015
 *      Author: root
 */

#ifndef EM_IO_H_
#define EM_IO_H_

#include "em_common.h"
#include "em_error.h"
#include "em_io_raspi.h"
#include "em_log.h"
#include "em_pin.h"
#include "em_types.h"






#define EM_SYSTEM_RASPI_B_PLUS 0x00
#define EM_SYSTEM_RASPI_B_R2 0x01
#define EM_SYSTEM_RASPI_AB_R2 0x02





//not thread safe
extern em_uint32 em_io_initialize(em_uint32 system);
extern em_uint32 em_io_gpio_mode(em_uint8 number,em_uint8 mode);
extern em_uint32 em_io_gpio_read(em_uint8 number,em_uint8 *val);
extern em_uint32 em_io_gpio_write(em_uint8 number,em_uint8 val);
extern em_uint32 em_io_gpio_pull(em_uint8 number,em_uint8 mode);
extern em_uint32 em_io_gpio_set_event(em_uint8 number,em_uint8 event,em_uint8 enable_ordisable);
extern em_uint32 em_io_gpio_read_event(em_uint8 number,em_uint8* val);







#endif /* EM_IO_H_ */
