#ifndef LIST_H_
#define LIST_H_

/*
 * Generic C Linked-list derived from Linux kernel source
 * See: https://github.com/torvalds/linux/blob/master/include/linux/list.h
 */

#include <stdlib.h>
#include <stddef.h>

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

/**
 * Initialize a list_head structure
 */
static inline void INIT_LIST_HEAD (struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

/**
 * Insert a @new entry between two consecutive @prev and @next
 */
static inline void __list_add(struct list_head *new,
                               struct list_head *prev,
                               struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/**
 * Insert a @new entry after the specified @head
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

/**
 * Delete a list entry by setting prev/next entries point to each other
 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * Delete a list entry from the list
 */
static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

/**
 * Iterate over a list
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * Get the list entry containing the member
 */
#define container_of(ptr, type, member) ({                          \
            const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
            (type*)( (char *)__mptr - offsetof(type, member) ); })

/**
 * Get the struct for the entry
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * Iterate over a list with each entry structure
 */
#define list_for_each_entry(pos, head, member)                          \
    for (pos = list_entry((head)->next, typeof(*pos), member);          \
         &pos-> member != (head);                                       \
         pos = list_entry(pos->member.next, typeof(*pos), member))

#endif
