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

#define EM_DIRECTION_IN 0x01
#define EM_DIRECTION_OUT 0x02

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



void em_io_delay_microseconds (em_uint32 micro_seconds);


#endif /* EM_COMMON_H_ */
