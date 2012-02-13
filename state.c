/*
 * Copyright(C) 2010-2011, Jocelyn Falempe jock@inpactien.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "state.h"



LIST_HEAD(current_state);
LIST_HEAD(previous_state);

state_t *create_first_state(void) {
	state_t *new;

	new = malloc(sizeof(*new));
	list_add(&new->list, &previous_state);
	new->dir.x = 0;



}

state_t *calc_new_state(state_t * prev, float t) {
	state_t * new;

	return new;

}

state_t *calc_new_statelist(state_t * prev, float t) {
	state_t *obj;
	list_for_each_entry(obj, previous_state, list) {
		calc_new_state(obj,t);

	}


}

void discard_oldstate(void) {

}

