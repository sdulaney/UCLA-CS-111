/*
  NAME: Stewart Dulaney
  EMAIL: sdulaney@ucla.edu
  ID: 904-064-791
 */

#include <string.h>
#include <pthread.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
    if (list == NULL || element == NULL || list->key != NULL)
	return;
    SortedList_t* prev = list;
    SortedList_t* curr = list->next;
    while (curr != list) {
	if (strcmp(curr->key, element->key) < 0)
	    break;
	prev = curr;
	curr = curr->next;
    }
    if (opt_yield & INSERT_YIELD)
	sched_yield();
    element->next = prev->next;
    element->prev = prev;
    prev->next = element;
    element->next->prev = element;
}

int SortedList_delete( SortedListElement_t *element) {
    if (element == NULL || element->key == NULL || element->prev->next != element || element->next->prev != element)
	return 1;
    if (opt_yield & DELETE_YIELD)
	sched_yield();
    element->prev->next = element->next;
    element->next->prev = element->prev;
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
    if (list == NULL || key == NULL || list->key != NULL)
	return NULL;
    SortedList_t* curr = list->next;
    while (curr != list) {
	if (strcmp(curr->key, key) == 0)
	    return curr;
	if (opt_yield & LOOKUP_YIELD)
	    sched_yield();
	curr = curr->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list) {
    if (list == NULL || list->key != NULL)
	return -1;
    int length = 0;
    SortedList_t* curr = list->next;
    while (curr != list) {
	if (curr->prev->next != curr || curr->next->prev != curr)
	    return -1;
	length++;
	if (opt_yield & LOOKUP_YIELD)
	    sched_yield();
	curr = curr->next;
    }
    return length;
}
