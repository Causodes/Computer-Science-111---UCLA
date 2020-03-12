/**
Name: Tian Ye
Email: tianyesh@gmail.com
UID: 704931660
*/

#define _GNU_SOURCE
#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	// Edge case check
	if(list == NULL || element == NULL) {
		return;
	}
	
	// Loop through circular list until full traverse (hit head node again)
	SortedList_t * node = list->next;
	while(node != list) {
		// Match found
		if(strcmp(element -> key, node -> key) <= 0) {
			break;
		}
		node = node -> next;
	}
	
	// Insert element
	if(opt_yield & INSERT_YIELD) {
		sched_yield();
	}
	
	// Adjust pointers
	element -> next = node;
	element -> prev = node -> prev;
	node -> prev -> next = element;
	node -> prev = element;
}

int SortedList_delete(SortedListElement_t *element) {
	// Edge case check
	if(element == NULL) {
		return 1;
	}
	// Corrupt pointer check
	if(element -> next -> prev != element || element -> prev -> next != element) {
		return 1;
	}
	
	// Delete element
	if(opt_yield & DELETE_YIELD) {
		sched_yield();
	}
	
	// Adjust pointers
	element -> next -> prev = element -> prev;
	element -> prev -> next = element -> next;
	return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	// Edge case check
	if(list == NULL || key == NULL) {
		return NULL;
	}
	
	// Loop through circular list until full traverse (hit head node again)
	SortedList_t * node = list->next;
	while(node != list) {
		// Match found
		if(strcmp(node -> key, key) == 0) {
			return node;
		}
		if(opt_yield & LOOKUP_YIELD) {
			sched_yield();
		}
		node = node -> next;
	} 
	return NULL;
}

int SortedList_length(SortedList_t *list) {
	// Edge case check
	if(list == NULL) {
		return -1;
	}
	
	int length = 0;
	// Loop through circular list until full traverse (hit head node again)
	SortedList_t * node = list->next;
	while(node != list) {
		length++;
		if(opt_yield & LOOKUP_YIELD) {
			sched_yield();
		}
		node = node -> next;
	} 
	return length;
}