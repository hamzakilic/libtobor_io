/*بِسْــــــــــــــــــــــمِ اﷲِارَّحْمَنِ ارَّحِيم
 *
 *  A user space driver library for raspberry pi
 *
 *  Created on: Apr 8, 2015
 *  Author: Hamza Kılıç
 */

#ifndef LIBTOBOR_IO2_INCLUDE_EM_IO_H_
#define LIBTOBOR_IO2_INCLUDE_EM_IO_H_

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
extern em_uint32 em_io_current_time(em_uint64 *time_value);
extern em_uint32 em_io_busy_wait(em_uint64  micro_seconds);
extern em_uint32 em_io_mini_uart_start(em_uint32 options,em_uint32 baudrate,em_uint32 clock_frequency_mhz);
extern em_uint32 em_io_mini_uart_stop();
extern em_uint32 em_io_mini_uart_write(em_uint8 data);
extern em_uint32 em_io_mini_uart_read(em_uint8 *data);
extern em_uint32 em_io_uart_start(em_uint32 options,em_uint32 baudrate);
extern em_uint32 em_io_uart_stop();
extern em_uint32 em_io_uart_write(em_uint8 data);
extern em_uint32 em_io_uart_read(em_uint8 *data);
extern em_uint32 em_io_pwm_start(em_uint32 channel, em_uint32 options,em_uint32 range,em_uint16 divi,em_uint16 divif);
extern em_uint32 em_io_pwm_write(em_uint32 channel,em_uint32 data);
extern em_uint32 em_io_i2c_start(em_uint8 channel,em_uint16 divider,em_uint16 timeout);
extern em_uint32 em_io_i2c_stop(em_uint8 channel);
extern em_uint32 em_io_i2c_write(em_uint8 channel,em_uint16 address,const em_uint8 * const data,em_uint32 data_lenght,em_uint32 timeout);
extern em_uint32 em_io_i2c_read(em_uint8 channel,em_uint16 address,em_uint8 *data,em_uint32 data_lenght,em_uint32 timeout);

extern em_uint32 em_io_test();







#endif /* LIBTOBOR_IO2_INCLUDE_EM_IO_H_ */
