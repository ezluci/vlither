#include "util.h"
#include <string.h>
#include <stdlib.h>
#include "../game/game.h"
#include <math.h>

uint8_t* reduce_skin(game* g) {
	uint8_t* reduced = ig_darray_create(uint8_t);
	uint8_t sequence_count = 0;

	for (int i = 0; i < g->settings_instance.exp_ptr; i++) {
		sequence_count++;

		uint8_t data_i = g->settings_instance.cusk_skin_data_exp[i];

		if (g->settings_instance.cusk_skin_data_exp[i + 1] != data_i) {
			ig_darray_push(&reduced, &sequence_count);
			ig_darray_push(&reduced, &data_i);
			sequence_count = 0;
		}
	}
	return reduced;
}

uint8_t* make_nickname_skin_data(game* g, int* nickname_skin_data_len) {
	int nickname_len = (int) strlen(g->settings_instance.nickname);

	uint8_t* reduced_skin_data = reduce_skin(g);
	int reduced_length = g->settings_instance.cusk ? ig_darray_length(reduced_skin_data) : 0;
	uint8_t* result = malloc(*nickname_skin_data_len = 28 + nickname_len);

	const int client_version = 291;
	int cpw[] = {54, 206, 204, 169, 97, 178, 74, 136, 124, 117, 14, 210, 106, 236, 8, 208, 136, 213, 140, 111};

	result[0] = 115;
	result[1] = 30;
	result[2] = client_version >> 8 & 255;
	result[3] = client_version & 255;
	for (int i = 0; i < 20; ++i)
		result[i+4] = cpw[i];
	result[24] = g->settings_instance.cv;
	result[25] = nickname_len;

	int j = 26;
	for (int i = 0; i < nickname_len; i++) {
		result[j++] = (uint8_t) g->settings_instance.nickname[i];
	}

	result[j++] = 0;
	result[j++] = 255;

	// custom skin data:
	// result[j++] = 255;
	// result[j++] = 255;
	// result[j++] = 255;
	// result[j++] = 0;
	// result[j++] = 0;
	// result[j++] = 0;
	// result[j++] = (int) floorf(rand() % 256);
	// result[j++] = (int) floorf(rand() % 256);

	// for (int i = 0; i < reduced_length; i++) {
	// 	result[j++] = reduced_skin_data[i];
	// }
	
	ig_darray_destroy(reduced_skin_data);

	return result;
}

void set_mscps_fmlts_fpsls(game* g) {
	for (int i = 0; i <= g->config.mscps; i++) {
		if (i >= g->config.mscps) ig_darray_push(&(g->config.fmlts), &(g->config.fmlts[i - 1]));
		else ig_darray_push(&(g->config.fmlts), (float[]) { (powf(1 - (i / (float) (g->config.mscps)), 2.25f)) });
		if (i == 0) ig_darray_push(&(g->config.fpsls), (float[]) { 0 });
		else ig_darray_push(&(g->config.fpsls), (float[]) { g->config.fpsls[i - 1] + 1 / g->config.fmlts[i - 1] });
	}
	float t_fmlt = g->config.fmlts[ig_darray_length(g->config.fmlts) - 1];
	float t_fpsl = g->config.fpsls[ig_darray_length(g->config.fpsls) - 1];
	for (int i = 0; i < 2048; i++) {
		ig_darray_push(&(g->config.fmlts), &t_fmlt);
		ig_darray_push(&(g->config.fpsls), &t_fpsl);
	}
}

void recalcSepMults(game* g) {
	int n = 0;
	int k = 3;
	int mv = 0;
	for (int i = 0; i < SMUC; ++i) {
		if (i < k) {
			g->config.smus[i] = 1;
		} else {
			n++;
			if (n <= 4) {
				mv = g->config.cst * n / 4;
			}
			g->config.smus[i] = 1 - mv;
		}
	}
}

void reset_game(game* g) {
	// glfwSetTime(0);

	// g->config.last_ping_mtm = 0;
	// g->config.last_accel_mtm = 0;
	// g->config.last_e_mtm = 0;

	for (int i = 0; i < 10; i++) {
		strcpy(g->leaderboard[i].nk, "LOADING...");
		g->leaderboard[i].cv = 11;
		g->leaderboard[i].score = 0;
	}

	g->config.wfpr = 0;
	for (int j = VFC - 1; j >= 0; j--) {
		g->config.fvxs[j] = 0;
		g->config.fvys[j] = 0;
	}
	g->config.fvtg = 0;
	g->config.fvx = 0;
	g->config.fvy = 0;
	g->config.lag_mult = 1;
	g->network_done = 0;
	g->config.my_pos = 0;
	g->config.rank = 0;
	g->config.total_players = 0;

	snake_map_clear(&g->os);
	ig_darray_clear(g->foods);
	ig_darray_clear(g->preys);
	memset(g->config.mmdata, 0, 136 * 136);
}

void save_settings(game* g) {
	FILE *file = fopen("settings.dat", "wb");
	if (!file) {
		perror("Failed to open settings file for writing");
		return;
	}
	fwrite(&g->settings_instance, sizeof(settings), 1, file);
	fclose(file);

	g->config.qsm = g->settings_instance.hq ? 1.0f : 1.7f;
}

void load_settings(game* g) {
	FILE* file = fopen("settings.dat", "rb");
	if (!file) {
		return;
	}
	fread(&g->settings_instance, sizeof(settings), 1, file);
	fclose(file);

	g->config.qsm = g->settings_instance.hq ? 1.0f : 1.7f;
}
