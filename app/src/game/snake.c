#include "snake.h"
#include "game.h"
#include "../networking/util.h"
#include "food.h"

void snake_update_length(snake* snake, game* g) {
	float orl = snake->tl;
	snake->tl = snake->sct + fminf(1, snake->fam);
	float d = snake->tl - orl;
	int k = snake->flpos;
	for (int j = 0; j < LFC; j++) {
		snake->fls[k] -= d * g->config.lfas[j];
		k++;
		if (k >= LFC) k = 0;
	}
	snake->fl = snake->fls[snake->flpos];
	snake->fltg = LFC;
}

gpt_struct* arp(snake* o, int q, float xx, float yy)
{
	int gptz_len = ig_darray_length(o->gptz);
	if (q < gptz_len) {
		gpt_struct* gpo = o->gptz + q;
		gpo->xx = xx;
		gpo->yy = yy;
		return gpo;
	} else {
		ig_darray_push(&o->gptz, (&(gpt_struct) {
			.xx = xx,
			.yy = yy,
			.ox = 0,
			.oy = 0,
			.d = 0
		}));
		return o->gptz + gptz_len;
	}
}