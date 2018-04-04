/* ****************************************************************************************************
 * \file	lock_cas.h
 *
 *  compiler:   GNU Tools ARM Embedded (4.7.201xqx)
 *  target:     Cortex Mx
 *  author:		Tom
 * ****************************************************************************************************/

/* ****************************************************************************************************/

/*
 *	******************************* change log *******************************
 *  date			user			comment
 *	06.04.2017	tom	creation
 */

#ifndef SH_LOCK_FREERTOS_H_
#define SH_LOCK_FREERTOS_H_

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
	if(portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK)
		*_lock = taskENTER_CRITICAL_FROM_ISR();
	else
		taskENTER_CRITICAL();
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
	if(portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK)
		taskEXIT_CRITICAL_FROM_ISR(*_lock);
	else
		taskEXIT_CRITICAL();
    return EOK;
}

#endif /* SH_LOCK_FREERTOS_H_ */
