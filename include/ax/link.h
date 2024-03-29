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

#ifndef AX_LINK_H
#define AX_LINK_H
#include "def.h"

#ifndef AX_LINK_DEFINED
#define AX_LINK_DEFINED
typedef struct ax_link_st ax_link;
#endif

struct ax_link_st
{
	ax_link *prev, *next;
};

#define AX_LINK_INITIALIZER(name) { &(name), &(name) }

inline static void ax_link_init(ax_link *list)
{
        list->next = list->prev = list;
}

inline static void ax_link_init_empty(ax_link *list){
        list->next = list->prev = NULL;
}

inline static void __ax_list_add(ax_link *new_link, ax_link *prev, ax_link *next)
{
        next->prev = new_link;
        new_link->next = next;
        new_link->prev = prev;
        prev->next = new_link;
}

inline static void ax_link_add_front(ax_link *new_link, ax_link *head)
{
        __ax_list_add(new_link, head, head->next);
}

inline static void ax_link_add_back(ax_link *new_link, ax_link *head)
{
        __ax_list_add(new_link, head->prev, head);
}

inline static void __list_del(ax_link * prev, ax_link * next)
{
        next->prev = prev;
        prev->next = next;
}

inline static void ax_link_del(ax_link *entry)
{
        if(entry == 0 || entry->prev == 0 || entry->next == 0)
		return;

        __list_del(entry->prev, entry->next);
        entry->prev = entry->next = 0;
}

inline static int ax_link_is_empty(const ax_link *head)
{
        return head->next == head;
}

#define ax_link_entry(ptr, type, member) ax_container_of(ptr, type, member)

#define ax_link_first_entry(ptr, type, member) ax_link_entry((ptr)->next, type, member)

/**
 * Note, that list is expected to be not empty.
 */
#define ax_link_last_entry(ptr, type, member) ax_link_entry((ptr)->prev, type, member)

/**
 * Note that if the list is empty, it returns NULL.
 */
#define ax_link_first_entry_or_null(ptr, type, member) \
	(!ax_link_is_empty(ptr) ? ax_link_first_entry(ptr, type, member) : NULL)

#define ax_link_last_entry_or_null(ptr, type, member) \
	(!ax_link_is_empty(ptr) ? ax_link_last_entry(ptr, type, member) : NULL)

#define ax_link_foreach(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

#endif
