/* ****************************************************************************************************
 * \file	lock_mtx_types.h
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

#ifndef	_LOCK_MTX_TYPES_H_
#define _LOCK_MTX_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <lib_thread.h>


/* *******************************************************************
 * custom data types (e.g. enumerations, structures, unions)
 * ******************************************************************/
typedef mutex_hdl_t semilock_t;

#ifdef __cplusplus
}
#endif

#endif /* _LOCK_MTX_TYPES_H_ */
