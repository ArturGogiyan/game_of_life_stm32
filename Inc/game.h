#ifndef __GAME__
#define __GAME__

/*
 * L_CELL must be greater than D_CELL
 * else you must rewrite calc_step() in field.c
 */
#define D_CELL 32 /*dead cell (ascii space)*/
#define L_CELL 35 /*living cell (ascii #)*/ 

// todo: configure this
#define WIDTH 64
#define HEIGHT 32

typedef struct {
	char plane[HEIGHT][WIDTH];
	char buff[HEIGHT][WIDTH];
	int n;
	int m;
} field_t;

/* initializer */
void init_field(field_t* field, int n, int m);

/* funcs */
void calc_step(field_t* field);

#endif