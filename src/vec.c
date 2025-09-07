/*
 * Copyright 2025 8dcc
 *
 * This file is part of maze-generatorñ.
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

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#include "include/vec.h"

bool vec_stack_init(Vec2Stack* stack, size_t size) {
    stack->pos = 0;
    stack->size = size;
    stack->data = calloc(stack->size, sizeof(Vec2));
    return (stack->data != NULL);
}

void vec_stack_destroy(Vec2Stack* stack) {
    if (stack->data != NULL) {
        free(stack->data);
        stack->data = NULL;
    }
}

void vec_stack_push(Vec2Stack* stack, Vec2 v) {
    if (stack->pos < stack->size)
        stack->data[stack->pos++] = v;
}

Vec2 vec_stack_pop(Vec2Stack* stack) {
    if (stack->pos <= 0)
        return VEC2(-1, -1);

    return stack->data[--stack->pos];
}
