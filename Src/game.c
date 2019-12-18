#include "game.h"

static void init_plane(field_t *field)
{
    for (int i = 0; i < field->n; ++i)
    {
        for (int j = 0; j < field->m; ++j)
        {
            field->plane[i][j] = D_CELL;
            field->buff[i][j] = D_CELL;
        }
    }
}

static void swap_planes(field_t *field)
{
    for (int i = 0; i < field->n; ++i)
    {
        for (int j = 0; j < field->m; ++j)
        {
            field->plane[i][j] = field->plane[i][j] ^ field->buff[i][j];
            field->buff[i][j] = field->plane[i][j] ^ field->buff[i][j];
            field->plane[i][j] = field->plane[i][j] ^ field->buff[i][j];
        }
    }
}

void init_field(field_t *field, int n, int m)
{
    field->n = n;
    field->m = m;
    init_plane(field);
}

void calc_step(field_t *field)
{
    int neighbor_count;
    int three_neighbor = L_CELL * 3 + D_CELL * 5;
    int two_neighbor = L_CELL * 2 + D_CELL * 6;
    for (int i = field->n; i < field->n * 2; ++i)
    {
        for (int j = field->m; j < field->m * 2; ++j)
        {
            neighbor_count = field->plane[(i - 1) % field->n][(j - 1) % field->m] +
                             +field->plane[i % field->n][(j - 1) % field->m] +
                             +field->plane[(i + 1) % field->n][(j - 1) % field->m] +
                             +field->plane[(i - 1) % field->n][j % field->m] +
                             +field->plane[(i + 1) % field->n][j % field->m] +
                             +field->plane[(i - 1) % field->n][(j + 1) % field->m] +
                             +field->plane[i % field->n][(j + 1) % field->m] +
                             +field->plane[(i + 1) % field->n][(j + 1) % field->m];

            switch (field->plane[i % field->n][j % field->m])
            {
            case D_CELL:
                field->buff[i % field->n][j % field->m] = D_CELL;
                if (neighbor_count == three_neighbor)
                    field->buff[i % field->n][j % field->m] = L_CELL;
                break;
            case L_CELL:
                field->buff[i % field->n][j % field->m] = L_CELL;
                if (neighbor_count > three_neighbor)
                    field->buff[i % field->n][j % field->m] = D_CELL;
                if (neighbor_count < two_neighbor)
                    field->buff[i % field->n][j % field->m] = D_CELL;
                break;
            }
        }
    }
    swap_planes(field);
}

void init_default_fields(saves_t* saves)
{

}

void set_letter(char letter, field_t* field, int x, int y)
{
	switch(letter) {
	case 'A':
	case 'a':
		field->plane[y+1][x] = L_CELL;
		field->plane[y+2][x] = L_CELL;
		field->plane[y+3][x] = L_CELL;
		field->plane[y+4][x] = L_CELL;
		field->plane[y+5][x] = L_CELL;

		field->plane[y][x+1] = L_CELL;
		field->plane[y+3][x+1] = L_CELL;

		field->plane[y+1][x+2] = L_CELL;
		field->plane[y+2][x+2] = L_CELL;
		field->plane[y+3][x+2] = L_CELL;
		field->plane[y+4][x+2] = L_CELL;
		field->plane[y+5][x+2] = L_CELL;
		break;
	case 'E':
	case 'e':
		field->plane[y][x] = L_CELL;
		field->plane[y+1][x] = L_CELL;
		field->plane[y+2][x] = L_CELL;
		field->plane[y+3][x] = L_CELL;
		field->plane[y+4][x] = L_CELL;
		field->plane[y+5][x] = L_CELL;

		field->plane[y][x+1] = L_CELL;
		field->plane[y+3][x+1] = L_CELL;
		field->plane[y+5][x+1] = L_CELL;

		field->plane[y][x+2] = L_CELL;
		field->plane[y+1][x+2] = L_CELL;
		field->plane[y+2][x+2] = L_CELL;
		field->plane[y+3][x+2] = L_CELL;
		field->plane[y+5][x+2] = L_CELL;


		break;
	case 'I':
	case 'i':
		field->plane[y][x] = L_CELL;
		field->plane[y+5][x] = L_CELL;

		field->plane[y][x+1] = L_CELL;
		field->plane[y+1][x+1] = L_CELL;
		field->plane[y+2][x+1] = L_CELL;
		field->plane[y+3][x+1] = L_CELL;
		field->plane[y+4][x+1] = L_CELL;
		field->plane[y+5][x+1] = L_CELL;

		field->plane[y][x+2] = L_CELL;
		field->plane[y+5][x+2] = L_CELL;
		break;
	case 'K':
	case 'k':
		field->plane[y][x] = L_CELL;
		field->plane[y+1][x] = L_CELL;
		field->plane[y+2][x] = L_CELL;
		field->plane[y+3][x] = L_CELL;
		field->plane[y+4][x] = L_CELL;
		field->plane[y+5][x] = L_CELL;

		field->plane[y+3][x+1] = L_CELL;

		field->plane[y][x+2] = L_CELL;
		field->plane[y+1][x+2] = L_CELL;
		field->plane[y+2][x+2] = L_CELL;
		field->plane[y+4][x+2] = L_CELL;
		field->plane[y+5][x+2] = L_CELL;
		break;
	case 'L':
	case 'l':
		field->plane[y][x] = L_CELL;
		field->plane[y+1][x] = L_CELL;
		field->plane[y+2][x] = L_CELL;
		field->plane[y+3][x] = L_CELL;
		field->plane[y+4][x] = L_CELL;
		field->plane[y+5][x] = L_CELL;

		field->plane[y+5][x+1] = L_CELL;
		field->plane[y+5][x+2] = L_CELL;
		break;
	case 'R':
	case 'r':
		field->plane[y][x] = L_CELL;
		field->plane[y+1][x] = L_CELL;
		field->plane[y+2][x] = L_CELL;
		field->plane[y+3][x] = L_CELL;
		field->plane[y+4][x] = L_CELL;
		field->plane[y+5][x] = L_CELL;

		field->plane[y][x+1] = L_CELL;
		field->plane[y+3][x+1] = L_CELL;

		field->plane[y+1][x+2] = L_CELL;
		field->plane[y+2][x+2] = L_CELL;
		field->plane[y+4][x+2] = L_CELL;
		field->plane[y+5][x+2] = L_CELL;
		break;
	case 'T':
	case 't':
		field->plane[y][x] = L_CELL;

		field->plane[y][x+1] = L_CELL;
		field->plane[y+1][x+1] = L_CELL;
		field->plane[y+2][x+1] = L_CELL;
		field->plane[y+3][x+1] = L_CELL;
		field->plane[y+4][x+1] = L_CELL;
		field->plane[y+5][x+1] = L_CELL;

		field->plane[y][x+2] = L_CELL;
		break;
	case 'U':
	case 'u':
	case 'V':
	case 'v':
		field->plane[y][x] = L_CELL;
		field->plane[y+1][x] = L_CELL;
		field->plane[y+2][x] = L_CELL;
		field->plane[y+3][x] = L_CELL;
		field->plane[y+4][x] = L_CELL;

		field->plane[y+5][x+1] = L_CELL;

		field->plane[y][x+2] = L_CELL;
		field->plane[y+1][x+2] = L_CELL;
		field->plane[y+2][x+2] = L_CELL;
		field->plane[y+3][x+2] = L_CELL;
		field->plane[y+4][x+2] = L_CELL;
		break;
	case 'X':
	case 'x':
		field->plane[y][x] = L_CELL;
		field->plane[y+1][x] = L_CELL;
		field->plane[y+4][x] = L_CELL;
		field->plane[y+5][x] = L_CELL;

		field->plane[y+2][x+1] = L_CELL;
		field->plane[y+3][x+1] = L_CELL;

		field->plane[y][x+2] = L_CELL;
		field->plane[y+1][x+2] = L_CELL;
		field->plane[y+4][x+2] = L_CELL;
		field->plane[y+5][x+2] = L_CELL;
		break;
	}

}

