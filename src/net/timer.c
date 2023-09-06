/*
 * Copyright (c) 2022-2023 Li Xilin <lixilin@gmx.com>
 * 
 * Permission is hereby granted, free of charge, to one person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "timer.h"
#include "reactor_type.h"
#include "ax/reactor.h"
#include "ax/event.h"
#include "ax/socket.h"
#include "ax/timeval.h"
#include "ax/log.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

struct heap_entry
{
        /* The expiration of the event */
        struct timeval expiration;
        ax_event *e;
};

/*
* Heapify the timerheap in a top-down way after
* alteration of the heap with the @idxth entry being the top entry.
* This function is used when deleting a entry from the heap.
* @pti: The related timerheap_st structure.
* @idx: The top entry's index.
*/
static void timerheap_heapify_topdown(struct timerheap_st *pti, int idx);

struct timerheap_st *timerheap_init()
{
	struct timerheap_st *ret;
	struct heap_entry *heap;
	if ((ret = malloc(sizeof(struct timerheap_st))) == NULL) {
		ax_perror("failed to malloc for timerheap_st: %s", strerror(errno));
		return NULL;
	}
	memset(ret, 0, sizeof(struct timerheap_st));
	
	if ((heap = malloc(TIMERHEAP_INIT_SIZE *sizeof(struct heap_entry))) == NULL) {
		ax_perror("failed to malloc for heap_entry: %s", strerror(errno));
		free(ret);
		return NULL;
	}
	ret->heap = heap;
	ret->size = 0;
	ret->capacity = TIMERHEAP_INIT_SIZE;
	return ret;
}

/*
* Expand the timerheap to be big enough to hold @size entries.
*/
static int timerheap_grow(struct timerheap_st *pti, int size)
{
	int new_cap;
	struct heap_entry *new_heap;
	assert(pti != NULL);
	assert(size > 1);

	new_cap = pti->capacity;

	while(new_cap <= size) {
		new_cap <<= 1;
	}

	if ((new_heap = realloc(pti->heap, sizeof(struct heap_entry) *new_cap)) == NULL) {
		ax_perror("failed on realloc: %s", strerror(errno));
		return (-1);
	}

	pti->heap = new_heap;
	pti->capacity = new_cap;
	ax_pinfo("expanded to %d.", pti->capacity);
	return (0);
}
/*
* Initialize the heap entry with timer event.
*/
static void init_heap_entry(struct heap_entry *phe, ax_event *e)
{
	assert(phe != NULL);
	assert(e != NULL);

	ax_timeval_timeofday(&phe->expiration);
	ax_timeval_add_millise(phe->expiration, e->fd);
	phe->e = e;
}

/*
* Tests whether the heap is empty.
*/
static inline int timerheap_empty(struct timerheap_st *pti)
{
	assert(pti != NULL);
	return pti->size == 0;
}

int timerheap_top_expired(ax_reactor *r)
{
	struct timeval cur;

	assert(r != NULL);
	assert(r->pti != NULL);
	if (timerheap_empty(r->pti)) {
		return (0);
	}

	ax_timeval_timeofday(&cur);
	return ax_timeval_le(r->pti->heap[1].expiration, cur);
}

struct timeval *timerheap_top_timeout(ax_reactor *r, struct timeval *timeout)
{
	struct timeval cur;

	assert(r != NULL);
	assert(r->pti != NULL);
	if (timerheap_empty(r->pti)) {
		return NULL;
	}
	*timeout = r->pti->heap[1].expiration;
	ax_timeval_timeofday(&cur);
	ax_timeval_sub(timeout, &cur, timeout);
	if (ax_timeval_to_millise(*timeout) < 0) {
		timeout->tv_sec = timeout->tv_usec = 0;	
	}
	return timeout;
}

ax_event *timerheap_get_top(ax_reactor *r)
{
	assert(r != NULL);
	assert(r->pti != NULL);

	if (timerheap_empty(r->pti)) {
		return NULL;
	}

	return r->pti->heap[1].e;
}

ax_event *timerheap_pop_top(ax_reactor *r)
{
	ax_event *pe;
	assert(r != NULL);
	assert(r->pti != NULL);

	struct timerheap_st *pti = r->pti;

	if (timerheap_empty(pti))
		return NULL;

	pe = pti->heap[1].e;
	timerheap_remove_event(r, pe);

	return pe;
}

/*
* Heapify the timerheap in a bottom-up way after 
* alteration of the heap with the @idxth entry being the bottom entry.
* This function is used when inserting a entry to the heap.
*/
static void timerheap_heapify_bottomup(struct timerheap_st *pti, int idx)
{
	int parent;
	struct heap_entry *heap;
	struct heap_entry he;

	assert(pti != NULL);
	assert(idx > 0 && idx <= pti->size);

	heap = pti->heap;
	he = heap[idx];

	parent = idx >> 1;

	while(parent && ax_timeval_ge(heap[parent].expiration, he.expiration)) {
		heap[idx] = heap[parent];
		heap[idx].e->timerheap_idx = idx;
		idx = parent;
		parent >>= 1;
	}

	heap[idx] = he; 
	heap[idx].e->timerheap_idx = idx;
}

inline void timerheap_reset_timer(ax_reactor *r, ax_event *e)
{
	assert(r != NULL);
	assert(e != NULL);

	init_heap_entry(&r->pti->heap[e->timerheap_idx], e);
	timerheap_heapify_topdown(r->pti, e->timerheap_idx);
}

static void timerheap_heapify_topdown(struct timerheap_st *pti, int idx)
{
	int child, i, size;
	struct heap_entry *heap;
	struct heap_entry he;

	assert(pti != NULL);
	assert(idx > 0 && idx <= pti->size);

	size = pti->size;
	heap = pti->heap;
	he = heap[idx];

	for(i = idx; i<= (size >> 1); i = child) {
		child = i << 1;

		if (child + 1 <= size &&
			 ax_timeval_g(heap[child].expiration, heap[child + 1].expiration))
			++child;

		if (ax_timeval_g(heap[child].expiration, he.expiration))
			break;
		
		heap[i] = heap[child];
		heap[i].e->timerheap_idx = i;
	}

	heap[i] = he;
	heap[i].e->timerheap_idx = i;
}

int timerheap_add_event(ax_reactor *r, ax_event *e)
{
	struct timerheap_st *pti;
	assert(r != NULL);
	assert(r->pti != NULL);
	assert(e != NULL);

	pti = r->pti;

	if (e->timerheap_idx != E_OUT_OF_TIMERHEAP) {
		ax_perror("The timer event is already in the heap.");
		return (-1);
	}
	
	if (pti->size + 1 >= pti->capacity && timerheap_grow(pti, pti->size + 1) == -1) {
		ax_perror("failed on timerheap_grow: %s", strerror(errno));
		return (-1);
	}

	/* 
	* Initialize the heap_entry and heapfiy the heap.
	*/
	init_heap_entry(&pti->heap[++pti->size], e);
	timerheap_heapify_bottomup(pti, pti->size);

	return (0);
}

void timerheap_remove_event(ax_reactor *r, ax_event *e)
{
	assert(r != NULL);
	assert(r->pti != NULL);
	assert(e != NULL);
	assert(e->timerheap_idx != E_OUT_OF_TIMERHEAP);

	struct timerheap_st *pti = r->pti;
	struct heap_entry *heap = pti->heap;
	/* 
	* Fills in the entry being removed with the
	* rearest entry and heapify the heap.
	*/
	heap[e->timerheap_idx] = heap[pti->size--];

	/*
	* We might encounter that the @e is the rearest
	* entry in the heap. In that case, we simply
	* skip the heapification.
	*/
	if (e->timerheap_idx <= pti->size)
		timerheap_heapify_topdown(pti, e->timerheap_idx);
	e->timerheap_idx = E_OUT_OF_TIMERHEAP;
}

static int timerheap_free(struct timerheap_st *pti)
{
	assert(pti);
	free(pti->heap);
	pti->heap = NULL;
	pti->size = pti->capacity = 0;

	return (0);
}

void timerheap_clean_events(ax_reactor *r)
{
	assert(r);
	while(timerheap_pop_top(r));
}

void timerheap_destroy(ax_reactor *r)
{
	assert(r);

	struct timerheap_st *pti = r->pti;
	timerheap_free(pti);
	free(pti);
	r->pti = NULL;
}

