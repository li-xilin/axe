#include "event_ht.h"
#include "ax/link.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static const int event_ht_primes[] = {
			53, 97, 193, 389, 769, 1543, 3079, 6151,
			12289, 24593, 49157, 98317, 196613, 393241,
			786433, 1572869, 3145739, 3145739, 12582917,
			25165843, 50331653, 100663319, 201326611, 
			402653189, 805306457, 1610612741
			};
static const int event_ht_nprimes = sizeof(event_ht_primes) / sizeof(int);

inline unsigned event_ht_hash(unsigned key){
	key = (~key) + (key << 21); // key = (key << 21) - key - 1; 
	key = key ^ (key >> 24); 
	key = (key + (key << 3)) + (key << 8); // key * 265 
	key = key ^ (key >> 14); 
	key = (key + (key << 2)) + (key << 4); // key * 21 
	key = key ^ (key >> 28);
	key = key + (key << 31); 
	return key; 
}

static int event_ht_expand(ax_event_ht * ht, int size){
	printf("!! REALLOCATE\n");
	int new_len, new_idx, new_load_limit,  i;
	ax_link * new_table, *p, *q, *head;
	ax_event * entry;
	unsigned h;
	new_load_limit = ht->load_limit;
	new_len = ht->len;
	new_idx = ht->p_index;
	while(new_load_limit < size && new_idx < event_ht_nprimes){
		new_len = event_ht_primes[++new_idx];
		new_load_limit = ht->load_factor * new_len;
	}

	if((new_table = malloc(new_len * sizeof(ax_link))) == NULL){
		ax_perror("failed to malloc: %s", strerror(errno));
		return (-1);
	}

	for(i = 0; i < new_len; ++i){
		ax_link_init(&new_table[i]);
	}

	/*
	* Rehash and move all event to new_table.
	*/
	for(i = 0; i < ht->len; ++i){
		head = &(ht->table[i]);
		if(!ax_link_is_empty(head)){
			p = head->next;
			while(p != head){
				q = p->next;
				entry = ax_link_entry(p, ax_event, hash_link);
				ax_link_del(p);
				h = event_ht_hash(entry->fd) % new_len;
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

	return (0);
}

int event_ht_init(ax_event_ht * ht, double load_factor){
	int i, idx;

	idx = 0;
	ht->p_index = 0;
	ht->load_limit = load_factor * event_ht_primes[idx];
	ht->load_factor = load_factor;
	ht->n_entries = 0;
	ht->len = event_ht_primes[idx];

	ht->table = malloc(ht->len * sizeof(ax_link));
	if(ht->table == NULL){
		ax_perror("memory shortage.");
		return (-1);
	}
	for(i = 0; i < ht->len; ++i){
		ax_link_init(&ht->table[i]);
	}
	return (0);
}

int event_ht_insert(ax_event_ht * ht, ax_event * new, unsigned key){
	unsigned h;

	if(new->hash_link.prev || new->hash_link.next){
		/*
		* This event is already in the hash table.
		* Assume every event only can be in one reactor.
		*/
		return (-1);
	}

	/* expand the hash table if nessesary */
	if(ht->n_entries >= ht->load_limit)
		event_ht_expand(ht, ht->n_entries + 1);

	
	h = event_ht_hash(key) % ht->len;
	ax_link_add_back(&new->hash_link, &ht->table[h]);
	++(ht->n_entries);
	return (0);
}

int event_ht_insert_replace(ax_event_ht * ht, ax_event * new, unsigned key){
	unsigned h;

	if(new->hash_link.prev || new->hash_link.next){
		/*
		* This event is not in the hash table.
		* Assume every event only can be in one reactor.
		*/
		return (0);
	}

	/* expand the hash table if nessesary */
	if(ht->n_entries >= ht->load_limit)
		event_ht_expand(ht, ht->n_entries + 1);

	/* rehash the key */
	h = event_ht_hash(key) % ht->len;
	ax_link_add_back(&new->hash_link, &ht->table[h]);
	++(ht->n_entries);
	return (0);
}

void event_ht_delete_by_key(ax_event_ht * ht, unsigned key){
	ax_link *p;
	unsigned h;
	h = event_ht_hash(key) % ht->len;
	
	ax_link_foreach(p, &ht->table[h]){
		ax_event * entry = ax_link_entry(p, ax_event, hash_link);
		if(entry->fd == key){
			ax_link_del(p);
			--(ht->n_entries);
			return;
		}
	}
}

int event_ht_delete(ax_event_ht * ht, ax_event * e){

	if(e->hash_link.prev == NULL || e->hash_link.next == NULL){
		/*
		* This event is not in the hash table.
		* Assume every event only can be in one reactor.
		*/
		return (-1);
	}

	ax_link_del(&e->hash_link);
	--(ht->n_entries);
	return (0);
}

ax_event * event_ht_retrieve(ax_event_ht * ht, unsigned key){
	unsigned h;
	ax_link *p;

	h =  event_ht_hash(key) % ht->len;

	ax_link_foreach(p, &ht->table[h]){
		ax_event * entry = ax_link_entry(p, ax_event, hash_link);
		if(entry->fd == key){
			return entry;
		}
	}
	
	return NULL;
}

void event_ht_free(ax_event_ht * ht){
	free(ht->table);
}

ax_event * event_ht_iterate(ax_event_ht * ht, unsigned key){
	unsigned h;
	ax_link *p;
	h = event_ht_hash(key) % ht->len;
	ax_link_foreach(p, &ht->table[h]){
		ax_event * entry = ax_link_entry(p, ax_event, hash_link);
		printf("key:%d, fd:%lld\n", key, (long long int)entry->fd);
	}
	return NULL;
}
