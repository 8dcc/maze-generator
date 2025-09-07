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

#ifndef IMAGE_H_
#define IMAGE_H_ 1

#include <stdbool.h>

#include <png.h>

#include "maze_ctx.h"

typedef struct {
    png_bytep* rows;
    int img_w, img_h; /* Pixels */
} Image;

/*----------------------------------------------------------------------------*/

bool write_png_from_maze_ctx(const MazeCtx* maze, const char* output_filename);

#endif /* IMAGE_H_ */
