/*
#include <linked.h>
#include <stddef.h>
#include <stdlib.h>

linked_t * linked_leaf(linked_t * node) {
	while (node->next) {
		node = node->next;
	}
	return node;
}

linked_t * linked_create(void * p) {
	linked_t * linked = malloc(sizeof(linked_t));
	linked->next = NULL;
	linked->p = p;
	return linked;
}

linked_t * linked_add(linked_t * list, linked_t * node) {
	if (!list) {
		node->next = NULL;
		return node;
	}

	linked_t * leaf = linked_leaf(list);
	leaf->next = node;
	node->next = NULL;
	return list;
}

linked_t * linked_append(linked_t * list, void * p) {
	if (!list) {
		return linked_create(p);
	}
	linked_t * leaf = linked_leaf(list);
	linked_t * node = linked_create(p);
	leaf->next = node;
	node->next = NULL;
	return list;
}
*/
