#ifndef SNAKE_H
#define SNAKE_H

#include <ignite.h>
#include <stdbool.h>

#define LFC 53
#define RFC 53
#define HFC 53
#define AFC 26
#define VFC 62

typedef struct game game;

typedef struct body_part {
	float xx;
	float yy;
	float fx;
	float fy;
	float iang;
	float ebx;
	float eby;
	float fltn;
	float da;
	float ltn;
	int fpos;
	int ftg;
	float fsmu;
	float smu;
	int dying;
	float fxs[HFC];
	float fys[HFC];
	float fltns[HFC];
	float fsmus[HFC];
} body_part;

typedef struct gpt_struct {
	float d;
	float xx;
	float yy;
	float ox;
	float oy;
} gpt_struct;

typedef struct snake {
	int id;
	int sct;
	bool iiv;

	int skin_data_len;
	uint8_t* skin_data;
	bool cusk;

	float ang;
	float dir;
	float wang;
	float sp;
	float fam;
	float xx;
	float yy;
	float fx;
	float fy;
	float rex;
	float rey;
	float eang;
	float spang;
	float sc;
	float scang;
	float ssp;
	float fsp;
	float tsp;
	float sep;
	float sfr;
	float wsep;
	float tl;
	float chl;
	float ehl;
	float wehang;
	float ehang;
	float fchl;
	int edir;
	body_part* pts;
	gpt_struct* gptz;

	float fxs[RFC];
	float fys[RFC];
	float fchls[RFC];
	float fls[LFC];
	float fas[AFC];
	float cfl;
	float fl;
	float fa;
	int flpos;
	int fltg;
	int fpos;
	int fapos;
	int fatg;
	int ftg;
	char nk[25];

	bool dead;
	float dead_amt;
	float alive_amt;
	int kill_count;
} snake;

void snake_update_length(snake* snake, game* g);

// returns an index instead of a pointer
size_t arp(snake* o, int q, float xx, float yy);

#endif