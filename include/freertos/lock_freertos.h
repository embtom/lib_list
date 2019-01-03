/*
 * This file is part of the EMBTOM project
 * Copyright (c) 2018-2019 Thomas Willetal 
 * (https://github.com/tom3333)
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

#ifndef _LOCK_FREERTOS_H_
#define _LOCK_FREERTOS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* *******************************************************************
 * includes
 * ******************************************************************/

/* system */
#include <FreeRTOS.h>
#include <task.h>

/* own libs */
#include <lib_convention__errno.h>

/* project */
#include "lock_freertos_types.h"

/* *******************************************************************
 * Static Inline Functions
 * ******************************************************************/

/* ************************************************************************//**
 * \brief	Initialization of the FREERTOS critical section lock
 *
 * \param	*_lock [out]		The semilock_t data type will be initialized
 * \return 	EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * ****************************************************************************/
static inline int freertos_lock__init(semilock_t *_lock)
{
	if(_lock == NULL) {
		return -LIB_LIST__EPAR_NULL;
	}

    *_lock = 0;
    return LIB_LIST__EOK;
}

/* ************************************************************************//**
 * \brief	Lock of the of the FREERTOS critical section
 *
 * \param	*_lock [in]	 	The semilock_t data type to check if section
 * 							is already entered
 * \return 	EOK
 * ****************************************************************************/
static inline int freertos_lock__lock(semilock_t *_lock)
{
	*_lock = taskENTER_CRITICAL_FROM_ISR();
	return EOK;
}

/* ************************************************************************//**
 * \brief	Unlock of the of the FREERTOS critical section
 *
 * \param	*_lock [in]	 	The semilock_t data type to signalizes that a section
 * 							is left
 * \return 	EOK
 * ****************************************************************************/
static inline int freertos_lock__unlock(semilock_t *_lock)
{
	taskEXIT_CRITICAL_FROM_ISR(*_lock);
    return EOK;
}

#ifdef __cplusplus
}
#endif

#endif /* _LOCK_FREERTOS_H_ */
