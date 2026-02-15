#pragma once

#include <stdint.h>

typedef struct {
	uint64_t bcd;
	char * name;
} mcdonalds_item_t;

extern mcdonalds_item_t us_items[];
extern mcdonalds_item_t ca_items[];
extern mcdonalds_item_t ch_items[];
extern mcdonalds_item_t uk_items[];
extern mcdonalds_item_t sa_items[];
extern mcdonalds_item_t gb_items[];
extern mcdonalds_item_t de_items[];
