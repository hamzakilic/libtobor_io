/*
 * log.h
 *
 *  Created on: Apr 6, 2015
 *      Author: root
 */

#ifndef LIBTOBOR_IO2_INCLUDE_EM_LOG_H_
#define LIBTOBOR_IO2_INCLUDE_EM_LOG_H_

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "em_common.h"
#include "em_types.h"



#define EM_LOG_ERROR 0x00
#define EM_LOG_INFO  0x01
#define EM_LOG_FATAL 0x02

void em_log(em_uint8 log_type,em_int32 system_err, const char * fmt,...);


#endif /* LIBTOBOR_IO2_INCLUDE_EM_LOG_H_ */
