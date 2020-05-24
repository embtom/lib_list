/*
 * This file is part of the EMBTOM project
 * Copyright (c) 2018-2020 Thomas Willetal 
 * (https://github.com/embtom)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _LOCK_MEM_H_
#define _LOCK_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* *******************************************************************
 * includes
 * ******************************************************************/

/* c-runtime */
#ifndef __KERNEL__
	#include <string.h>
	#include <stdint.h>
#else
	#include <linux/string.h>
	#include <linux/types.h>
#endif

/* project */
#include "lib_list_types.h"
#include "lock_mem_types.h"

/* *******************************************************************
 * Static Inline Functions
 * ******************************************************************/

/* ************************************************************************//**
 * \brief	Initialization of a semilock object - Based on shared memory
 *
 *	The utilized mutual exclusion algorithm is the "Filter Lock" is derived
 *	form the "Peterson Lock" The difference is that the "Peterson Lock" uses
 *	a two-element "interested_context" variable to indicate if a thread is trying
 *	to enter a critical section. The "Filter Lock" generalizes the request of
 *	"interested_context" to N number of contexts.
 *
 * \param	*_lock [out]		The semilock_t data type will be initialized
 * \return 	EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * ****************************************************************************/
static inline int memlock__init(semilock_t *_lock)
{
	if(_lock == NULL) {
		return -LIB_LIST__EPAR_NULL;
	}

	memset(_lock, 0, sizeof(semilock_t));
	return LIB_LIST__EOK;
}

/* ************************************************************************//**
 * \brief	Lock of the semilock object - Based on shared memory
 *
 *	The contexed_id's are sequential number defined between the range of
 *		(0 <= _context_id < M_DEV_NUMBER_OF_LOCK_CONTEXT)
 *
 *	The define M_DEV_NUMBER_OF_LOCK_CONTEXT is responsible to define the
 *	number to _contexts to handle
 *
 * \param	*_lock [in]	 	The semilock_t data type to check if section
 *							is already entered
 *			_context_id		The
 *
 * \return 	EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * ****************************************************************************/
static inline int memlock__lock(semilock_t *_lock, uint32_t _context_id)
{
	int lock_count, k;
	static volatile unsigned int lock_wait = 0;

	if (_lock == NULL) {
		return -LIB_LIST__EPAR_NULL;
	}

	/*Check if the passed context ID is greater than the number of defined contexts */
	if(_context_id >= M_DEV_NUMBER_OF_LOCK_CONTEXT) {
		return -LIB_LIST__ESTD_FAULT;
	}

	for (lock_count = 1; lock_count < M_DEV_NUMBER_OF_LOCK_CONTEXT; lock_count++) {
		_lock->interested_context[_context_id] = lock_count;
		_lock->victim[lock_count] = _context_id;
		for (k = 0; k < M_DEV_NUMBER_OF_LOCK_CONTEXT; k++) {
		      while ((k != _context_id) && (_lock->interested_context[k] >= lock_count && _lock->victim[lock_count] == _context_id)) {
		    	  /*Busy waiting - volatile variable is used to prevent for optimizing out a busy wait loop */
		    	  lock_wait++;
		      }
		}
	}
	return LIB_LIST__EOK;
}

/* ************************************************************************//**
 * \brief	Unlock of the semilock object - Based on shared memory
 *
 * \param	*_lock [in]	 	The semilock_t data type to signalizes that a section
 * 							is left
 *
 * \return 	EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * ****************************************************************************/
static inline int memlock__unlock(semilock_t *_lock, uint32_t _context_id)
{
	if (_lock == NULL) {
			return -LIB_LIST__EPAR_NULL;
	}

	/*Check if the passed context ID is greater than the number of defined contexts */
	if(_context_id >= M_DEV_NUMBER_OF_LOCK_CONTEXT) {
		return -LIB_LIST__ESTD_FAULT;
	}

	_lock->interested_context[_context_id] = 0;
	return LIB_LIST__EOK;
}

#ifdef __cplusplus
}
#endif

#endif


