/*
 * log.c
 *
 *  Created on: Apr 6, 2015
 *      Author: root
 */

#include "em_log.h"


void em_log(em_uint8 log_type,em_int32 system_err, const char * fmt, ...) {
#ifdef EM_DEBUG

	va_list arg;
	va_start(arg, fmt);
	vfprintf(stdout, fmt, arg);
	va_end(arg);
	if(system_err!=0)
	{
		em_uint8 error_msg[255];
		strerror_r(system_err,error_msg,255);
		fprintf(stdout,error_msg);

	}
#endif
}
