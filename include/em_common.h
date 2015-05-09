/*
 * common.h
 *
 *  Created on: Apr 7, 2015
 *      Author: root
 */

#ifndef EM_COMMON_H_
#define EM_COMMON_H_

#include "em_types.h"
#include <time.h>

#define EM_DEBUG

#define EM_MODE_GPIO_IN 0x01
#define EM_MODE_GPIO_OUT 0x02
#define EM_MODE_GPIO_FUNC0 0x04
#define EM_MODE_GPIO_FUNC1 0x08
#define EM_MODE_GPIO_FUNC2 0x10
#define EM_MODE_GPIO_FUNC3 0x20
#define EM_MODE_GPIO_FUNC4 0x40
#define EM_MODE_GPIO_FUNC5 0x80





#define EM_GPIO_LOW 0x01
#define EM_GPIO_HIGH 0x02

#define EM_PULL_OFF 0x01
#define EM_PULL_UP 0x02
#define EM_PULL_DOWN 0x04

#define EM_EVENT_DISABLE 0x01
#define EM_EVENT_ENABLE 0x02

#define EM_EVENT_RISING_EDGE_DETECT 0x01
#define EM_EVENT_FALLING_EDGE_DETECT 0x02
#define EM_EVENT_HIGH_DETECT 0x04
#define EM_EVENT_LOW_DETECT 0x08

#define EM_EVENT_DETECTED 0x01
#define EM_EVENT_NOT_DETECTED 0x02


#define EM_PWM_MODE_SERIAL 0x01
#define EM_PWM_MODE_DEFAULT 0x02
#define EM_PWM_USE_FIF0 0x04



#define EM_MINI_UART_TRANSMIT_ENABLE 0x01
#define EM_MINI_UART_RECEIVE_ENABLE 0x02
#define EM_MINI_UART_DATA_7BIT_ENABLE 0x4
#define EM_MINI_UART_DATA_8BIT_ENABLE 0x8



#define EM_UART_TRANSMIT_ENABLE 0x01
#define EM_UART_SEND_BREAK_ENABLE 0x02
#define EM_UART_PARITY_ENABLE 0x4
#define EM_UART_PARITY_EVEN_ENABLE 0x8
#define EM_UART_TWO_STOPBITS_ENABLE 0x10
#define EM_UART_FIF0_ENABLE 0x20
#define EM_UART_DATA_5BIT_ENABLE 0x40
#define EM_UART_DATA_6BIT_ENABLE 0x80
#define EM_UART_DATA_7BIT_ENABLE 0x100
#define EM_UART_DATA_8BIT_ENABLE 0x200
#define EM_UART_RECEIVE_ENABLE 0x0400

#define EM_USE_BSC0 0x01
#define EM_USE_BSC1 0x02












void em_io_delay_microseconds (em_uint32 micro_seconds);
inline void em_io_delay_loops(em_uint32 count);

enum
{
    O32_LITTLE_ENDIAN = 0x03020100ul,
    O32_BIG_ENDIAN = 0x00010203ul,
    O32_PDP_ENDIAN = 0x01000302ul
};

typedef union
{
	em_uint8 bytes[4];
	em_uint32 value;
}endianness;
extern endianness o32_host_order;

#define O32_HOST_ORDER (o32_host_order.value)


#endif /* EM_COMMON_H_ */
