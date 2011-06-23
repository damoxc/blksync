/***
 * blksync - synchronize chunks of data between files and/or block devices
 *
 * Copyright (C) 2011  Damien Churchill <damoxc@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "action.h"

typedef struct bs_action_t *Action;

typedef enum {
    HASH_CHUNK,
    END_THREAD
} bs_action_type;

struct bs_action_t {
    bs_action_type type;
    void          *data;
};

/**
 * Create a new thread message
 */
Action bs_new_action(bs_action_type type, void *data) {
    Action action = malloc(sizeof(struct bs_action_t));

    if (action != NULL) {
        action->type = type;
        action->data = data;
    }

    return action;
}
