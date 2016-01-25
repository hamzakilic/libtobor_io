/*
 * em_common.c

 *
 *  Created on: Apr 12, 2015
 *      Author: root
 */

#include "em_common.h"

void em_io_delay_microseconds (em_uint32 micro_seconds)
{
  struct timespec sleeper ;
  unsigned int uSecs = micro_seconds % 1000000 ;
  unsigned int wSecs = micro_seconds / 1000000 ;


    sleeper.tv_sec  = wSecs ;
    sleeper.tv_nsec = (long)(uSecs * 1000L) ;
    nanosleep (&sleeper, 0) ;

}

inline em_uint32 time_as_microseconds(){
	struct timeval tv;
	    gettimeofday(&tv,0);
	    return 1000000 * tv.tv_sec + tv.tv_usec;
}

inline void em_io_delay_loops(em_uint32 count){
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
			 : : [count]"r"(count) : "cc");
}


endianness o32_host_order={{0,1,2,3}};




