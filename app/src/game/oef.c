#include "oef.h"
#include <math.h>
#include "../game/game.h"
#include "../game/prey.h"
#include "../networking/util.h"

void oef(game* g, struct mg_connection* c, const input_data* input_data) {
	g->config.vfr = (input_data->ctm - g->config.ltm) / 8;
	// if (g->config.vfr > 5) g->config.vfr = 5;
	// if (g->config.vfr < 0) g->config.vfr = 0;
	// printf("time = %.2f\n", input_data->ctm);
	// printf("vfr = %.2f\n", g->config.vfr);

	g->config.avfr = g->config.vfr;
	g->config.ltm = input_data->ctm;

	if (!g->config.lagging)
		if (g->config.wfpr && input_data->ctm - g->config.last_ping_mtm > 750) {
			g->config.lagging = 1;
		}
	if (g->config.lagging) {
		g->config.lag_mult *= 0.85f;
		if (g->config.lag_mult < 0.2f)	g->config.lag_mult = 0.2f;
	} else if (g->config.lag_mult < 1) {
		g->config.lag_mult += 0.05f;
		if (g->config.lag_mult >= 1)	g->config.lag_mult = 1.0f;
	}

	if (g->config.vfr > 120) g->config.vfr = 120;
	g->config.vfr *= g->config.lag_mult;
	g->config.lfr = g->config.fr;
	g->config.fr += g->config.vfr;
	g->config.vfrb = floorf(g->config.fr) - floorf(g->config.lfr);
	g->config.lfr2 = g->config.fr2;
	g->config.fr2 += g->config.vfr * 2;
	g->config.vfrb2 = floorf(g->config.fr2) - floorf(g->config.lfr2);
	g->config.afr += g->config.avfr;

	if (!g->config.wfpr) {
		if (input_data->ctm - g->config.last_ping_mtm > 250) {
			g->config.last_ping_mtm = input_data->ctm;
			g->config.wfpr = true;
			uint8_t ba[1] = { 251 };
			mg_ws_send(c, ba, 1, WEBSOCKET_OP_BINARY);
			// printf("ping\n");
		}
	}

	if (g->config.vfrb > 0) {
		if (g->config.flx_tg > 0) {
			int ki = g->config.vfrb;
			if (ki > g->config.flx_tg)	ki = g->config.flx_tg;
			g->config.flx_tg -= ki;
			for (int qq = 1; qq <= ki; qq++) {
				if (qq == ki)	g->config.flux_grd = g->config.flux_grds[g->config.flux_grd_pos];
				g->config.flux_grds[g->config.flux_grd_pos] = g->config.real_flux_grd;
				g->config.flux_grd_pos++;
				if (g->config.flux_grd_pos >= FLXC)	g->config.flux_grd_pos = 0;
         }
		}
		else if (g->config.flx_tg == 0)	g->config.flx_tg = -1;
	}

	if (g->settings_instance.enable_zoom) {
		if (input_data->m_pressed) {
			g->config.zoom -= 2 * 0.1f * g->config.gsc;
		} else if (input_data->n_pressed) {
			g->config.zoom += 2 * 0.1f * g->config.gsc;
		}

		g->config.zoom += input_data->mouse_dwheel * 0.1f * g->config.gsc;
		g->config.zoom = fmaxf(0.1f, fminf(g->config.zoom, 8));

		g->config.gsc += (g->config.zoom - g->config.gsc) * 0.09f * g->config.vfr;
	}

	if (!g->snake_null) {
		if (input_data->k_pressed) {
			g->config.assist = !g->config.assist;
		} else if (input_data->b_pressed) {
			g->config.big_food = !g->config.big_food;
		} else if (input_data->p_pressed) {
			g->config.player_names = !g->config.player_names;
		} else if (input_data->s_pressed) {
			g->config.shadow = !g->config.shadow;
		} else if (input_data->h_pressed) {
			g->config.show_hud = !g->config.show_hud;
		} else if (input_data->g_pressed) {
			g->config.show_boost = !g->config.show_boost;
		} else if (input_data->nine_pressed) {
			g->network_done = 1;
		} else if (input_data->zero_pressed) {
			g->respawn = true;
			g->network_done = 1;
		}

		g->config.wmd = input_data->btn_down;

		if (g->config.md != g->config.wmd && input_data->ctm - g->config.last_accel_mtm > 150) {
			g->config.md = g->config.wmd;
			g->config.last_accel_mtm = input_data->ctm;
			uint8_t ba[1] = { g->config.md ? 253 : 254 };
			mg_ws_send(c, ba, 1, WEBSOCKET_OP_BINARY);
		}

		int want_e = 0;
		float ang = 0;
		if (input_data->mouse_delta.x != 0 || input_data->mouse_delta.y != 0) want_e = 1;
		float xm = input_data->mouse_pos.x - g->icontext->default_frame.resolution.x / 2;
		float ym = input_data->mouse_pos.y - g->icontext->default_frame.resolution.y / 2;
		g->os.snakes[0].eang = atan2f(ym, xm);

		if (want_e && input_data->ctm - g->config.last_e_mtm > 50) {
			want_e = 0;
			g->config.last_e_mtm = input_data->ctm;
			float d2 = xm * xm + ym * ym;
			if (d2 > 256) {
				ang = atan2f(ym, xm);
				g->os.snakes[0].eang = ang;
			}
			else ang = g->os.snakes[0].wang;
			ang = fmodf(ang, X2PI);
			if (ang < 0) ang += X2PI;
			int sang = floorf((250 + 1) * ang / X2PI);
			if (sang != g->config.lsang) {
				g->config.lsang = sang;
				mg_ws_send(c, &sang, 1, WEBSOCKET_OP_BINARY);
			}
		}
	}

	float mang, vang, tang;

	int snakes_len = ig_darray_length(g->os.snakes);
	for (int i = snakes_len - 1; i >= 0; i--) {
		snake* o = g->os.snakes + i;
		mang = g->config.mamu * g->config.vfr * o->scang * o->spang;
		float csp = o->sp * g->config.vfr / 4.0f;
		if (csp > o->msl) csp = o->msl;
		
		if (!o->dead) {
			if (o->tsp != o->sp) {
				if (o->tsp < o->sp) {
					o->tsp += (o->sp - o->tsp) * .1f;
					o->tsp += 1e-4;
					if (o->tsp > o->sp) o->tsp = o->sp;
				} else {
					o->tsp += (o->sp - o->tsp) * .3f;
					o->tsp -= 1e-4;
					if (o->tsp < o->sp) o->tsp = o->sp;
				}
			}
			if (o->tsp > o->fsp) o->sfr += (o->tsp - o->fsp) * g->config.vfr * 0.021f;
			if (o->fltg > 0) {
				int k = g->config.vfrb; // invisible tail fix - no
				if (k > o->fltg) k = o->fltg;
				o->fltg -= k;
				for (int qq = 0; qq < k; qq++) {
					o->fl = o->fls[o->flpos];
					o->fls[o->flpos] = 0;
					o->flpos++;
					if (o->flpos >= LFC) o->flpos = 0;
				}
			}
			else if (o->fltg == 0) {
				o->fltg = -1;
				o->fl = 0;
			}
			o->cfl = o->tl + o->fl - .6f;
		}
		if (o->dir == 1) {
			o->ang -= mang;
			if (o->ang < 0 || o->ang >= X2PI) o->ang = fmodf(o->ang, X2PI);
			if (o->ang < 0) o->ang += X2PI;
			vang = fmodf(o->wang - o->ang, X2PI);
			if (vang < 0) vang += X2PI;
			if (vang > PI) vang -= X2PI;
			if (vang > 0) {
				o->ang = o->wang;
				o->dir = 0;
			}
		} else if (o->dir == 2) {
			o->ang += mang;
			if (o->ang < 0 || o->ang >= X2PI) o->ang = fmodf(o->ang, X2PI);
			if (o->ang < 0) o->ang += X2PI;
			vang = fmodf(o->wang - o->ang, X2PI);
			if (vang < 0) vang += X2PI;
			if (vang > PI) vang -= X2PI;
			if (vang < 0) {
				o->ang = o->wang;
				o->dir = 0;
			}
		} else o->ang = o->wang;
		if (o->ehl != 1) {
			o->ehl += 0.03f * g->config.vfr;
			if (o->ehl >= 1) o->ehl = 1;
		}
		// int pts_len = ig_darray_length(o->pts);
		// body_part* po = o->pts + pts_len - 1;
		// o->wehang = atan2f(o->yy + o->fy - po->yy - po->fy + po->eby * (1 - o->ehl), o->xx + o->fx - po->xx - po->fx + po->ebx * (1 - o->ehl));
		// if (o == g->os.snakes)
			// printf("wehang = %.2f\n", po->eby);

		if (!o->dead) {
			if (o->ehang != o->wehang) {
				vang = fmodf(o->wehang - o->ehang, X2PI);
				if (vang < 0) vang += X2PI;
				if (vang > PI) vang -= X2PI;
				if (vang < 0) o->edir = 1;
				else if (vang > 0) o->edir = 2;
			}
		}
		if (o->edir == 1) {
			tang = fmodf(o->wehang - o->ehang, X2PI);
			if (tang < 0)	tang += X2PI;
			if (tang > PI)	tang -= X2PI;
			o->ehang += tang * g->config.p12[g->config.vfrb];
			if (o->ehang < 0 || o->ehang >= X2PI) o->ehang = fmodf(o->ehang, X2PI);
			if (o->ehang < 0) o->ehang += X2PI;
			vang = fmodf(o->wehang - o->ehang, X2PI);
			if (vang < 0) vang += X2PI;
			if (vang > PI) vang -= X2PI;
			if (vang > 0) {
				o->ehang = o->wehang;
				o->edir = 0;
			}
		} else if (o->edir == 2) {
			tang = fmodf(o->wehang - o->ehang, X2PI);
			if (tang < 0)	tang += X2PI;
			if (tang > PI)	tang -= X2PI;
			o->ehang += tang * g->config.p12[g->config.vfrb];
			if (o->ehang < 0 || o->ehang >= X2PI) o->ehang = fmodf(o->ehang, X2PI);
			if (o->ehang < 0) o->ehang += X2PI;
			vang = fmodf(o->wehang - o->ehang, X2PI);
			if (vang < 0) vang += X2PI;
			if (vang > PI) vang -= X2PI;
			if (vang < 0) {
				o->ehang = o->wehang;
				o->edir = 0;
			}
		}

		if (!o->dead) {
			o->xx += cosf(o->ang) * csp;
			o->yy += sinf(o->ang) * csp;
			o->chl += csp / o->msl;
		}

		if (g->config.vfrb > 0) {
			int k = 0;
			int pts_len = ig_darray_length(o->pts);
			body_part* po;
			for (int j = pts_len - 1; j >= 0; j--) {
				po = o->pts + j;
				if (po->dying) {
					k++;
					po->da += .0015f * g->config.vfrb;
					if (po->da >= 1) {
						po->da = 1;
						if (k >= 4) {
							po->dying = false;
							ig_darray_remove(o->pts, j);
						}
					}
				}
			}
			pts_len = ig_darray_length(o->pts);
			for (int j = pts_len - 1; j >= 0; j--) {
				po = o->pts + j;
				if (po->ftg > 0) {
					int k = g->config.vfrb;
					if (k > po->ftg)	k = po->ftg;
					po->ftg -= k;
					for (int qq = 0; qq < k; ++qq) {
						po->fx = po->fxs[po->fpos];
						po->fy = po->fys[po->fpos];
						po->fltn = po->fltns[po->fpos];
						po->fsmu = po->fsmus[po->fpos];
						po->fxs[po->fpos] = 0;
						po->fys[po->fpos] = 0;
						po->fltns[po->fpos] = 0;
						po->fsmus[po->fpos] = 0;
						po->fpos++;
						if (po->fpos >= HFC)	po->fpos = 0;
					}
				} else if (po->ftg == 0) {
					po->ftg = -1;
					po->fx = 0;
					po->fy = 0;
					po->fltn = 0;
					po->fsmu = 0;
				}
			}
		}

		// eyes
		float wx = cosf(o->eang) * 2.3f; // pma
		float wy = sinf(o->eang) * 2.3f;
		if (o->rex < wx) {
			o->rex += g->config.vfr / 6.0f;
			if (o->rex >= wx) o->rex = wx;
		}
		if (o->rey < wy) {
			o->rey += g->config.vfr / 6.0f;
			if (o->rey >= wy) o->rey = wy;
		}
		if (o->rex > wx) {
			o->rex -= g->config.vfr / 6.0f;
			if (o->rex <= wx) o->rex = wx;
		}
		if (o->rey > wy) {
			o->rey -= g->config.vfr / 6.0f;
			if (o->rey <= wy) o->rey = wy;
		}

		if (g->config.vfrb > 0) {
			if (o->ftg > 0) {
				int k = g->config.vfrb;
				if (k > o->ftg) k = o->ftg;
				o->ftg -= k;
				for (int qq = 0; qq < k; qq++) {
					o->fx = o->fxs[o->fpos];
					o->fy = o->fys[o->fpos];
					o->fchl = o->fchls[o->fpos];
					o->fxs[o->fpos] = 0;
					o->fys[o->fpos] = 0;
					o->fchls[o->fpos] = 0;
					o->fpos++;
					if (o->fpos >= RFC) o->fpos = 0;
				}
			} else if (o->ftg == 0) {
				o->ftg = -1;
				o->fx = 0;
				o->fy = 0;
				o->fchl = 0;
			}
			if (o->fatg > 0) {
				int k = g->config.vfrb;
				if (k > o->fatg) k = o->fatg;
				o->fatg -= k;
				for (int qq = 0; qq < k; qq++) {
					o->fa = o->fas[o->fapos];
					o->fas[o->fapos] = 0;
					o->fapos++;
					if (o->fapos >= AFC) o->fapos = 0;
				}
			} else if (o->fatg == 0) {
				o->fatg = -1;
				o->fa = 0;
			}
		}

		if (o->dead) {
			o->dead_amt += g->config.vfr * 0.02f;
			if (o->dead_amt >= 1) {
				if (i == 0 && !g->snake_null) {
					g->snake_null = 1;
				}
				snake_map_remove_idx(&g->os, i);
			}
		} else if (o->alive_amt != 1) {
			o->alive_amt += g->config.vfr * 0.015f;
			if (o->alive_amt >= 1) o->alive_amt = 1;
		}
	}

	int preys_len = ig_darray_length(g->preys);
	for (int i = preys_len - 1; i >= 0; i--)  {
		prey* pr = g->preys + i;
		mang = g->config.mamu2 * g->config.vfr;
		float csp = pr->sp * g->config.vfr / 4;
		if (g->config.vfrb > 0) {
			if (pr->ftg > 0) {
				int k = g->config.vfrb;
				if (k > pr->ftg) k = pr->ftg;
				pr->ftg -= k;
				for (int qq = 1; qq <= k; qq++) {
					if (qq == k) {
						pr->fx = pr->fxs[pr->fpos];
						pr->fy = pr->fys[pr->fpos];
					}
					pr->fxs[pr->fpos] = 0;
					pr->fys[pr->fpos] = 0;
					pr->fpos++;
					if (pr->fpos >= RFC) pr->fpos = 0;
				}
			} else if (pr->ftg == 0) {
				pr->fx = 0;
				pr->fy = 0;
				pr->ftg = -1;
			}
		}
		if (pr->dir == 1) {
			pr->ang -= mang;
			if (pr->ang < 0 || pr->ang >= X2PI) pr->ang = fmodf(pr->ang, X2PI);
			if (pr->ang < 0) pr->ang += X2PI;
			vang = fmodf(pr->wang - pr->ang, X2PI);
			if (vang < 0) vang += X2PI;
			if (vang > PI) vang -= X2PI;
			if (vang > 0) {
				pr->ang = pr->wang;
				pr->dir = 0;
			}
		}
		else if (pr->dir == 2) {
			pr->ang += mang;
			if (pr->ang < 0 || pr->ang >= X2PI) pr->ang = fmodf(pr->ang, X2PI);
			if (pr->ang < 0) pr->ang += X2PI;
			vang = fmodf(pr->wang - pr->ang, X2PI);
			if (vang < 0) vang += X2PI;
			if (vang > PI) vang -= X2PI;
			if (vang < 0) {
				pr->ang = pr->wang;
				pr->dir = 0;
			}
		}
		else pr->ang = pr->wang;
		pr->xx += cosf(pr->ang) * csp;
		pr->yy += sinf(pr->ang) * csp;
		pr->gfr += g->config.vfr * pr->gr;
		pr->blink += 0.02f * g->config.vfr * pr->blink_dir;

		if (pr->blink >= 1.0) {
			pr->blink = 1.0;
			pr->blink_dir = -1;
		}
		else if (pr->blink <= 0.0) {
			pr->blink = 0.0;
			pr->blink_dir = 1;
		}

		if (pr->eaten) {
			if (pr->fr != 1.5f) {
				pr->fr += g->config.vfr / 150.0f;
				if (pr->fr >= 1.5f) pr->fr = 1.5f;
			}
			pr->eaten_fr += g->config.vfr / 47.0f;
			pr->gfr += g->config.vfr;
			snake* o = snake_map_get(&g->os, pr->eaten_by);
			if (pr->eaten_fr >= 1 || !o) ig_darray_remove(g->preys, i);
			else pr->rad = 1 - powf(pr->eaten_fr, 3);
		}
		else if (pr->fr != 1) {
			pr->fr += g->config.vfr / 150.0f;
			if (pr->fr >= 1) {
				pr->fr = 1;
				pr->rad = 1;
			}
			else {
				pr->rad = 0.5f * (1 - cosf(PI * pr->fr));
				pr->rad += (0.5f * (1 - cosf(PI * pr->rad)) - pr->rad) * 0.66f;
			}
		}
	}

	int cm1 = ig_darray_length(g->foods) - 1;
	for (int i = cm1; i >= 0; i--) {
		food* fo = g->foods + i;
		fo->gfr += g->config.vfr * fo->gr;
		if (fo->eaten) {
			fo->eaten_fr += g->config.vfr / 41.0f;
			snake* o = fo->eaten_by == -1 ? NULL : snake_map_get(&g->os, fo->eaten_by);

			if (fo->eaten_fr >= 1 || !o) {
				ig_darray_remove(g->foods, i);
			}
			else {
				// if (!o) {
				// 	printf("WT BRO\n");
				// 	g->network_done = 1;
				// 	g->game_quit = 1;
				// }

				float k = fo->eaten_fr * fo->eaten_fr;
				fo->rad = fo->lrrad * (1 - fo->eaten_fr * k);
				fo->rx = fo->xx + (o->xx + o->fx + cosf(o->ang + o->fa) * (43 - k * 24) * (1 - k) - fo->xx) * k;
				fo->ry = fo->yy + (o->yy + o->fy + sinf(o->ang + o->fa) * (43 - k * 24) * (1 - k) - fo->yy) * k;
				fo->rx += cosf(fo->wsp * fo->gfr) * 6 * (1 - fo->eaten_fr);
				fo->ry += sinf(fo->wsp * fo->gfr) * 6 * (1 - fo->eaten_fr);
			}
		} else {
			if (fo->fr != 1) {
				fo->fr += fo->rsp * g->config.vfr / 150.0f;
				if (fo->fr >= 1) {
					fo->fr = 1;
					fo->rad = 1;
				} else {
					fo->rad = 0.5f * (1 - cosf(PI * fo->fr));
					fo->rad += (0.5f * (1 - cosf(PI * fo->rad)) - fo->rad) * 0.66f;
				}
				fo->lrrad = fo->rad;
			}
			fo->rx = fo->xx;
			fo->ry = fo->yy;
			fo->rx = fo->xx + cosf(fo->wsp * fo->gfr) * 6.0f;
			fo->ry = fo->yy + sinf(fo->wsp * fo->gfr) * 6.0f;
		}
	}
}
