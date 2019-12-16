#include "game.h"

static void init_plane(field_t *field)
{
    for (int i = 0; i < field->n; ++i)
    {
        for (int j = 0; j < field->m; ++j)
        {
            field->plane[i][j] = D_CELL;
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

void init_field(field_t* field, int n, int m)
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
