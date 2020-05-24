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

#ifndef _LIB_LIST_H_
#define _LIB_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/* *******************************************************************
 * includes
 * ******************************************************************/

/* project */
#include "lib_list_types.h"


/* *******************************************************************
 * custom data types (e.g. enumerations, structures, unions)
 * ******************************************************************/


/* *******************************************************************
 * Global Functions - QUEUE HANDLING
 * ******************************************************************/

/* ************************************************************************//**
 * \brief	Initialization of the icb_fifo list
 *
 * \param	*_queue [in]		Initialization of the fifo description attribute
 *
 * \return 	EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		NULL pointer check
 * ****************************************************************************/
int lib_list__init(struct queue_attr *_queue, void *_base);

/* ************************************************************************//**
 * \brief	Enqueue of a list element
 *
 * \param	*_queue [in]		fifo description attribute, to enqueue
 * \param	*_new [in]			new entry for the list
 * \param	_context_id			Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *
 * \return 	EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * ****************************************************************************/
int lib_list__enqueue(struct queue_attr *_queue, struct list_node * _new, uint32_t _context_id, void *_base);

/* ************************************************************************//**
 * \brief	Dequeue of a list element
 *
 *  \param	*_queue [in]		 fifo description attribute, to dequeue
 *	\param  **_dequeue_node[out] pointer to dequeue a list element
 *  \param	_context_id			 Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * 			-ESTD_AGAIN		: fifo is empty
 *
 * ****************************************************************************/
int lib_list__dequeue(struct queue_attr *_queue, struct list_node **_dequeue_node, uint32_t _context_id, void *_base);

/* ************************************************************************//**
 * \brief	Get first node
 *
 *  \param	*_queue [in]		 fifo description attribute, to dequeue
 *	\param  **_first_node[out]   pointer to first list element
 *  \param	_context_id			 Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * 			-ESTD_AGAIN		: fifo is empty
 *
 * ****************************************************************************/
int lib_list__get_begin(struct queue_attr *_queue, struct list_node ** _begin_node, uint32_t _context_id, void *_base);

/* ************************************************************************//**
 * \brief	Get last node
 *
 *  \param	*_queue [in]		 fifo description attribute, to dequeue
 *	\param  **_first_node[out]   pointer to first list element
 *  \param	_context_id			 Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * 			-ESTD_AGAIN		: fifo is empty
 *
 * ****************************************************************************/
int lib_list__get_end(struct queue_attr *_queue, struct list_node ** _end_node, uint32_t _context_id, void *_base);

/* ************************************************************************//**
 * \brief	Get next node
 *
 *  \param	*_queue [in]		 fifo description attribute, to dequeue
 *	\param  **_first_node[out]   pointer to next list element
 *  \param	_context_id			 Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * 			-ESTD_AGAIN		: fifo is empty
 *
 * ****************************************************************************/
int lib_list__get_next(struct queue_attr *_queue, struct list_node ** _next_node, uint32_t _context_id, void *_base);

/* ************************************************************************//**
 * \brief	add a new element after the selected position
 *
 *  \param	*_queue [in]		fifo description attribute, to dequeue
 *	\param  *_pos_to_add[in]	pointer to list element after the new node is to attach
 *	\param  *_to_add[in]		pointer to list element_to_add
 *  \param	_context_id[in]		Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *  \param	*_base[in]			base mem address
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * 			-ESTD_AGAIN		: fifo is empty
 *
 * ****************************************************************************/
int lib_list__add_after(struct queue_attr *_queue, struct list_node *_pos_after_to_add, struct list_node *_to_add, uint32_t _context_id, void *_base);

/* ************************************************************************//**
 * \brief	add a new element to a selected position
 *
 *  \param	*_queue [in]		fifo description attribute, to dequeue
 *	\param  *_pos_before_to_add[in]	pointer to list element after the new node is to attach
 *	\param  *_to_add[in]		pointer to list element_to_add
 *  \param	_context_id[in]		Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *  \param	*_base[in]			base mem address
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * 			-ESTD_AGAIN		: fifo is empty
 *
 * ****************************************************************************/
int lib_list__add_before(struct queue_attr *_queue, struct list_node *_pos_before_to_add, struct list_node *_to_add, uint32_t _context_id, void *_base);

/* ************************************************************************//**
 * \brief	Get delete node
 *
 *  \param	*_queue [in]		 fifo description attribute, to dequeue
 *	\param  **_first_node[out]   pointer to next list element
 *  \param	_context_id			 Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *  \param	*_base[in]			base mem address
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * 			-ESTD_AGAIN		: fifo is empty
 *
 * ****************************************************************************/
int lib_list__delete(struct queue_attr *_queue, struct list_node * _del, uint32_t _context_id, void *_base);

/* ************************************************************************//**
 * \brief	Check if node is at list
 *
 *  \param	*_queue [in]		 fifo description attribute, to dequeue
 *	\param  **_first_node[out]   pointer to next list element
 *  \param	_context_id			 Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *  \param	*_base[in]			 base mem address
 *
 *	\return  return "1" node is in list or "0" if not, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * 			-ESTD_AGAIN		: fifo is empty
 *
 * ****************************************************************************/
int lib_list__contains(struct queue_attr *_queue, struct list_node * _node, uint32_t _context_id, void *_base);

/* ************************************************************************//**
 * \brief	Check if list is emty
 *
 *  \param	*_queue [in]		 fifo description attribute, to dequeue
 *  \param	_context_id			 Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *  \param	*_base[in]			 base mem address
 * 
 *	\return EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * 			-ESTD_AGAIN		: fifo is empty
 *
 * ****************************************************************************/
int lib_list__emty(struct queue_attr *_queue, uint32_t _context_id, void *_base);

/* ************************************************************************//**
 * \brief	Request number of queue entries
 *
 *  \param	*_queue [in]		 fifo description attribute, to dequeue
 *  \param	_context_id			 Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *  \param	*_base[in]			 base mem address
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-EPAR_NULL		: NULL pointer check
 * 			-EEXEC_NOINIT   : Queue is not yet initialized
 * 			-ESTD_FAULT		: The passed contexed_id exceeds the number of defined
 * 							  contexts (M_DEV_NUMBER_OF_LOCK_CONTEXT)
 * 			-ESTD_AGAIN		: fifo is empty
 *
 * ****************************************************************************/
int lib_list__count(struct queue_attr *_queue, uint32_t _context_id, void *_base);


struct list_node* ITR_BEGIN(struct queue_attr *_queue, uint32_t _context_id, void *_base);
struct list_node* ITR_END(struct queue_attr *_queue, uint32_t _context_id, void *_base);
void ITR_NEXT(struct queue_attr *_queue, struct list_node **_itr_node,  uint32_t _context_id, void *_base);

/* *******************************************************************
 * Global Functions - MEMORY HANDLING
 * ******************************************************************/

/* ************************************************************************//**
 * \brief	Calculation of the required buffer size
 *
 *  The required memory size calculated to handle a defined number of "entry_count"
 *  with a defines number of "_entry_size". The memory is aligned to a multiple
 *  of sizeof(uint32_t)
 *
 *
 *		Memory_table: 	    --------------------  0x0
 *							| MEM_GLOBAL 	   |
 *							--------------------  sizeof (struct mem_info_attr)
 *							| ENTRY_LOCK_TABLE |
 *							--------------------  sizeof(struct mem_info_attr) + _entry_count * sizeof (uint32_t)
 *							| MEMORY TO PROVIDE|
 *							--------------------  mem_size
 *
 *  \param	*_hdl [out]		 memory description handle
 *	\param  _entry_size		 size of the element to manage
 *  \param	_entry_count	 number of elements to manage
 *
 *	\return "buffer_size" if successful, or negative errno value on error
 * 			-LIB_LIST__EPAR_NULL	: NULL pointer check
 *
 * ****************************************************************************/
int lib_list__mem_calc_size(mem_hdl_t * const _hdl, size_t _entry_size, unsigned int _entry_count);

/* ************************************************************************//**
 * \brief	Setup of the memory handling
 *
 *	The memory with size, calculated with the assistance of "lib_icb_fifo__mem_calc_size"
 *	routine will be initialized.
 *
 *  \param	*_hdl [out]		 memory description handle
 *	\param  *_mem_base [IN]  memory to setup
 *  \param	_mem_size		 size of the memory to setup
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-LIB_LIST__EPAR_NULL	: NULL pointer check
 * 			-LIB_LIST__EEXEC_NOINIT : Memory size to expect was not calculated by "lib_icb_fifo__mem_calc_size"
 * 			-LIB_LIST__ESTD_INVAL	: Passed mem_size is 0 or not aligned to sizeof(uint32_t)
 * 			-LIB_LIST__EPAR_RANGE   : Expected memory size does not fits with the passed memory size
 *
 * ****************************************************************************/
int lib_list__mem_setup(mem_hdl_t * const _hdl, enum mem_setup_mode _mode, const void *_mem_base, size_t _mem_size);

/* ************************************************************************//**
 * \brief	Cleanup of the memory handling
 *
 *
 *  \param	*_hdl [out]		 memory description handle
 *  \param	_mode			 sets the cleanup, if master queue will deleted or slave detach on it
 *  \param  **_ptr_mem_base  pointer to pointer to pass the memory base address to the caller
 *  \param  **_ptr_mem_size  pointer to pointer to pass the memory size to the caller
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-LIB_LIST__EPAR_NULL	: NULL pointer check
 * 			-LIB_LIST__EEXEC_NOINIT : Memory size to expect was not calculated by "lib_icb_fifo__mem_calc_size"
 * 			-LIB_LIST__ESTD_FAULT	: Error occurred, calculated size not fits with size stored at the handle
 * 			-LIB_LIST__ESTD_INVAL   : Invalid parameter
 * ****************************************************************************/
int lib_list__mem_cleanup(mem_hdl_t * const _hdl, enum mem_setup_mode _mode, void **_ptr_mem_base, size_t *_ptr_mem_size);


/* ************************************************************************//**
 * \brief	Allocation of a specific number of memory entries
 *
 *  From the managed buffer a specific number entries with the adjusted size of
 *  _entry_size (passed at "lib_icb_fifo__mem_calc_size") can to be requested
 *
 *  \param	*_hdl [out]		 	Memory description handle
 *	\param  _req_entry_count 	Number of memory nodes to request
 *  \param	_context_id			Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *  \parm   *_ret [out]			The allocation routine passes NULL on error and the error cause is passed by call of pointer
 *	\return POINTER TO MEMORY if successful, or negative NULL value on error
 *		Value of *_ret is
 * 			-LIB_LIST__EPAR_NULL	: NULL pointer check
 * 			-LIB_LIST__EEXEC_NOINIT : Memory size to expect was not initialized by "lib_icb_fifo__mem_setup"
 * 			-LIB_LIST__ENOSPC		: Request size not fits
 *
 * ****************************************************************************/
void* lib_list__mem_alloc(mem_hdl_t * const _hdl, unsigned int _req_entry_count, unsigned int _context_id, int* _ret);

/* ************************************************************************//**
 * \brief	Free of the memory
 *
 *  \param	*_hdl [out]		 	Memory description handle
 *  \param 	*_ptr [in]
 *  \param	_contxt_id			Sequential number of contexts defined up to M_DEV_NUMBER_OF_LOCK_CONTEXT
 *
 *	\return EOK if successful, or negative errno value on error
 *			-LIB_LIST__EPAR_NULL	: NULL pointer check
 * 			-LIB_LIST__EEXEC_NOINIT : Memory size to expect was not initialized by "lib_queue__mem_setup"
 * 			-LIB_LIST__ESTD_INVAL	: Invalid pointer to free
 *
 * ****************************************************************************/
int lib_list__mem_free(mem_hdl_t * const _hdl, void *_ptr, unsigned int _context_id);

#ifdef __cplusplus
}
#endif

#endif /* _LIB_LIST_H_ */
