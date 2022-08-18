/*
* Copyright (c) 2014 Xinjing Chow
*/

#ifndef AX_EVENT_EVENT_HT_H
#define AX_EVENT_EVENT_HT_H

#include "ax/event/event.h"

typedef struct ax_event_ht_st {
	ax_link *table;
	/* The index of event_ht_primes we are using as the size of the hash table */
	int p_index;
	/* We should expand the hash table if the threshold has been exceeded. */
	int load_limit;
	/* The load factor we apply to the hash table */
	double load_factor;
	/* the number of entries this hash table has */
	int n_entries;
	/* The number of slots this hash table has */
	int len;
} ax_event_ht;

/*
* Thomas Wang's hash function
* @key: key to be hashed
*/
unsigned event_ht_hash(unsigned key);

/*
* Initialize the event hash table.
* @ht: &ax_event_ht_st to be initialized.
* @load_factor: the load factor we apply on the hash table.
*/
int event_ht_init(ax_event_ht * ht, double load_factor);

/*
* Insert a event into the hash table.
* Do nothing if the event is already in the table,
* @ht: &ax_event_ht into which the event to be inserted
* @event: &ax_event entry to be inserted
* @key: hash key
*/
int event_ht_insert(ax_event_ht * ht, ax_event *new_entry, unsigned key);

/*
* Insert a event into the hash table. 
* Replace old event by new one if the old event is already in the table.
* @ht: &ax_event_ht into which the event to be inserted
* @event: &ax_event entry to be inserted
* @key: hash key
*/
int event_ht_insert_replace(ax_event_ht * ht, ax_event * new_entry, unsigned key);

/*
* Delete the event with the key from the hash table.
* Do nothing if there is no matching key.
* @ht: &ax_event_ht from which the event will be deleted
* @key: hash key
*/
void event_ht_delete_by_key(ax_event_ht * ht, unsigned key);

/*
* Delete the event with the key from the hash table.
* Do nothing if there is no matching key.
* @ht: &ax_event_ht from which the event will be deleted
* @key: hash key
*/
int event_ht_delete(ax_event_ht * ht, ax_event * e);


/*
* Retrieve the coressponding event from the hash table.
* Return null if there is no matching key.
* @ht: &ax_event_ht from which the event will be retrieved
* @key: hash key
*/
ax_event * event_ht_retrieve(ax_event_ht * ht, unsigned key);

/*
* Free up the hash table.
* @ht: the hash table
*/
void event_ht_free(ax_event_ht * ht);

#endif
