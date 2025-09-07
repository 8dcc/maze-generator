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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#include "include/image.h"
#include "include/maze_ctx.h"
#include "include/util.h"
#include "include/config.h"

/* Bytes of each entry in the 'MazeCtx.rows' array */
#define COL_SZ 4

static void draw_rect(Image* img, int x, int y, int w, int h, uint64_t c) {
    for (int cur_y = y; cur_y < y + h; cur_y++) {
        /* To get the real position in the rows array, we need to multiply the
         * positions by the size of each element: COL_SZ (4) */
        for (int cur_x = x * COL_SZ; cur_x < (x + w) * COL_SZ;
             cur_x += COL_SZ) {
            /* Make sure we are not out of bounds */
            if (cur_x < 0 || cur_x >= img->img_w * 4 || cur_y < 0 ||
                cur_y >= img->img_h)
                continue;

            img->rows[cur_y][cur_x]     = (c >> 24) & 0xFF; /* r */
            img->rows[cur_y][cur_x + 1] = (c >> 16) & 0xFF; /* g */
            img->rows[cur_y][cur_x + 2] = (c >> 8) & 0xFF;  /* b */
            img->rows[cur_y][cur_x + 3] = c & 0xFF;         /* a */
        }
    }
}

static void maze_ctx_to_image(Image* img, const MazeCtx* maze) {
    /* Clear rows with background */
    draw_rect(img, 0, 0, img->img_w, img->img_h, COL_BACKGROUND);

    for (int y = 0; y < maze->grid_h; y++) {
        for (int x = 0; x < maze->grid_w; x++) {
            const int px_y   = y * CELL_SZ;
            const int px_x   = x * CELL_SZ;
            const int half_w = WALL_WIDTH / 2;

            if (maze->grid[maze->grid_w * y + x].walls & WALL_NORTH)
                draw_rect(img,
                          px_x - half_w,
                          px_y - half_w,
                          CELL_SZ + WALL_WIDTH,
                          WALL_WIDTH,
                          COL_WALL);

            if (maze->grid[maze->grid_w * y + x].walls & WALL_SOUTH)
                draw_rect(img,
                          px_x - half_w,
                          px_y + CELL_SZ - half_w,
                          CELL_SZ + WALL_WIDTH,
                          WALL_WIDTH,
                          COL_WALL);

            if (maze->grid[maze->grid_w * y + x].walls & WALL_WEST)
                draw_rect(img,
                          px_x - half_w,
                          px_y - half_w,
                          WALL_WIDTH,
                          CELL_SZ + WALL_WIDTH,
                          COL_WALL);

            if (maze->grid[maze->grid_w * y + x].walls & WALL_EAST)
                draw_rect(img,
                          px_x + CELL_SZ - half_w,
                          px_y - half_w,
                          WALL_WIDTH,
                          CELL_SZ + WALL_WIDTH,
                          COL_WALL);
        }
    }
}

static bool image_init(Image* img, const MazeCtx* maze) {
    img->img_w = maze->grid_w * CELL_SZ;
    img->img_h = maze->grid_h * CELL_SZ;

    img->rows = malloc(img->img_h * sizeof(png_bytep));
    if (img->rows == NULL)
        return false;

    for (int y = 0; y < img->img_h; y++) {
        img->rows[y] = malloc(img->img_w * COL_SZ);
        if (img->rows[y] == NULL)
            return false;
    }

    return true;
}

static void image_destroy(Image* img) {
    if (img->rows != NULL) {
        for (int y = 0; y < img->img_h; y++)
            free(img->rows[y]);
        free(img->rows);
        img->rows = NULL;
    }
}

/*----------------------------------------------------------------------------*/

bool write_png_from_maze_ctx(const MazeCtx* maze, const char* output_filename) {
    FILE* fd = fopen(output_filename, "wb");
    if (!fd) {
        ERR("Can't open file '%s': %s", output_filename, strerror(errno));
        return false;
    }

    png_structp png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL) {
        ERR("Can't create 'png_structp'.");
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (!info)
        DIE("Can't create 'png_infop'.");

    Image img;
    if (!image_init(&img, maze))
        return false;

    printf("Writing %dx%d file...\n", img.img_w, img.img_h);

    /* Specify the PNG info */
    png_init_io(png, fd);
    png_set_IHDR(png,
                 info,
                 img.img_w,
                 img.img_h,
                 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    /* Convert the grid to png */
    maze_ctx_to_image(&img, maze);

    /* Write the rows */
    png_write_image(png, img.rows);
    png_write_end(png, NULL);

    image_destroy(&img);
    png_destroy_write_struct(&png, &info);
    fclose(fd);

    return true;
}
