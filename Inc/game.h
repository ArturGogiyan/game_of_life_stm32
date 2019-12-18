#ifndef __GAME__
#define __GAME__

/*
 * L_CELL must be greater than D_CELL
 * else you must rewrite calc_step() in game.c
 * 
 */
#define D_CELL 32 /*dead cell (ascii space)*/
#define L_CELL 35 /*living cell (ascii #)*/

// display is 128x64, and one cell is 4x4 so..
#define WIDTH 31
#define HEIGHT 15

#define SAVES_COUNT 10

typedef struct
{
	char plane[HEIGHT][WIDTH];
	char buff[HEIGHT][WIDTH];
	int n;
	int m;
} field_t;

typedef struct
{
    field_t saves[SAVES_COUNT];
    int current_saves_count;
} saves_t;

/* initializer */
void init_field(field_t *field, int n, int m);

/* funcs */
void calc_step(field_t *field);

void init_default_fields(saves_t* saves);
void set_letter(char letter, field_t* field, int x, int y);

#endif
