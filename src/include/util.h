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

#ifndef UTIL_H_
#define UTIL_H_ 1

#include <stdio.h>
#include <stdlib.h>

#define ERR(...)                                                               \
    do {                                                                       \
        fprintf(stderr, "%s: ", __func__);                                     \
        fprintf(stderr, __VA_ARGS__);                                          \
        fputc('\n', stderr);                                                   \
    } while (0)

#define DIE(...)                                                               \
    do {                                                                       \
        ERR(__VA_ARGS__);                                                      \
        exit(1);                                                               \
    } while (0)

#endif /* UTIL_H_ */
