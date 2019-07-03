#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>

struct list_head {
    struct list_head *next, *prev;
};

/**
 * Initialize the list as an empty list.
 *
 * Example:
 * INIT_LIST_HEAD(&bar->list_of_foos);
 *
 * @param The list to initialized.
 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void
INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list->prev = list;
}

static inline void
__list_add(struct list_head *entry,
                struct list_head *prev, struct list_head *next)
{
    next->prev = entry;
    entry->next = next;
    entry->prev = prev;
    prev->next = entry;
}

/**
 * Insert a new element after the given list head. The new element does not
 * need to be initialised as empty list.
 * The list changes from:
 *      head → some element → ...
 * to
 *      head → new element → older element → ...
 *
 * Example:
 * struct foo *newfoo = malloc(...);
 * list_add(&newfoo->entry, &bar->list_of_foos);
 *
 * @param entry The new element to prepend to the list.
 * @param head The existing list.
 */
static inline void
list_add(struct list_head *entry, struct list_head *head)
{
    __list_add(entry, head, head->next);
}

/**
 * Append a new element to the end of the list given with this list head.
 *
 * The list changes from:
 *      head → some element → ... → lastelement
 * to
 *      head → some element → ... → lastelement → new element
 *
 * Example:
 * struct foo *newfoo = malloc(...);
 * list_add_tail(&newfoo->entry, &bar->list_of_foos);
 *
 * @param entry The new element to prepend to the list.
 * @param head The existing list.
 */
static inline void
list_add_tail(struct list_head *entry, struct list_head *head)
{
    __list_add(entry, head->prev, head);
}

static inline void
__list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * Remove the element from the list it is in. Using this function will reset
 * the pointers to/from this element so it is removed from the list. It does
 * NOT free the element itself or manipulate it otherwise.
 *
 * Using list_del on a pure list head (like in the example at the top of
 * this file) will NOT remove the first element from
 * the list but rather reset the list as empty list.
 *
 * Example:
 * list_del(&foo->entry);
 *
 * @param entry The element to remove.
 */
static inline void
list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void
list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

static inline void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

/**
 * Check if the list is empty.
 *
 * Example:
 * list_empty(&bar->list_of_foos);
 *
 * @return True if the list contains one or more elements or False otherwise.
 */
static inline bool
list_empty(struct list_head *head)
{
    return head->next == head;
}

/**
 * Returns a pointer to the container of this list element.
 *
 * Example:
 * struct foo* f;
 * f = container_of(&foo->entry, struct foo, entry);
 * assert(f == foo);
 *
 * @param ptr Pointer to the struct list_head.
 * @param type Data type of the list element.
 * @param member Member name of the struct list_head field in the list element.
 * @return A pointer to the data struct containing the list head.
 */
#ifndef container_of
#define container_of(ptr, type, member) \
    (type *)((char *)(ptr) - (char *) &((type *)0)->member)
#endif

/**
 * Alias of container_of
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * Retrieve the first list entry for the given list pointer.
 *
 * Example:
 * struct foo *first;
 * first = list_first_entry(&bar->list_of_foos, struct foo, list_of_foos);
 *
 * @param ptr The list head
 * @param type Data type of the list element to retrieve
 * @param member Member name of the struct list_head field in the list element.
 * @return A pointer to the first list element.
 */
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

/**
 * Retrieve the last list entry for the given listpointer.
 *
 * Example:
 * struct foo *first;
 * first = list_last_entry(&bar->list_of_foos, struct foo, list_of_foos);
 *
 * @param ptr The list head
 * @param type Data type of the list element to retrieve
 * @param member Member name of the struct list_head field in the list element.
 * @return A pointer to the last list element.
 */
#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)

#define __container_of(ptr, sample, member)				\
    (void *)container_of((ptr), typeof(*(sample)), member)

/**
 * Loop through the list given by head and set pos to struct in the list.
 *
 * Example:
 * struct foo *iterator;
 * list_for_each_entry(iterator, &bar->list_of_foos, entry) {
 *      [modify iterator]
 * }
 *
 * This macro is not safe for node deletion. Use list_for_each_entry_safe
 * instead.
 *
 * @param pos Iterator variable of the type of the list elements.
 * @param head List head
 * @param member Member name of the struct list_head in the list elements.
 *
 */
#define list_for_each_entry(pos, head, member)				\
    for (pos = __container_of((head)->next, pos, member);		\
	 &pos->member != (head);					\
	 pos = __container_of(pos->member.next, pos, member))

/**
 * Loop through the list, keeping a backup pointer to the element. This
 * macro allows for the deletion of a list element while looping through the
 * list.
 *
 * See list_for_each_entry for more details.
 */
#define list_for_each_entry_safe(pos, tmp, head, member)		\
    for (pos = __container_of((head)->next, pos, member),		\
	 tmp = __container_of(pos->member.next, pos, member);		\
	 &pos->member != (head);					\
	 pos = tmp, tmp = __container_of(pos->member.next, tmp, member))


#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = __container_of((head)->prev, pos, member);		\
	     &pos->member != (head);					\
	     pos = __container_of(pos->member.prev, pos, member))

#define list_for_each_entry_continue(pos, head, member)			\
	for (pos = __container_of(pos->member.next, pos, member);	\
	     &pos->member != (head);					\
	     pos = __container_of(pos->member.next, pos, member))

#define list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = __container_of(pos->member.prev, pos, member);	\
	     &pos->member != (head);					\
	     pos = __container_of(pos->member.prev, pos, member))

#define list_for_each_entry_from(pos, head, member)			\
	for (;								\
	     &pos->member != (head);					\
	     pos = __container_of(pos->member.next, pos, member))

#endif
