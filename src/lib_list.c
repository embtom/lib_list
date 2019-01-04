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

/* *******************************************************************
 * includes
 * ******************************************************************/

/* c -runtime */
#ifndef __KERNEL__
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stddef.h>
	#include <errno.h>

/* frame */
	#include <lib_convention__errno.h>
	#include <lib_convention__macro.h>
#else
	#include <linux/types.h>
	#include <linux/kernel.h>
#endif

/* project */
#include "lib_list.h"


/* *******************************************************************
 * includes
 * ******************************************************************/
#undef CONFIG_LIST__LOCK_TYPE_SUCCESS

#ifdef CONFIG_LIST__LOCK_TYPE_CAS
	#include "lock_cas.h"
	#define LIB_LIST_CRITICAL_SECTION__INIT(_param) 					spinlock__init(&_param);
	#define LIB_LIST_CRITICAL_SECTION__LOCK(_param, _context_id)		spinlock__lock(&_param);
	#define LIB_LIST_CRITICAL_SECTION__UNLOCK(_param, _context_id)		spinlock__unlock(&_param);
	#define CONFIG_LIST__LOCK_TYPE_SUCCESS
#endif

#ifdef CONFIG_LIST__LOCK_TYPE_MEM
	#include "lock_mem.h"
	#define LIB_LIST_CRITICAL_SECTION__INIT(_param) 					memlock__init(&_param);
	#define LIB_LIST_CRITICAL_SECTION__LOCK(_param, _context_id)		memlock__lock(&_param, _context_id);
	#define LIB_LIST_CRITICAL_SECTION__UNLOCK(_param, _context_id)		memlock__unlock(&_param, _context_id);
	#define CONFIG_LIST__LOCK_TYPE_SUCCESS
#endif

#ifdef CONFIG_LIST__LOCK_TYPE_MTX
	#include <lib_thread.h>
	#define LIB_LIST_CRITICAL_SECTION__INIT(_param) 					lib_thread__mutex_init(&_param);
	#define LIB_LIST_CRITICAL_SECTION__LOCK(_param, _context_id)		lib_thread__mutex_lock(_param);
	#define LIB_LIST_CRITICAL_SECTION__UNLOCK(_param, _context_id)		lib_thread__mutex_unlock(_param);
	#define CONFIG_LIST__LOCK_TYPE_SUCCESS
#endif

#ifdef CONFIG_LIST__LOCK_TYPE_FREERTOS
	#include "lock_freertos.h"
	#define LIB_LIST_CRITICAL_SECTION__INIT(_param)						freertos_lock__init(&_param);
	#define LIB_LIST_CRITICAL_SECTION__LOCK(_param, _context_id)		freertos_lock__lock(&_param);
	#define LIB_LIST_CRITICAL_SECTION__UNLOCK(_param, _context_id)		freertos_lock__unlock(&_param);
	#define CONFIG_LIST__LOCK_TYPE_SUCCESS
#endif

#ifndef CONFIG_LIST__LOCK_TYPE_SUCCESS
	#error "CONFIGURATION MISSING. YOU MUST DEFINE CONFIG_ICB_FIFO__LOCK_TYPE"
#endif


/* *******************************************************************
 * defines
 * ******************************************************************/
#define M_CMP_INITIALIZED			0xABBA8778

#define M_MEM_CALCULATED			0xAEEA4334
#define M_MEM_REGISTERED			0xBEEB3223

#define M_MEM_ENTRY_FREE				0
#define M_MEM_ENTRY_ID(_pos, _size)		((_size << 16) | _pos)
#define M_MEM_ENTRY_ID_TO_SIZE(_id)		(_id >> 16)
#define M_MEM_ENTRY_ID_TO_POS(_id)		(0xFFFF & _id)


/* *******************************************************************
 * Static Inline Functions - QUEUE ADDRESS TRANSLATION
 * ******************************************************************/
static inline void* addr_to_virt(void *_base, void *_phys)
{
	void *ptr;
	ptr = (void*)((char*)_phys - (uintptr_t)_base);
	return ptr;
}

static inline void* addr_to_phys(void *_base, void *_virt)
{
	void *ptr;
	ptr = (void*)((char*)_virt + (uintptr_t)_base);
	return ptr;
}

/* *******************************************************************
 * Static Inline Functions - QUEUE HANDLING
 * ******************************************************************/
static inline void node_add_prev(struct list_node *_new, struct list_node *_prev, struct list_node *_next, void *_base)
{
	struct list_node *phys_next, *virt_prev, *virt_new;

	phys_next = (struct list_node*)addr_to_phys(_base, _next);
	virt_prev = (struct list_node*)addr_to_virt(_base, _prev);
	virt_new = (struct list_node*)addr_to_virt(_base, _new);

	phys_next->prev = virt_new;
	_new->next = _next;
	_new->prev = virt_prev;
	_prev->next = virt_new;
}

static inline void node_add_next(struct list_node *_new, struct list_node *_prev, struct list_node *_next, void *_base)
{
	struct list_node *phys_next, *virt_prev, *virt_new;

	phys_next = (struct list_node*)addr_to_phys(_base, _next);
	virt_prev = (struct list_node*)addr_to_virt(_base, _prev);
	virt_new = (struct list_node*)addr_to_virt(_base, _new);

	phys_next->next = virt_new;
	_new->next = virt_prev;
	_new->prev = _next;
	_prev->prev = virt_new;
}

static inline void node_del(struct list_node *_prev, struct list_node *_next, void *_base)
{
	struct list_node *phys_next, *phys_prev;

	phys_next = (struct list_node*)addr_to_phys(_base, _next);
	phys_prev = (struct list_node*)addr_to_phys(_base, _prev);

	phys_next->prev = _prev;
	phys_prev->next = _next;
}

static inline int list_emty(struct list_node *_head, void *_base)
{
	struct list_node *virt_head;
	virt_head = (struct list_node*)addr_to_virt(_base, _head);
	return ((_head->next == virt_head) ? 1 : 0);
}

static inline void list_add_prev(struct list_node *_new, struct list_node *_head, void *_base)
{
	node_add_prev(_new, _head, _head->next, _base);
}

static inline void list_add_next(struct list_node *_new, struct list_node *_head, void *_base)
{
	node_add_next(_new, _head, _head->prev, _base);
}

static inline void list_del(struct list_node *_remove, void *_base)
{
	node_del(_remove->prev,_remove->next, _base);
}

static inline struct list_node* list_next(struct list_node *_node, void *_base)
{
	return (struct list_node*)addr_to_phys(_base, _node->prev);
}

static inline struct list_node* list_priv(struct list_node *_node, void *_base)
{
	return (struct list_node*)addr_to_phys(_base, _node->next);
}

static inline unsigned int list_equal(struct list_node *_node_one, struct list_node *_node_two)
{
	if (_node_one == _node_two)
		return 1;
	else
		return 0;
}

/* *******************************************************************
 * Static Inline Functions - MEM HANDLING
 * ******************************************************************/
static inline void mem_lock_type(uint32_t *_lock_table, uint32_t _lock_start, uint32_t _lock_count, uint32_t _lock_type) {
	unsigned int count;
	for (count = 0; count < _lock_count; count++) {
		_lock_table[_lock_start + count] = _lock_type;
	}
}

static inline unsigned int mem_check_free_block(uint32_t *_lock_table, uint32_t _check_start, uint32_t _check_count) {
	unsigned int iterator_check_count, ret = 0;
	for(iterator_check_count = 0; iterator_check_count < _check_count; iterator_check_count++) {
		ret += _lock_table[_check_start + iterator_check_count];
	}
	return (ret > 0) ? 1 : 0;
 }

static inline int mem_find_next_free_block(uint32_t *_lock_table, uint32_t _entry_count, uint32_t _get_pos, uint32_t _get_count) {
	unsigned int ret, iterator_entry_count;

	uint32_t temp_pos;

	for (iterator_entry_count = 0; iterator_entry_count < _entry_count; iterator_entry_count++) {

		temp_pos = (_get_pos + iterator_entry_count) % _entry_count;
		ret = mem_check_free_block(_lock_table, temp_pos, _get_count);
		if(ret == 0)
			return temp_pos;
	}
	return -LIB_LIST__ENOSPC;

}

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
int lib_list__init(struct queue_attr *_queue, void *_base)
{
	int ret;
	if (_queue == NULL) {
		return -LIB_LIST__EPAR_NULL;
	}

	ret = LIB_LIST_CRITICAL_SECTION__INIT(_queue->lock);
	if(ret < LIB_LIST__EOK) {
		return ret;
	}


	_queue->head.next = _queue->head.prev = (struct list_node*)addr_to_virt(_base, (void*)&_queue->head);
	_queue->initialized = M_CMP_INITIALIZED;
	return LIB_LIST__EOK;
}

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
int lib_list__enqueue(struct queue_attr *_queue, struct list_node * _new, uint32_t _context_id, void *_base)
{
	int ret;
	if(_queue == NULL) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	/* Lock critical section */
	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if(ret < LIB_LIST__EOK) {
		return ret;
	}

	/* Enqueue list element */
	list_add_prev(_new,&_queue->head,_base);
	/*return value check at the unlock functions is not necessary, because check is already passed at the lock functions */
	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	return LIB_LIST__EOK;
}

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
int lib_list__dequeue(struct queue_attr *_queue, struct list_node **_dequeue_node, uint32_t _context_id, void *_base)
{
	int ret;
	struct list_node *dequeue_node;

	if ((_queue == NULL) || (_dequeue_node == NULL)) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if (ret < LIB_LIST__EOK) {
		return ret;
	}

	/*return value check at the unlock functions is not necessary, because check is already passed at the lock functions */
	if(list_emty(&_queue->head,_base)) {
		LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
		return -LIB_LIST__ESTD_AGAIN;
	}

	dequeue_node = (struct list_node*)addr_to_phys(_base, _queue->head.prev);
	list_del(dequeue_node, _base);
	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	*_dequeue_node = dequeue_node;
	return LIB_LIST__EOK;
}

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
int lib_list__get_begin(struct queue_attr *_queue, struct list_node ** _begin_node, uint32_t _context_id, void *_base)
{
	int ret;

	if ((_queue == NULL) || (_begin_node == NULL)) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if (ret < LIB_LIST__EOK) {
		return ret;
	}

	/*return value check at the unlock functions is not necessary, because check is already passed at the lock functions */
	if(list_emty(&_queue->head,_base)) {
		LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
		return -LIB_LIST__ESTD_AGAIN;
	}

	*_begin_node = (struct list_node*)addr_to_phys(_base, _queue->head.prev);

	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	return LIB_LIST__EOK;
}

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
int lib_list__get_end(struct queue_attr *_queue, struct list_node ** _end_node, uint32_t _context_id, void *_base)
{
	int ret;

	if ((_queue == NULL) || (_end_node == NULL)) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if (ret < LIB_LIST__EOK) {
		return ret;
	}

	/*return value check at the unlock functions is not necessary, because check is already passed at the lock functions */
	if(list_emty(&_queue->head,_base)) {
		LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
		return -LIB_LIST__ESTD_AGAIN;
	}

	*_end_node = (struct list_node*)addr_to_phys(_base, _queue->head.next);
	//*_end_node = &_queue->head;

	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	return LIB_LIST__EOK;
}

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
int lib_list__get_next(struct queue_attr *_queue, struct list_node ** _next_node, uint32_t _context_id, void *_base)
{
	int ret;
	struct list_node *current_node, *next_node;

	if ((_queue == NULL) || (_next_node == NULL)) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if (ret < LIB_LIST__EOK) {
		return ret;
	}

	if(list_emty(&_queue->head,_base)) {
		LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
		return -LIB_LIST__ESTD_AGAIN;
	}

	current_node = *_next_node;
	next_node = list_next(current_node, _base);

	if(list_equal(&_queue->head, next_node)) {

		//*_next_node = next_node;
		*_next_node = list_next(next_node, _base);
		ret = -LIB_LIST__LIST_OVERFLOW;
	}
	else {
		*_next_node = next_node;
		ret = LIB_LIST__EOK;
	}

	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	return ret;
}

/* ************************************************************************//**
 * \brief	add a new element to a selected position
 *
 *  \param	*_queue [in]		fifo description attribute, to dequeue
 *	\param  *_pos_after_to_add[in]	pointer to list element after the new node is to attach
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
int lib_list__add_after(struct queue_attr *_queue, struct list_node *_pos_after_to_add, struct list_node *_to_add, uint32_t _context_id, void *_base)
{
	int ret;

	if ((_queue == NULL) || (_pos_after_to_add == NULL) || (_to_add == NULL)) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if (ret < LIB_LIST__EOK) {
		return ret;
	}

	list_add_next(_to_add,_pos_after_to_add, _base);

	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	return EOK;
}

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
int lib_list__add_before(struct queue_attr *_queue, struct list_node *_pos_before_to_add, struct list_node *_to_add, uint32_t _context_id, void *_base)
{
	int ret;

	if ((_queue == NULL) || (_pos_before_to_add == NULL) || (_to_add == NULL)) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if (ret < LIB_LIST__EOK) {
		return ret;
	}

	list_add_prev(_to_add,_pos_before_to_add, _base);

	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	return EOK;
}

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
int lib_list__delete(struct queue_attr *_queue, struct list_node * _del, uint32_t _context_id, void *_base)
{

	int ret;
	struct list_node *list_node;

	if ((_queue == NULL) || (_del == NULL)) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if (ret < LIB_LIST__EOK) {
		return ret;
	}

	if(list_emty(&_queue->head,_base)) {
		LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
		return -LIB_LIST__ESTD_AGAIN;
	}

	list_del(_del,_base);

	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	return LIB_LIST__EOK;
}

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
int lib_list__contains(struct queue_attr *_queue, struct list_node * _node, uint32_t _context_id, void *_base)
{
	int ret;
	struct list_node *itr_node, *start, *end;
	struct list_node *list_node;

	if ((_queue == NULL) || (_node == NULL)) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if (ret < LIB_LIST__EOK) {
		return ret;
	}

	list_node = (struct list_node*)addr_to_phys(_base, _queue->head.prev);
	while(!list_equal(&_queue->head, list_node)) {
		if(_node == list_node){
			LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
			return 1;
		}
		list_node = list_next(list_node, _base);
	}

	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	return 0;
}

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
int lib_list__emty(struct queue_attr *_queue, uint32_t _context_id, void *_base)
{
	int ret;

	if (_queue == NULL) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if (ret < LIB_LIST__EOK) {
		return ret;
	}

	ret = list_emty(&_queue->head,_base);

	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	return ret;
}

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
int lib_list__count(struct queue_attr *_queue, uint32_t _context_id, void *_base)
{
	int ret;
	struct list_node *end, *itr;
	unsigned int entryCount = 1;

	if (_queue == NULL) {
		return -LIB_LIST__EPAR_NULL;
	}

	if(_queue->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	ret = LIB_LIST_CRITICAL_SECTION__LOCK(_queue->lock,_context_id);
	if (ret < LIB_LIST__EOK) {
		return ret;
	}

	if(list_emty(&_queue->head,_base)) {
		LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
		return 0;
	}

	// Set itr to begin and request end
	itr = (struct list_node*)addr_to_phys(_base, _queue->head.prev);
	end = (struct list_node*)addr_to_phys(_base, _queue->head.next);
	while(!list_equal(itr, end)) {
		itr = list_next(itr, _base);
		entryCount++;
	}
	LIB_LIST_CRITICAL_SECTION__UNLOCK(_queue->lock,_context_id);
	return entryCount;
}


struct list_node* ITR_BEGIN(struct queue_attr *_queue, uint32_t _context_id, void *_base)
{
	int itr_begin_ret;
	struct list_node *itr_begin_node;
	itr_begin_ret = lib_list__get_begin(_queue, &itr_begin_node, _context_id, _base);
	if (itr_begin_ret != LIB_LIST__EOK)
		return NULL;
	else
		return itr_begin_node;
}

struct list_node* ITR_END(struct queue_attr *_queue, uint32_t _context_id, void *_base)
{
	int itr_end_ret;
	struct list_node *itr_end_node;
	itr_end_ret = lib_list__get_end(_queue, &itr_end_node, _context_id, _base);
	if (itr_end_ret != LIB_LIST__EOK)
		return NULL;
	else
		return itr_end_node;
}

void ITR_NEXT(struct queue_attr *_queue, struct list_node **_itr_node,  uint32_t _context_id, void *_base)
{
	int itr_next_ret;
	struct list_node *itr_next_node;
	itr_next_ret = lib_list__get_next(_queue, _itr_node, _context_id, _base);
	if ((itr_next_ret != LIB_LIST__EOK) && (itr_next_ret != -LIB_LIST__LIST_OVERFLOW))
		*_itr_node = NULL;
}



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
int lib_list__mem_calc_size(mem_hdl_t * const _hdl, size_t _entry_size, unsigned int _entry_count)
{
	size_t align_entry_size;
	size_t buffer_size;
	if (_hdl == NULL)
		return -LIB_LIST__EPAR_NULL;

	/* Reserve of memory for buffer management */
	buffer_size = sizeof(struct mem_info_attr);

	/*Calculation of the aligned size of a entry */
	align_entry_size = ALIGN(_entry_size, sizeof(uint32_t));

	buffer_size += (align_entry_size + sizeof (uint32_t)) * _entry_count;

	/* Init handle structure*/
	_hdl->entry_count = _entry_count;
	_hdl->entry_size = ALIGN(_entry_size, sizeof(uint32_t));
	_hdl->init_state = M_MEM_CALCULATED;
	return buffer_size;
}

/* ************************************************************************//**
 * \brief	Setup of the memory handling
 *
 *	The memory with size, calculated with the assistance of "lib_icb_fifo__mem_calc_size"
 *	routine will be initialized.
 *
 *  \param	*_hdl [out]		 memory description handle
 *  \param	_mode			 sets the master or slave mode of the memory queue, master creates queue and slave attach on a queue
 *  \param  *_mem_base [IN]  		 memory to setup
 *  \param	_mem_size		 size of the memory to setup
 *
 *	\return EOK if successful, or negative errno value on error
 * 			-LIB_LIST__EPAR_NULL	: NULL pointer check
 * 			-LIB_LIST__EEXEC_NOINIT : Memory size to expect was not calculated by "lib_icb_fifo__mem_calc_size"
 * 			-LIB_LIST__ESTD_INVAL	: Passed mem_size is 0 or not aligned to sizeof(uint32_t)
 * 			-LIB_LIST__EPAR_RANGE   : Expected memory size does not fits with the passed memory size
 *
 * ****************************************************************************/
int lib_list__mem_setup(mem_hdl_t * const _hdl, enum mem_setup_mode _mode, const void *_mem_base, size_t _mem_size)
{
	int ret;
	struct mem_info_attr *info;
	size_t expected_mem_size = M_MEM_SIZE_1__MEM_INFO_ATTR;

	if ((_mem_base == NULL)||(_hdl == NULL))
		return -LIB_LIST__EPAR_NULL;

	if (_hdl->init_state != M_MEM_CALCULATED)
		return -LIB_LIST__EEXEC_NOINIT;

	/*if passed mem size is aligned to sizeof uint32_t */
	if ((_mem_size == 0) || (_mem_size != ALIGN(_mem_size, sizeof(uint32_t))))
		return -LIB_LIST__ESTD_INVAL;

	/* Check if expected memory size fits into the passed memory */
	expected_mem_size += M_MEM_SIZE_2__ENTRY_LOCK(_hdl->entry_count) + M_MEM_SIZE_3__ENTRY_DATA(_hdl->entry_count,_hdl->entry_size);
	if (_mem_size != expected_mem_size)
		return -LIB_LIST__EPAR_RANGE;

	_hdl->mem_base = _mem_base;
	_hdl->mem_size = _mem_size;

	info = (struct mem_info_attr*)_mem_base;

	switch (_mode)
	{
		case MEM_SETUP_MODE_master:
		{
			memset((void*)_mem_base, 0, _mem_size);

			info->entry_size  = _hdl->entry_size;
			info->entry_count = _hdl->entry_count;

			ret = LIB_LIST_CRITICAL_SECTION__INIT(info->lock);
			if(ret < LIB_LIST__EOK) {
				return ret;
			}
			info->initialized = M_CMP_INITIALIZED;
		}
		break;

		case MEM_SETUP_MODE_slave:
		{
			if (info->initialized != M_CMP_INITIALIZED)
				return -LIB_LIST__ESTD_ACCES;

			if (info->entry_count != _hdl->entry_count)
				return -LIB_LIST__ESTD_ACCES;

			if (info->entry_size != _hdl->entry_size)
				return -LIB_LIST__ESTD_ACCES;
		}
		break;

		default:
			return -LIB_LIST__ESTD_INVAL;
	}

	_hdl->entry_lock_table = (uint32_t*)((uint8_t*)_hdl->mem_base + M_MEM_SIZE_1__MEM_INFO_ATTR);
	_hdl->entry_data = (uint32_t*)((uint8_t*)_hdl->mem_base + M_MEM_SIZE_1__MEM_INFO_ATTR + M_MEM_SIZE_2__ENTRY_LOCK(info->entry_count));

	_hdl->init_state = M_MEM_REGISTERED;
	return LIB_LIST__EOK;
}

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
int lib_list__mem_cleanup(mem_hdl_t * const _hdl, enum mem_setup_mode _mode, void **_ptr_mem_base, size_t *_ptr_mem_size)
{
	struct mem_info_attr *info;
	size_t calc_mem_size;

	if (_hdl == NULL)
		return -LIB_LIST__EPAR_NULL;

	if (_hdl->init_state != M_MEM_REGISTERED)
		return -LIB_LIST__EEXEC_NOINIT;

	info = (struct mem_info_attr*)_hdl->mem_base;
	if (info->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	calc_mem_size = M_MEM_SIZE_1__MEM_INFO_ATTR + M_MEM_SIZE_2__ENTRY_LOCK(info->entry_count) +
					M_MEM_SIZE_3__ENTRY_DATA(info->entry_count ,info->entry_size);

	if(calc_mem_size != _hdl->mem_size) {
		return -LIB_LIST__ESTD_FAULT;
	}

	switch (_mode)
	{
		case MEM_SETUP_MODE_master:
		{
			info->initialized = 0;
			_hdl->init_state = 0;
			if (_ptr_mem_base != NULL) {*_ptr_mem_base = (void*)_hdl->mem_base;}
			if (_ptr_mem_size != NULL) {*_ptr_mem_size = _hdl->mem_size;}
		}
		break;

		case MEM_SETUP_MODE_slave:
		{
			if (_ptr_mem_base != NULL) {*_ptr_mem_base = (void*)_hdl->mem_base;}
			if (_ptr_mem_size != NULL) {*_ptr_mem_size = _hdl->mem_size;}
			_hdl->init_state = 0;
		}
		break;

		default:
			return -LIB_LIST__ESTD_INVAL;

	}

	return LIB_LIST__EOK;
}

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
void* lib_list__mem_alloc(mem_hdl_t * const _hdl, unsigned int _req_entry_count, unsigned int _context_id, int* _ret)
{
	int ret;
	uint32_t pos;
	uint32_t *entry_lock_table;
	uint32_t *entry_data, *requested_memory;
	struct mem_info_attr *info;

	if (_hdl == NULL) {
		if (_ret != NULL) { *_ret = -LIB_LIST__EPAR_NULL; }
		return NULL;
	}

	if (_hdl->init_state != M_MEM_REGISTERED) {
		if (_ret != NULL) { *_ret = -LIB_LIST__EEXEC_NOINIT; }
		return NULL;
	}

	info = (struct mem_info_attr*)_hdl->mem_base;
	if (info->initialized != M_CMP_INITIALIZED) {
		if (_ret != NULL) { *_ret = -LIB_LIST__EEXEC_NOINIT; }
		return NULL;
	}

	//entry_lock_table = (uint32_t*)_hdl->mem_base + (M_MEM_SIZE_1__MEM_INFO_ATTR / sizeof(uint32_t));
	//entry_data = (uint32_t*)_hdl->mem_base + ((M_MEM_SIZE_1__MEM_INFO_ATTR + M_MEM_SIZE_2__ENTRY_LOCK(info->entry_count))/ sizeof(uint32_t));

	//if ((entry_lock_table != _hdl->entry_lock_table) || (entry_data != _hdl->entry_data)) {
	//	if (_ret != NULL) { *_ret = -LIB_LIST__ESTD_FAULT; }
	//	return NULL;
	//}
	entry_lock_table = _hdl->entry_lock_table;
	entry_data = _hdl->entry_data;



	/* Check if max possible memory size is exceeded */
	if (_req_entry_count > info->entry_count){
		if (_ret != NULL) {	*_ret = -LIB_LIST__ENOSPC; }
		return NULL;
	}

	//////////////////////////////////////
	/* BEGIN - critical section */
	ret = LIB_LIST_CRITICAL_SECTION__LOCK(info->lock,_context_id);
	if(ret < LIB_LIST__EOK) {
		if (_ret != NULL) {	*_ret = ret; }
		return NULL;
	}

	/*Check if enough remaining space is available and if not start to check at the beginning of the buffer */
	pos = info->get_pos;
	if ((pos + _req_entry_count) > info->entry_count) {
		pos = 0;
	}

	/* Search and get next free entries block */
	ret = mem_find_next_free_block(entry_lock_table, info->entry_count, pos, _req_entry_count);
	if(ret < 0) {
		if (_ret != NULL) {	*_ret = ret; }
		LIB_LIST_CRITICAL_SECTION__UNLOCK(info->lock,_context_id);
		return NULL;
	}
	pos = (uint32_t)ret;

	mem_lock_type(entry_lock_table, pos, _req_entry_count, M_MEM_ENTRY_ID(pos,_req_entry_count));
	info->get_pos = pos + _req_entry_count;

	//////////////////////////////////////
	/* END - critical section  (retval check not necessary, already done at lock function) */
	LIB_LIST_CRITICAL_SECTION__UNLOCK(info->lock,_context_id);

	/*pass locked memory to the caller */
	requested_memory = entry_data + (M_MEM_SIZE_3__ENTRY_DATA(pos ,info->entry_size)/sizeof(uint32_t));

	return (void*)requested_memory;
}

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
int lib_list__mem_free(mem_hdl_t * const _hdl, void *_ptr, unsigned int _context_id)
{
	int ret;
	uint32_t pos;
	uint32_t entry_id, entry_id_size, entry_id_pos;
	uint32_t *entry_lock_table;
	uint32_t *entry_data;
	uint8_t *mem_end;
	struct mem_info_attr *info;

	if ((_hdl == NULL) || (_ptr == NULL)) {
		return -LIB_LIST__EPAR_NULL;
	}

	if (_hdl->init_state != M_MEM_REGISTERED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	info = (struct mem_info_attr*)_hdl->mem_base;
	if (info->initialized != M_CMP_INITIALIZED) {
		return -LIB_LIST__EEXEC_NOINIT;
	}

	entry_lock_table = _hdl->entry_lock_table;
	entry_data = _hdl->entry_data;
	mem_end = (uint8_t*)_hdl->mem_base + _hdl->mem_size;

	/*Check if pointer to free is part of this buffer group*/
	if(((uint8_t*)entry_data > (uint8_t*)_ptr) || ((uint8_t*)_ptr >= mem_end)) {
		return -LIB_LIST__ESTD_INVAL;
	}

	/*Check if pointer is valid to unlock */
	pos = ((uint8_t*)_ptr - (uint8_t*)entry_data) / info->entry_size;

	/* Lock critical section */
	ret = LIB_LIST_CRITICAL_SECTION__LOCK(info->lock,_context_id);
	if(ret < LIB_LIST__EOK) {
		return ret;
	}

	entry_id = entry_lock_table[pos];
	entry_id_pos = M_MEM_ENTRY_ID_TO_POS(entry_id);
	entry_id_size = M_MEM_ENTRY_ID_TO_SIZE(entry_id);
	if(pos != entry_id_pos)
	{
		LIB_LIST_CRITICAL_SECTION__UNLOCK(info->lock,_context_id);
		return -LIB_LIST__ESTD_INVAL;
	}

	if(entry_lock_table[pos+entry_id_size-1] != entry_id)
	{
		LIB_LIST_CRITICAL_SECTION__UNLOCK(info->lock,_context_id);
		return -LIB_LIST__ESTD_INVAL;
	}


	mem_lock_type(entry_lock_table, pos, entry_id_size, 0);
	memset((uint32_t*)_ptr,0,entry_id_size);

	LIB_LIST_CRITICAL_SECTION__UNLOCK(info->lock,_context_id);

	return LIB_LIST__EOK;
}

