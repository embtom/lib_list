/* ****************************************************************************************************
 * \file	lock_mem_types.h
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

#ifndef _LOCK_MEM_TYPES_H_
#define _LOCK_MEM_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* *******************************************************************
 * defines
 * ******************************************************************/
#ifndef M_DEV_NUMBER_OF_LOCK_CONTEXT
	#define M_DEV_NUMBER_OF_LOCK_CONTEXT		2
#endif

/* *******************************************************************
 * custom data types (e.g. enumerations, structures, unions)
 * ******************************************************************/
struct semilock {
	volatile uint32_t victim[M_DEV_NUMBER_OF_LOCK_CONTEXT];
	volatile uint32_t interested_context[M_DEV_NUMBER_OF_LOCK_CONTEXT];
};

typedef struct semilock semilock_t;


#ifdef __cplusplus
}
#endif

#endif /* _LOCK_MEM_TYPES_H_ */
