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

#ifndef MAZE_CTX_H_
#define MAZE_CTX_H_ 1

#include <stdint.h>
#include <stdbool.h>

#include <png.h>

#include "vec.h"

/*----------------------------------------------------------------------------*/

/*
 * Enumeration representing all wall orientations of a cell.
 */
enum EWalls {
    WALL_INVALID = 0,
    WALL_NORTH   = (1 << 0),
    WALL_SOUTH   = (1 << 1),
    WALL_WEST    = (1 << 2),
    WALL_EAST    = (1 << 3),
};

/*----------------------------------------------------------------------------*/

/*
 * Structure representing a cell of the maze.
 */
typedef struct {
    uint8_t walls; /* Each bit represents a wall from the enumeration */
    bool visited;
} MazeCell;

/*
 * Structure with the necessary context for generating mazes.
 */
typedef struct {
    MazeCell* grid;
    int grid_w, grid_h; /* Cell number, not pixels */

    /* Stack of recently visited positions */
    Vec2Stack visited_stack;
} MazeCtx;

/*----------------------------------------------------------------------------*/

/*
 * Initialize a 'MazeCtx' structure with the specified grid width and height.
 */
bool maze_ctx_init(MazeCtx* ctx, int grid_w, int grid_h);

/*
 * Destroy a maze context, freeing its necessary members. Doesn't free the
 * argument pointer itself.
 */
void maze_ctx_destroy(MazeCtx* ctx);

/*
 * Generate a maze using the specified context.
 */
void maze_ctx_generate(MazeCtx* ctx);

#endif /* MAZE_CTX_H_ */
