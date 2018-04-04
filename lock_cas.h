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

#ifndef SH_LOCK_CAS_H_
#define SH_LOCK_CAS_H_

/* *******************************************************************
 * includes
 * ******************************************************************/

/* system */
#include <pthread.h>

/* own libs */
#include <lib_convention__errno.h>

/* project */
#include "lock_cas_types.h"

/* *******************************************************************
 * Static Inline Functions
 * ******************************************************************/

/* ************************************************************************//**
 * \brief	Initialization of a semilock object - Based on CAS
 *
 * A spinlock is a lock which causes a context trying to enter a critical section
 * to wait in a loop ("spin") while repeatedly checking if the lock is available.
 *
 * For the check if a context is already entered atomic "test_and_set" operations
 * are necessary
 *
 * \param	*_lock [out]		The semilock_t data type will be initialized
 * \return 	EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * ****************************************************************************/
static inline int spinlock__init(semilock_t *_lock)
{
	if(_lock == NULL) {
		return -LIB_LIST__EPAR_NULL;
	}

	/*GCC Memory barrier prevent instruction reordering */
	__asm__ __volatile__ ("" ::: "memory");
    *_lock = 0;
    return LIB_LIST__EOK;
}

/* ************************************************************************//**
 * \brief	Lock of the semilock object - Based on CAS
 *
 * \param	*_lock [in]	 	The semilock_t data type to check if section
 * 							is already entered
 * \return 	EOK
 * ****************************************************************************/
static inline int spinlock__lock(semilock_t *_lock)
{
    while (1) {
        int i;
        for (i=0; i < 10000; i++) {
            if (__sync_bool_compare_and_swap(_lock, 0, 1)) {
            	return LIB_LIST__EOK;
            }
        }
        sched_yield();
    }
}

/* ************************************************************************//**
 * \brief	Unlock of the semilock object - Based on CAS
 *
 * \param	*_lock [in]	 	The semilock_t data type to signalizes that a section
 * 							is left
 * \return 	EOK
 * ****************************************************************************/
static inline int spinlock__unlock(semilock_t *_lock) {
    __asm__ __volatile__ ("" ::: "memory");
    *_lock = 0;
    return EOK;
}

/* ************************************************************************//**
 * \brief	Trylock of the semilock object - Based on CAS
 *
 * \param	*_lock [in]	 	The semilock_t data type to check if section
 * 							is already entered
 *
 * \return 	EOK if successful, or negative errno value on error
 * 			-ESTD_BUSY		Critical section is already entered by other context
 * ****************************************************************************/
static inline int spinlock__trylock(semilock_t *_lock) {
    if (__sync_bool_compare_and_swap(_lock, 0, 1)) {
        return EOK;
    }
    return -ESTD_BUSY;
}
#endif /* SH_LOCK_CAS_H_ */
