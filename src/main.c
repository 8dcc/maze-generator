
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <png.h>

#define COL_BACKGROUND 0x000000FF
#define COL_WALL       0xFFFFFFFF

#define CELL_SZ    10 /* px */
#define WALL_WIDTH 2  /* px */

#define BIAS_HORIZ 1 /* 1-N */
#define BIAS_VERT  1 /* 1-N */

/* Grid positions of entrance and exit of the mace */
#define START_X 0
#define START_Y 0
#define END_X   (ctx.w - 1)
#define END_Y   (ctx.h - 1)

#define COL_SZ 4 /* Bytes of each entry in ctx.rows[] */

enum EWalls {
    WALL_NORTH = (1 << 0),
    WALL_SOUTH = (1 << 1),
    WALL_WEST  = (1 << 2),
    WALL_EAST  = (1 << 3),
};

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

#define VEC(X, Y) ((vec2_t){ .x = (X), .y = (Y) })

typedef struct {
    int x, y;
} vec2_t;

typedef struct {
    uint32_t walls;
    bool visited;
} Cell;

typedef struct {
    const char* filename;
    png_bytep* rows;
    int px_w, px_h; /* Pixels */

    Cell* grid;
    int w, h; /* Cell number, not px */
    vec2_t* stack;
    int stack_pos;
} Context;

/*----------------------------------------------------------------------------*/

static Context ctx = {
    /* Png stuff */
    .filename = "output.png",
    .rows     = NULL,
    .px_w     = 100 * CELL_SZ,
    .px_h     = 100 * CELL_SZ,

    /* Grid stuff */
    .grid      = NULL,
    .w         = 100,
    .h         = 100,
    .stack     = NULL,
    .stack_pos = 0,
};

/*----------------------------------------------------------------------------*/

static bool parse_args(int argc, char** argv) {
    if (argc >= 2)
        ctx.filename = argv[1];

    if (argc >= 3)
        ctx.w = atoi(argv[2]);
    if (argc >= 4)
        ctx.h = atoi(argv[3]);

    if (ctx.w <= 0 || ctx.h <= 0) {
        ERR("Invalid grid size.");
        return false;
    }

    ctx.px_w = ctx.w * CELL_SZ;
    ctx.px_h = ctx.h * CELL_SZ;

    ctx.grid  = calloc(ctx.w * ctx.h, sizeof(Cell));
    ctx.stack = calloc(ctx.w * ctx.h, sizeof(vec2_t));

    return true;
}

static void stack_push(vec2_t v) {
    if (ctx.stack_pos < ctx.w * ctx.h)
        ctx.stack[ctx.stack_pos++] = v;
}

static vec2_t stack_pop(void) {
    if (ctx.stack_pos <= 0)
        return (vec2_t){ -1, -1 };

    return ctx.stack[--ctx.stack_pos];
}

static int random_unvisited_neighbour(vec2_t v) {
    const int x = v.x;
    const int y = v.y;

    int possible_walls[4 * BIAS_VERT * BIAS_HORIZ] = { 0 };
    int num                                        = 0;

    if (y >= 1 && ctx.grid[ctx.w * (y - 1) + x].visited == false)
        for (int i = 0; i < BIAS_VERT; i++)
            possible_walls[num++] = WALL_NORTH;
    if (y < ctx.h - 1 && ctx.grid[ctx.w * (y + 1) + x].visited == false)
        for (int i = 0; i < BIAS_VERT; i++)
            possible_walls[num++] = WALL_SOUTH;
    if (x >= 1 && ctx.grid[ctx.w * y + (x - 1)].visited == false)
        for (int i = 0; i < BIAS_HORIZ; i++)
            possible_walls[num++] = WALL_WEST;
    if (x < ctx.w - 1 && ctx.grid[ctx.w * y + (x + 1)].visited == false)
        for (int i = 0; i < BIAS_HORIZ; i++)
            possible_walls[num++] = WALL_EAST;

    if (num == 0)
        return 0;

    int random_pos = rand() % num;
    return possible_walls[random_pos];
}

static vec2_t pos_from_wall(vec2_t v, enum EWalls wall) {
    switch (wall) {
        case WALL_NORTH:
            return VEC(v.x, v.y - 1);
        case WALL_SOUTH:
            return VEC(v.x, v.y + 1);
        case WALL_WEST:
            return VEC(v.x - 1, v.y);
        case WALL_EAST:
            return VEC(v.x + 1, v.y);
        default:
            ERR("Invalid wall direction: %d", wall);
            return VEC(0, 0);
    }
}

static inline int opposite_wall(int wall) {
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
            return WALL_NORTH;
    }
}

static inline void remove_walls(vec2_t a, vec2_t b, int wall) {
    ctx.grid[ctx.w * a.y + a.x].walls &= ~wall;
    ctx.grid[ctx.w * b.y + b.x].walls &= ~opposite_wall(wall);
}

static void generate_maze(void) {
    /* Clear maze */
    for (int y = 0; y < ctx.h; y++) {
        for (int x = 0; x < ctx.w; x++) {
            ctx.grid[ctx.w * y + x].walls =
              WALL_NORTH | WALL_SOUTH | WALL_WEST | WALL_EAST;
            ctx.grid[ctx.w * y + x].visited = false;
        }
    }

    /* Push starting position (center) into the stack, and mark as visited */
    vec2_t cur_pos = VEC(ctx.w / 2, ctx.h / 2);
    stack_push(cur_pos);
    ctx.grid[ctx.w * cur_pos.y + cur_pos.x].visited = true;

    /* While we have positions left in the stack */
    for (;;) {
        /* Get next position from the stack */
        cur_pos = stack_pop();

        /* No more positions to check, we are done */
        if (cur_pos.x < 0 || cur_pos.y < 0)
            break;

        /* Get a random adjacent cell which has not been visited */
        int valid_neighbour_wall = random_unvisited_neighbour(cur_pos);

        /* Has valid neighbours */
        if (valid_neighbour_wall != 0) {
            /* Push current position */
            stack_push(cur_pos);

            /* Get position of neighbour from wall orientation */
            vec2_t neighbour = pos_from_wall(cur_pos, valid_neighbour_wall);

            if (neighbour.x < 0 || neighbour.x >= ctx.w || neighbour.y < 0 ||
                neighbour.y >= ctx.h)
                ERR("Warning: Neighbour out of bounds.");

            /* Remove the wall in the current cell and the random neighbour */
            remove_walls(cur_pos, neighbour, valid_neighbour_wall);

            /* Mark neighbour as visited and push to the stack */
            ctx.grid[ctx.w * neighbour.y + neighbour.x].visited = true;
            stack_push(neighbour);
        }
    }

    /* Remove walls of entry and exit */
    ctx.grid[ctx.w * START_Y + START_X].walls &= ~WALL_NORTH;
    ctx.grid[ctx.w * END_Y + END_X].walls &= ~WALL_SOUTH;
}

static void draw_rect(int x, int y, int w, int h, uint64_t c) {
    for (int cur_y = y; cur_y < y + h; cur_y++) {
        /* To get the real position in the rows array, we need to multiply the
         * positions by the size of each element: COL_SZ (4) */
        for (int cur_x = x * COL_SZ; cur_x < (x + w) * COL_SZ;
             cur_x += COL_SZ) {
            /* Make sure we are not out of bounds */
            if (cur_x < 0 || cur_x >= ctx.px_w * 4 || cur_y < 0 ||
                cur_y >= ctx.px_h)
                continue;

            ctx.rows[cur_y][cur_x]     = (c >> 24) & 0xFF; /* r */
            ctx.rows[cur_y][cur_x + 1] = (c >> 16) & 0xFF; /* g */
            ctx.rows[cur_y][cur_x + 2] = (c >> 8) & 0xFF;  /* b */
            ctx.rows[cur_y][cur_x + 3] = c & 0xFF;         /* a */
        }
    }
}

static void grid_to_rows(void) {
    /* Clear rows with background */
    draw_rect(0, 0, ctx.px_w, ctx.px_h, COL_BACKGROUND);

    for (int y = 0; y < ctx.h; y++) {
        for (int x = 0; x < ctx.w; x++) {
            const int px_y   = y * CELL_SZ;
            const int px_x   = x * CELL_SZ;
            const int half_w = WALL_WIDTH / 2;

            if (ctx.grid[ctx.w * y + x].walls & WALL_NORTH)
                draw_rect(px_x - half_w,
                          px_y - half_w,
                          CELL_SZ + WALL_WIDTH,
                          WALL_WIDTH,
                          COL_WALL);

            if (ctx.grid[ctx.w * y + x].walls & WALL_SOUTH)
                draw_rect(px_x - half_w,
                          px_y + CELL_SZ - half_w,
                          CELL_SZ + WALL_WIDTH,
                          WALL_WIDTH,
                          COL_WALL);

            if (ctx.grid[ctx.w * y + x].walls & WALL_WEST)
                draw_rect(px_x - half_w,
                          px_y - half_w,
                          WALL_WIDTH,
                          CELL_SZ + WALL_WIDTH,
                          COL_WALL);

            if (ctx.grid[ctx.w * y + x].walls & WALL_EAST)
                draw_rect(px_x + CELL_SZ - half_w,
                          px_y - half_w,
                          WALL_WIDTH,
                          CELL_SZ + WALL_WIDTH,
                          COL_WALL);
        }
    }
}

static void write_png(void) {
    FILE* fd = fopen(ctx.filename, "wb");
    if (!fd)
        DIE("Can't open file: \"%s\"", ctx.filename);

    png_structp png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
        DIE("Can't create png_structp");

    png_infop info = png_create_info_struct(png);
    if (!info)
        DIE("Can't create png_infop");

    /* Specify the PNG info */
    png_init_io(png, fd);
    png_set_IHDR(png,
                 info,
                 ctx.px_w,
                 ctx.px_h,
                 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    /* Allocate the rows here */
    ctx.rows = malloc(ctx.px_h * sizeof(png_bytep));
    for (int y = 0; y < ctx.px_h; y++)
        ctx.rows[y] = malloc(ctx.px_w * sizeof(uint8_t) * 4);

    /* Convert the grid to png */
    grid_to_rows();

    /* Write the rows */
    png_write_image(png, ctx.rows);
    png_write_end(png, NULL);

    /* Free each pointer of the rows pointer array */
    for (int y = 0; y < ctx.px_h; y++)
        free(ctx.rows[y]);

    /* And the array itself */
    free(ctx.rows);

    fclose(fd);
    png_destroy_write_struct(&png, &info);
}

int main(int argc, char** argv) {
    if (!parse_args(argc, argv)) {
        fprintf(stderr, "Usage: %s <output> <w> <h>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    printf("Generating %dx%d maze...\n", ctx.w, ctx.h);
    generate_maze();

    printf("Writing %dx%d file...\n", ctx.px_w, ctx.px_h);
    write_png();

    puts("Done.");
    free(ctx.grid);
    free(ctx.stack);
    return 0;
}
