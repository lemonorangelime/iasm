#pragma once

typedef struct linked {
	struct linked * next;
	void * p;
} linked_t;

linked_t * linked_leaf(linked_t * node);
linked_t * linked_create(void * p);
linked_t * linked_add(linked_t * list, linked_t * node);
linked_t * linked_append(linked_t * list, void * p);
