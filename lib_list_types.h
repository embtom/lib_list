/* ****************************************************************************************************
 * \file	lib_list_types.h
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

#ifndef _SH_LIB_ICB_FIFO_LIB_ICB_FIFO_TYPES_H_
#define _SH_LIB_ICB_FIFO_LIB_ICB_FIFO_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif


/* *******************************************************************
 * includes
 * ******************************************************************/

/* c-runtime */
#ifndef __KERNEL__
	#include <stdint.h>
	#include <stddef.h>
#endif

/* system */

/* frame */

/* project */

#ifdef CONFIG_LIST__LOCK_TYPE_CAS
	#include "lock_cas_types.h"
#endif

#ifdef CONFIG_LIST__LOCK_TYPE_MEM
	#include "lock_mem_types.h"
#endif

#ifdef CONFIG_LIST__LOCK_TYPE_MTX
	#include "lock_mtx_types.h"
#endif

#ifdef CONFIG_LIST__LOCK_TYPE_FREERTOS
	#include "lock_freertos_types.h"
#endif

/* *******************************************************************
 * defines
 * ******************************************************************/

#ifndef __KERNEL__
	#define LIB_LIST__EOK				EOK
	#define LIB_LIST__ESTD_AGAIN		ESTD_AGAIN
	#define LIB_LIST__ESTD_FAULT		ESTD_FAULT
	#define LIB_LIST__ESTD_INVAL		ESTD_INVAL
	#define LIB_LIST__EPAR_NULL			EPAR_NULL
	#define LIB_LIST__EEXEC_NOINIT		EEXEC_NOINIT
	#define LIB_LIST__ENOSPC			ESTD_NOSPC
	#define LIB_LIST__EPAR_RANGE		EPAR_RANGE
	#define LIB_LIST__ESTD_ACCES		ESTD_ACCES
	#define LIB_LIST__LIST_OVERFLOW		ELIST_OVERFLOW
#else
	#define LIB_LIST__EOK				0
	#define LIB_LIST__ESTD_AGAIN		EAGAIN
	#define LIB_LIST__ESTD_FAULT		EFAULT
	#define LIB_LIST__ESTD_INVAL		EINVAL
	#define LIB_LIST__EPAR_NULL			EINVAL
	#define LIB_LIST__EEXEC_NOINIT		ESRCH
	#define LIB_LIST__ENOSPC			EINVAL
	#define LIB_LIST__EPAR_RANGE 		ENFILE
	#define LIB_LIST__ESTD_ACCES		EACCES
	#define LIB_LIST__LIST_OVERFLOW 	EOVERFLOW
#endif


#define M_MEM_SIZE_1__MEM_INFO_ATTR								(sizeof(struct mem_info_attr))
#define M_MEM_SIZE_2__ENTRY_LOCK(_entry_count)					(sizeof(uint32_t) * _entry_count)
#define M_MEM_SIZE_3__ENTRY_DATA(_entry_count, _entry_size)		(_entry_count * _entry_size)

/* *******************************************************************
 * custom data types (e.g. enumerations, structures, unions)
 * ******************************************************************/
enum mem_setup_mode {
	MEM_SETUP_MODE_master,
	MEM_SETUP_MODE_slave
};

struct list_node {
	struct list_node *next, *prev;
};

struct queue_attr {
	struct list_node head;
	semilock_t lock;
	unsigned int initialized;
};

struct mem_attr {
	uint32_t entry_size;
	uint32_t entry_count;
	const void *mem_base;
	size_t 	mem_size;
	uint32_t *entry_lock_table;
	uint32_t *entry_data;
	unsigned int init_state;
};


struct mem_info_attr {
	uint32_t entry_size;
	uint32_t entry_count;
	semilock_t lock;
	uint32_t get_pos;
	uint32_t initialized;
};



typedef struct mem_attr mem_hdl_t;

typedef struct mem_info_attr *queue_mem_hdl_t;


#ifdef __cplusplus
}
#endif

#endif /* _SH_LIB_ICB_FIFO_LIB_ICB_FIFO_TYPES_H_ */
