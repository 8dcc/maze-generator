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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/util.h"
#include "include/vec.h"
#include "include/maze_ctx.h"
#include "include/image.h"

/*
 * Structure representing the parsed program arguments.
 */
typedef struct {
    const char* output_filename;
    int grid_w, grid_h;
} Args;

/*----------------------------------------------------------------------------*/

static bool parse_args(Args* args, int argc, char** argv) {
    /* Default arguments */
    args->output_filename = "output.png";
    args->grid_w          = 100;
    args->grid_h          = 100;

    if (argc >= 2)
        args->output_filename = argv[1];

    if (argc >= 3)
        args->grid_w = atoi(argv[2]);
    if (argc >= 4)
        args->grid_h = atoi(argv[3]);

    if (args->grid_w <= 0 || args->grid_h <= 0) {
        ERR("Invalid grid size.");
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    Args args;
    if (!parse_args(&args, argc, argv)) {
        fprintf(stderr, "Usage: %s [OUTPUT.png] [WIDTH] [HEIGHT]\n", argv[0]);
        return 1;
    }

    MazeCtx ctx;
    if (!maze_ctx_init(&ctx, args.grid_w, args.grid_h)) {
        ERR("Failed to initialize maze context.");
        return 1;
    }

    maze_ctx_generate(&ctx);

    if (!write_png_from_maze_ctx(&ctx, args.output_filename)) {
        ERR("Failed to generate PNG image from maze.");
        return 1;
    }

    puts("Done.");
    maze_ctx_destroy(&ctx);
    return 0;
}
