
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>

#define COL_BACKGROUND 0x000000FF
#define COL_WALL       0xFFFFFFFF

#define CELL_SZ    10 /* px */
#define WALL_WIDTH 2  /* px */

#define COL_SZ 4 /* Bytes of each entry in ctx.rows[] */

#define WALL_NORTH (1 << 0)
#define WALL_SOUTH (1 << 1)
#define WALL_WEST  (1 << 2)
#define WALL_EAST  (1 << 3)

#define DIE(...)                      \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        putc('\n', stderr);           \
        exit(1);                      \
    }

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
} Context;

/*----------------------------------------------------------------------------*/

static Context ctx = {
    /* Png stuff */
    .filename = "output.png",
    .rows     = NULL,
    .px_w     = 100 * CELL_SZ,
    .px_h     = 100 * CELL_SZ,

    /* Grid stuff */
    .grid = NULL,
    .w    = 100,
    .h    = 100,
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
        fprintf(stderr, "Invalid grid size.\n");
        return false;
    }

    ctx.px_w = ctx.w * CELL_SZ;
    ctx.px_h = ctx.h * CELL_SZ;

    ctx.grid = calloc(ctx.w * ctx.h, sizeof(Cell));

    return true;
}

static void draw_rect(int x, int y, int w, int h, uint64_t c) {
    for (int cur_y = y; cur_y < y + h; cur_y++) {
        /* To get the real position in the rows array, we need to multiply the
         * positions by the size of each element: COL_SZ (4) */
        for (int cur_x = x * COL_SZ; cur_x < (x + w) * COL_SZ;
             cur_x += COL_SZ) {
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
            int px_y = y * CELL_SZ;
            int px_x = x * CELL_SZ;

            if (ctx.grid[ctx.w * y + x].walls & WALL_NORTH)
                draw_rect(px_x, px_y, CELL_SZ, WALL_WIDTH, COL_WALL);

            if (ctx.grid[ctx.w * y + x].walls & WALL_SOUTH)
                draw_rect(px_x, px_y + CELL_SZ, CELL_SZ + WALL_WIDTH,
                          WALL_WIDTH, COL_WALL);

            if (ctx.grid[ctx.w * y + x].walls & WALL_WEST)
                draw_rect(px_x, px_y, WALL_WIDTH, CELL_SZ, COL_WALL);

            if (ctx.grid[ctx.w * y + x].walls & WALL_EAST)
                draw_rect(px_x + CELL_SZ, px_y, WALL_WIDTH,
                          CELL_SZ + WALL_WIDTH, COL_WALL);
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
    png_set_IHDR(png, info, ctx.px_w, ctx.px_h, 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
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

    // TODO: Generate maze

    // DELME
    ctx.grid[ctx.w * 5 + 3].walls |= WALL_NORTH;
    ctx.grid[ctx.w * 5 + 3].walls |= WALL_SOUTH;
    ctx.grid[ctx.w * 5 + 3].walls |= WALL_WEST;
    ctx.grid[ctx.w * 5 + 3].walls |= WALL_EAST;

    printf("Writing %dx%d file...\n", ctx.px_w, ctx.px_h);
    write_png();

    puts("Done.");
    free(ctx.grid);
    return 0;
}
