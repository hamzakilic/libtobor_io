/*
 * types.h
 *
 *  Created on: Apr 6, 2015
 *      Author: root
 */

#ifndef LIBTOBOR_IO2_INCLUDE_EM_TYPES_H_
#define LIBTOBOR_IO2_INCLUDE_EM_TYPES_H_

typedef char em_int8;
typedef unsigned char em_uint8;
typedef em_uint8 em_byte;

typedef short em_int16;
typedef unsigned short em_uint16;

typedef signed int em_int32;
typedef unsigned int em_uint32;

typedef signed long long em_int64;
typedef unsigned long long em_uint64;

typedef float em_float32;
typedef double em_double64;
typedef em_uint32 em_pointer_size;
typedef em_uint8 em_bool;


#define em_true 0
#define em_false 1



#endif /* LIBTOBOR_IO2_INCLUDE_EM_TYPES_H_ */
