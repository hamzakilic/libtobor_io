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



