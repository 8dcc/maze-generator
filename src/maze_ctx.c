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

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "include/maze_ctx.h"
#include "include/util.h"
#include "include/vec.h"
#include "include/config.h"

/*
 * Return a random adjacent cell which has not been visited.
 */
static enum EWalls random_unvisited_neighbour(MazeCtx* ctx, Vec2 v) {
    const int x = v.x;
    const int y = v.y;

    enum EWalls possible_walls[4 * BIAS_VERT * BIAS_HORIZ];
    int num_stored = 0;

    if (y >= 1 && ctx->grid[ctx->grid_w * (y - 1) + x].visited == false)
        for (int i = 0; i < BIAS_VERT; i++)
            possible_walls[num_stored++] = WALL_NORTH;
    if (y < ctx->grid_h - 1 &&
        ctx->grid[ctx->grid_w * (y + 1) + x].visited == false)
        for (int i = 0; i < BIAS_VERT; i++)
            possible_walls[num_stored++] = WALL_SOUTH;
    if (x >= 1 && ctx->grid[ctx->grid_w * y + (x - 1)].visited == false)
        for (int i = 0; i < BIAS_HORIZ; i++)
            possible_walls[num_stored++] = WALL_WEST;
    if (x < ctx->grid_w - 1 &&
        ctx->grid[ctx->grid_w * y + (x + 1)].visited == false)
        for (int i = 0; i < BIAS_HORIZ; i++)
            possible_walls[num_stored++] = WALL_EAST;

    if (num_stored <= 0)
        return WALL_INVALID;

    const int random_pos = rand() % num_stored;
    return possible_walls[random_pos];
}

/*
 * Return the position of the cell adjacent to V, given a wall orientation.
 */
static Vec2 pos_from_wall(Vec2 v, enum EWalls wall) {
    switch (wall) {
        case WALL_NORTH:
            return VEC2(v.x, v.y - 1);
        case WALL_SOUTH:
            return VEC2(v.x, v.y + 1);
        case WALL_WEST:
            return VEC2(v.x - 1, v.y);
        case WALL_EAST:
            return VEC2(v.x + 1, v.y);
        default:
            ERR("Invalid wall direction: %d", wall);
            return VEC2(-1, -1);
    }
}

/*
 * Return the wall at the opposite direction of the argument.
 */
static inline int opposite_wall(enum EWalls wall) {
    switch (wall) {
        case WALL_NORTH:
            return WALL_SOUTH;
        case WALL_SOUTH:
            return WALL_NORTH;
        case WALL_WEST:
            return WALL_EAST;
        case WALL_EAST:
            return WALL_WEST;
        default:
            ERR("Invalid wall number (%d)", wall);
            return WALL_INVALID;
    }
}

/*
 * Remove the walls between cells A and B. The wall direction is relative to A.
 */
static inline void remove_walls(MazeCtx* ctx,
                                Vec2 a,
                                Vec2 b,
                                enum EWalls wall) {
    ctx->grid[ctx->grid_w * a.y + a.x].walls &= ~wall;
    ctx->grid[ctx->grid_w * b.y + b.x].walls &= ~opposite_wall(wall);
}

/*----------------------------------------------------------------------------*/

bool maze_ctx_init(MazeCtx* ctx, int grid_w, int grid_h) {
    ctx->grid_w = grid_w;
    ctx->grid_h = grid_h;

    ctx->grid = calloc(ctx->grid_w * ctx->grid_h, sizeof(MazeCell));
    if (ctx->grid == NULL) {
        ERR("Failed to allocate grid.");
        return false;
    }

    if (!vec_stack_init(&ctx->visited_stack, ctx->grid_w * ctx->grid_h)) {
        ERR("Failed to initialize 2D vector stack.");
        return false;
    }

    return true;
}

void maze_ctx_destroy(MazeCtx* ctx) {
    if (ctx->grid != NULL) {
        free(ctx->grid);
        ctx->grid = NULL;
    }

    vec_stack_destroy(&ctx->visited_stack);
}

void maze_ctx_generate(MazeCtx* ctx) {
    printf("Generating %dx%d maze...\n", ctx->grid_w, ctx->grid_h);

    /* Initialize random seed for maze generation */
    srand(time(NULL));

    /* Clear maze */
    for (int y = 0; y < ctx->grid_h; y++) {
        for (int x = 0; x < ctx->grid_w; x++) {
            ctx->grid[ctx->grid_w * y + x].walls =
              WALL_NORTH | WALL_SOUTH | WALL_WEST | WALL_EAST;
            ctx->grid[ctx->grid_w * y + x].visited = false;
        }
    }

    /* Push starting position (center) into the stack, and mark as visited */
    Vec2 cur_pos = VEC2(ctx->grid_w / 2, ctx->grid_h / 2);
    vec_stack_push(&ctx->visited_stack, cur_pos);
    ctx->grid[ctx->grid_w * cur_pos.y + cur_pos.x].visited = true;

    /* While we have positions left in the stack */
    for (;;) {
        /* Get next position from the stack */
        cur_pos = vec_stack_pop(&ctx->visited_stack);

        /* No more positions to check, we are done */
        if (cur_pos.x < 0 || cur_pos.y < 0)
            break;

        /* Get a random adjacent cell which has not been visited */
        const int valid_neighbour_wall =
          random_unvisited_neighbour(ctx, cur_pos);
        if (valid_neighbour_wall == WALL_INVALID)
            continue;

        /* Push current position */
        vec_stack_push(&ctx->visited_stack, cur_pos);

        /* Get position of neighbour from wall orientation */
        const Vec2 neighbour = pos_from_wall(cur_pos, valid_neighbour_wall);

        if (neighbour.x < 0 || neighbour.x >= ctx->grid_w || neighbour.y < 0 ||
            neighbour.y >= ctx->grid_h) {
            ERR("Warning: Neighbour out of bounds.");
            continue;
        }

        /* Remove the wall in the current cell and the random neighbour */
        remove_walls(ctx, cur_pos, neighbour, valid_neighbour_wall);

        /* Mark neighbour as visited and push to the stack */
        ctx->grid[ctx->grid_w * neighbour.y + neighbour.x].visited = true;
        vec_stack_push(&ctx->visited_stack, neighbour);
    }

    /* Remove walls of entry and exit */
    ctx->grid[ctx->grid_w * START_Y + START_X].walls &= ~WALL_NORTH;
    ctx->grid[ctx->grid_w * END_Y + END_X].walls &= ~WALL_SOUTH;
}
