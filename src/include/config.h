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

#ifndef CONFIG_H_
#define CONFIG_H_ 1

/*
 * TODO: Move these macros to program arguments.
 */

#define COL_BACKGROUND 0x000000FF
#define COL_WALL       0xFFFFFFFF

#define CELL_SZ    10 /* px */
#define WALL_WIDTH 2  /* px */

#define BIAS_HORIZ 1 /* 1-N */
#define BIAS_VERT  1 /* 1-N */

/*
 * Grid positions of entrance and exit of the mace.
 *
 * FIXME: Variables in macros... Not good.
 */
#define START_X 0
#define START_Y 0
#define END_X   (ctx->grid_w - 1)
#define END_Y   (ctx->grid_h - 1)

#endif /* CONFIG_H_ */
