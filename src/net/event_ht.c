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

#include "event_ht.h"
#include "ax/link.h"
#include "ax/mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

static const int event_ht_primes[] =
{
	53, 97, 193, 389, 769, 1543, 3079, 6151,
	12289, 24593, 49157, 98317, 196613, 393241,
	786433, 1572869, 3145739, 3145739, 12582917,
	25165843, 50331653, 100663319, 201326611, 
	402653189, 805306457, 1610612741
};

static const int event_ht_nprimes = sizeof(event_ht_primes) / sizeof(int);

static int event_ht_expand(ax_event_ht *ht, int size)
{
	int new_len, new_idx, new_load_limit;
	ax_link * new_table, *p, *q, *head;
	ax_event * entry;
	unsigned h;
	new_load_limit = ht->load_limit;
	new_len = ht->len;
	new_idx = ht->p_index;
	while(new_load_limit < size && new_idx < event_ht_nprimes) {
		new_len = event_ht_primes[++new_idx];
		new_load_limit = ht->load_factor * new_len;
	}

	if(!(new_table = malloc(new_len * sizeof(ax_link)))) {
		ax_perror("failed to malloc: %s", strerror(errno));
		return -1;
	}

	for(int i = 0; i < new_len; ++i)
		ax_link_init(&new_table[i]);

	/*
	 * Rehash and move all event to new_table.
	 */
	for(int i = 0; i < ht->len; ++i) {
		head = &(ht->table[i]);
		if(!ax_link_is_empty(head)) {
			p = head->next;
			while(p != head) {
				q = p->next;
				entry = ax_link_entry(p, ax_event, hash_link);
				ax_link_del(p);
				h = ax_hash64_thomas(entry->fd) % new_len;
				ax_link_add_back(&entry->hash_link, &new_table[h]);
				p = q;
			}
		}
	}

	free(ht->table);

	ht->p_index = new_idx;
	ht->table = new_table;
	ht->len = new_len;
	ht->load_limit = new_load_limit;

	return 0;
}

int event_ht_init(ax_event_ht * ht, double load_factor)
{
	int idx = 0;

	ht->p_index = 0;
	ht->load_limit = load_factor * event_ht_primes[idx];
	ht->load_factor = load_factor;
	ht->n_entries = 0;
	ht->len = event_ht_primes[idx];

	ht->table = malloc(ht->len * sizeof(ax_link));
	if(ht->table == NULL) {
		ax_perror("memory shortage.");
		return -1;
	}
	for(int i = 0; i < ht->len; ++i)
		ax_link_init(&ht->table[i]);
	return 0;
}

int event_ht_insert(ax_event_ht *ht, ax_event *new, unsigned key)
{
	assert(!new->hash_link.prev && !new->hash_link.next);

	/* expand the hash table if nessesary */
	if(ht->n_entries >= ht->load_limit)
		event_ht_expand(ht, ht->n_entries + 1);

	unsigned h = ax_hash64_thomas(key) % ht->len;
	ax_link_add_back(&new->hash_link, &ht->table[h]);
	++(ht->n_entries);
	return 0;
}

int event_ht_insert_replace(ax_event_ht *ht, ax_event *new, unsigned key){
	assert(!new->hash_link.prev && !new->hash_link.next);

	/* expand the hash table if nessesary */
	if(ht->n_entries >= ht->load_limit)
		event_ht_expand(ht, ht->n_entries + 1);

	/* rehash the key */
	unsigned h = ax_hash64_thomas(key) % ht->len;
	ax_link_add_back(&new->hash_link, &ht->table[h]);
	++(ht->n_entries);
	return 0;
}

void event_ht_delete_by_key(ax_event_ht * ht, unsigned key) {
	ax_link *p;
	unsigned h;
	h = ax_hash64_thomas(key) % ht->len;

	ax_link_foreach(p, &ht->table[h]){
		ax_event * entry = ax_link_entry(p, ax_event, hash_link);
		if(entry->fd == key){
			ax_link_del(p);
			--(ht->n_entries);
			return;
		}
	}
}

int event_ht_delete(ax_event_ht * ht, ax_event * e)
{
	if(e->hash_link.prev == NULL || e->hash_link.next == NULL)
		return -1;

	ax_link_del(&e->hash_link);
	--(ht->n_entries);
	return 0;
}

ax_event * event_ht_retrieve(ax_event_ht * ht, unsigned key)
{
	unsigned h = ax_hash64_thomas(key) % ht->len;

	ax_link *p;
	ax_link_foreach(p, &ht->table[h]) {
		ax_event * entry = ax_link_entry(p, ax_event, hash_link);
		if(entry->fd == key)
			return entry;
	}

	return NULL;
}

void event_ht_free(ax_event_ht * ht)
{
	free(ht->table);
}

ax_event * event_ht_iterate(ax_event_ht * ht, unsigned key)
{
	unsigned h;
	ax_link *p;
	h = ax_hash64_thomas(key) % ht->len;
	ax_link_foreach(p, &ht->table[h]) {
		ax_event * entry = ax_link_entry(p, ax_event, hash_link);
		printf("key:%d, fd:%lld\n", key, (long long int)entry->fd);
	}
	return NULL;
}
