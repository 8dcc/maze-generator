/*
 * Copyright 2025 8dcc
 *
 * This file is part of maze-generator.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VEC_H_
#define VEC_H_ 1

#include <stddef.h>
#include <stdbool.h>

/*
 * Simple constructor macro.
 */
#define VEC2(X, Y) ((Vec2){ .x = (X), .y = (Y) })

/*
 * Structure representing a 2D vector.
 */
typedef struct {
    int x, y;
} Vec2;

/*
 * Structure representing a stack of 2D vectors.
 */
typedef struct {
    Vec2* data;
    size_t pos;
    size_t size;
} Vec2Stack;

/*----------------------------------------------------------------------------*/

/*
 * Initialize a 2D vector stack.
 */
bool vec_stack_init(Vec2Stack* stack, size_t size);

/*
 * Destroy a 2D vector stack, freeing its necessary members. Doesn't free the
 * argument pointer itself.
 */
void vec_stack_destroy(Vec2Stack* stack);

/*
 * Push an element into the top of a 2D vector stack.
 */
void vec_stack_push(Vec2Stack* stack, Vec2 v);

/*
 * Pop an element from the top of a 2D vector stack.
 */
Vec2 vec_stack_pop(Vec2Stack* stack);

#endif /* VEC_H_ */
