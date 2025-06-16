#include "redraw.h"
#include "game.h"
#include "prey.h"
#include "../networking/util.h"
#include "hud.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../external/cimgui/cimgui.h"

void redraw(game* g, const input_data* input_data) {
	float mww = g->icontext->default_frame.resolution.x;
	float mhh = g->icontext->default_frame.resolution.y;
	float mww2 = mww / 2;
	float mhh2 = mhh / 2;
	float mwwp50 = mww + 50;
	float mhhp50 = mhh + 50;

	igSetNextWindowPos((ImVec2) { .x = 0, .y = 0 }, ImGuiCond_None, (ImVec2) {});
	igSetNextWindowSize((ImVec2) { .x = g->icontext->default_frame.resolution.x, .y = g->icontext->default_frame.resolution.y }, ImGuiCond_None);
	igBegin("hud", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar);
	ImDrawList* draw_list = igGetWindowDrawList();

	igPushFont(g->renderer->fonts[RENDERER_FONT_SMALL]);
	if (!g->snake_null) { // snake != NULL
		if (!g->settings_instance.enable_zoom) {
			float dgsc = 0.64285f + 0.514285714f / fmaxf(1, (g->os.snakes[0].sct + 16.0f) / 36.0f);
			if (g->config.gsc != dgsc) {
				if (g->config.gsc < dgsc) {
					g->config.gsc += 2e-4;
					if (g->config.gsc >= dgsc) g->config.gsc = dgsc;
				} else {
					g->config.gsc -= 2e-4;
					if (g->config.gsc <= dgsc) g->config.gsc = dgsc;
				}
			}
		}
	}

	float lvx = g->config.view_xx;
	float lvy = g->config.view_yy;

	if (!g->snake_null) { // snake != NULL
		if (g->config.fvtg > 0) {
			g->config.fvtg--;
			g->config.fvx = g->config.fvxs[g->config.fvpos];
			g->config.fvy = g->config.fvys[g->config.fvpos];
			g->config.fvxs[g->config.fvpos] = 0;
			g->config.fvys[g->config.fvpos] = 0;
			g->config.fvpos++;
			if (g->config.fvpos >= VFC) g->config.fvpos = 0;
		}
	
		// follow_view
		g->config.view_xx = g->os.snakes[0].xx + g->os.snakes[0].fx + g->config.fvx;
		g->config.view_yy = g->os.snakes[0].yy + g->os.snakes[0].fy + g->config.fvy;

		g->config.view_ang = atan2f(g->config.view_yy - g->config.grd, g->config.view_xx - g->config.grd);
		// i dont need view_dist
		g->config.bpx1 = g->config.view_xx - (mww2 / g->config.gsc + 84);
		g->config.bpy1 = g->config.view_yy - (mhh2 / g->config.gsc + 84);
		g->config.bpx2 = g->config.view_xx + (mww2 / g->config.gsc + 84);
		g->config.bpy2 = g->config.view_yy + (mhh2 / g->config.gsc + 84);
		g->config.fpx1 = g->config.view_xx - (mww2 / g->config.gsc + 24);
		g->config.fpy1 = g->config.view_yy - (mhh2 / g->config.gsc + 24);
		g->config.fpx2 = g->config.view_xx + (mww2 / g->config.gsc + 24);
		g->config.fpy2 = g->config.view_yy + (mhh2 / g->config.gsc + 24);
		g->config.apx1 = g->config.view_xx - (mww2 / g->config.gsc + 210);
		g->config.apy1 = g->config.view_yy - (mhh2 / g->config.gsc + 210);
		g->config.apx2 = g->config.view_xx + (mww2 / g->config.gsc + 210);
		g->config.apy2 = g->config.view_yy + (mhh2 / g->config.gsc + 210);
	}

	g->config.bgx2 -= (g->config.view_xx - lvx) * 1 / (float) g->bg_tex->dim.x;
	g->config.bgy2 -= (g->config.view_yy - lvy) * 1 / (float) g->bg_tex->dim.y;
	
	renderer_push_bd(g->renderer, &(bd_instance) {
		.circ = {
			.x = mww2 + (g->config.grd - g->config.view_xx) * g->config.gsc,
			.y = mhh2 + (g->config.grd - g->config.view_yy) * g->config.gsc,
			.z = (g->config.flux_grd * 2) * g->config.gsc },
		.color = { .x = 0.188235, .y = 0, .z = 0, .w = 0.8 }
	});

	renderer_push_bg(g->renderer, &(sprite_instance) {
		.rect = {
			.x = 0,
			.y = 0,
			.z = mww,
			.w = mhh,
		},
		.ratios = { .x = 0, .y = 1 },
		.uv_rect = {
			.x = (((-mww / g->bg_tex->dim.x) / 2) / g->config.gsc) - g->config.bgx2,
			.y = (((-mhh / g->bg_tex->dim.y) / 2) / g->config.gsc) - g->config.bgy2,
			.z = (mww / g->bg_tex->dim.x) / g->config.gsc,
			.w = (mhh / g->bg_tex->dim.y) / g->config.gsc },
		.color = { .x = 1 * (1 - g->settings_instance.black_bg), .y = 1 * (1 - g->settings_instance.black_bg), .z = 1 * (1 - g->settings_instance.black_bg), .w = 1 }
	});

	// foods
	for (int i = ig_darray_length(g->foods) - 1; i >= 0; i--) {
		// FOOD RENDER
		food* fo = g->foods + i;
		ig_vec3* col = g->config.color_groups + fo->cv;

		if (g->config.big_food && fo->f < 68) continue; // dont render small food

		if (fo->rx >= g->config.fpx1 && fo->ry >= g->config.fpy1 && fo->rx <= g->config.fpx2 && fo->ry <= g->config.fpy2) {
			float fx = mww2 + g->config.gsc * (fo->rx - g->config.view_xx) - g->config.gsc * ((fo->rad * fo->f2) * g->settings_instance.food_scale);
			float fy = mhh2 + g->config.gsc * (fo->ry - g->config.view_yy) - g->config.gsc * ((fo->rad * fo->f2) * g->settings_instance.food_scale);

			renderer_push_food(g->renderer, &(food_instance) {
				.circ = { .x = fx, .y = fy, .z = ((fo->rad * fo->f) * g->settings_instance.food_scale) * g->config.gsc },
				.ratios = { .x = 0, .y = 1 },
				.color = { .x = col->x, .y = col->y, .z = col->z, .w = .8 * fo->fr }
			});
		}
	}

	// preys
	for (int i = ig_darray_length(g->preys) - 1; i >= 0; i--) {
		prey* pr = g->preys + i;
		float tx = pr->xx + pr->fx;
		float ty = pr->yy + pr->fy;
		float px = mww2 + g->config.gsc * (tx - g->config.view_xx);
		float py = mhh2 + g->config.gsc * (ty - g->config.view_yy);
		ig_vec3* col = g->config.color_groups + pr->cv;

		if (px >= -50 && py >= -50 && px <= mwwp50 && py <= mhhp50) {
			if (pr->eaten) {
				snake* o = snake_map_get(&g->os, pr->eaten_by);
				if (o) {
					float k = powf(pr->eaten_fr, 2);
					tx += (o->xx + o->fx + cosf(o->ang + o->fa) * (43 - k * 24) * (1 - k) - tx) * k;
					ty += (o->yy + o->fy + sinf(o->ang + o->fa) * (43 - k * 24) * (1 - k) - ty) * k;
					px = mww2 + g->config.gsc * (tx - g->config.view_xx);
					py = mhh2 + g->config.gsc * (ty - g->config.view_yy);
				}
			}
			float fx = px - g->config.gsc * ((pr->f2 * pr->rad) * 1.2f);
			float fy = py - g->config.gsc * ((pr->f2 * pr->rad) * 1.2f);
			
			renderer_push_food(g->renderer, &(food_instance) {
				.circ = { .x = fx, .y = fy, .z = g->config.gsc * ((pr->f * pr->rad) * 1.2f) },
				.ratios = { .x = 0, .y = 1 },
				.color = { .x = col->x + pr->blink * (1 - col->x), .y = col->y + pr->blink * (1 - col->y), .z = col->z + pr->blink * (1 - col->z), .w = pr->fr }
			});
		}
	}

	// snake is in view
	int snakes_len = snake_map_get_total(&g->os);
	for (int i = 0; i < snakes_len; i++) {
		snake* o = g->os.snakes + i;
		o->iiv = 0;
		int pts_len = ig_darray_length(o->pts);
		for (int j = pts_len - 1; j >= 0; j--) {
			body_part* po = o->pts + j;
			float px = po->xx + po->fx;
			float py = po->yy + po->fy;
			if (px >= g->config.bpx1 && py >= g->config.bpy1 && px <= g->config.bpx2 && py <= g->config.bpy2) {
				o->iiv = 1;
				break;
			}
		}
		if (o->iiv)	o->ehang = o->wehang = o->ang;
	}

	// snakes render:
	snakes_len = snake_map_get_total(&g->os);
	for (int i = 0; i < snakes_len; i++) {
		snake* o = g->os.snakes + i;
		if (o->iiv) {
			float hx = o->xx + o->fx;
			float hy = o->yy + o->fy;
			float px = hx;
			float py = hy;
			float fang = o->ehang;
			float ssc = o->sc;
			float lsz = 29 * ssc;
			float rl = o->cfl;

			int pts_len = ig_darray_length(o->pts);
			body_part* po = o->pts + (pts_len - 1);

			lsz *= 0.5f;
			float ix1, iy1, ix2, iy2, ax1, ay1, ax2, ay2;
			int bp = 0;
			ax2 = px;
			ay2 = py;
			float ax = px;
			float ay = py;
			bp = 0;
			
			// drez is for some skin - off by default
			float rezc = 0;
			
			if (o->sep != o->wsep) {
				if (o->sep < o->wsep) {
					o->sep += 0.0035f;
					if (o->sep >= o->wsep) o->sep = o->wsep;
				} else if (o->sep > o->wsep) {
					o->sep -= 0.0035f;
					if (o->sep <= o->wsep) o->sep = o->wsep;
				}
			}

			float px2, py2, px3, py3;
			body_part *po2, *po3, *lpo;
			float d, d2, dx, dy, d3 = 0;
			float tx, ty, ox, oy, rx, ry;
			tx = 0;
			ty = 0;
			float k, l, m, k2, irl, wk = 0, wwk, nkr;
			int j, j2;
			float msl = o->msl;
			float mct = 6 / (g->config.qsm * o->sep / 6);
			float omct = mct;
			float rmct = 1 / mct;
			float sep = msl / mct;
			float ll = 0;
			pts_len = ig_darray_length(o->pts);
			po = o->pts + pts_len - 1;
			px = po->xx + po->fx;
			py = po->yy + po->fy;
			d = sqrtf(powf(hx - px, 2) + powf(hy - py, 2));
			dx = (hx - px) / d;
			dy = (hy - py) / d;
			nkr = d / msl;
			gpt_struct *gpt, *lgpt;
			gpt_struct *gpt2, *lgpt2;
			float gpo;
			int q = 0;
			if (pts_len >= 2) {
				po3 = o->pts + pts_len - 2;
			} else {
				po3 = NULL;
			}
			po2 = o->pts + pts_len - 1;
			px = hx;
			py = hy;
			px2 = po2->xx + po2->fx;
			py2 = po2->yy + po2->fy;
			if (po3) {
				px3 = po3->xx + po3->fx;
				py3 = po3->yy + po3->fy;
			}
			if (d > msl) {
				px = px2 + dx * msl;
				py = py2 + dy * msl;
			}
			ax1 = px + (px2 - px) * .5;
			ay1 = py + (py2 - py) * .5;
			if (nkr < 1) {
				ax1 += (px - ax1) * (1 - nkr);
				ay1 += (py - ay1) * (1 - nkr);
			}
			ax2 = px3 + (px2 - px3) * .5;
			ay2 = py3 + (py2 - py3) * .5;
			d2 = sqrtf(powf(hx - ax1, 2) + powf(hy - ay1, 2));
			k = sep;
			m = 1;
			gpt = arp(o, q, hx, hy);
			q++;
			gpt->d = 0;
			wk++;
			while (k < d2) {
				tx = hx - m * dx * sep;
				ty = hy - m * dy * sep;
				gpt = arp(o, q, tx, ty);
				q++;
				d = sep;
				gpt->d = d;
				wk++;
				if (ll == 1) {
					ll = 2;
					break;
				}
				rl -= rmct;
				if (rl <= 0) {
					ll = 1;
					m += (rmct + rl) / rmct;
					k += sep * (rmct + rl) / rmct;
				} else {
					m++;
					k += sep;
				}
			}
			irl = (k - d2) / msl;
			if (ll <= 1) {
				if (rl >= -1E-4 && rl <= 0) rl = 0;
				if (rl >= 0 || ll == 1) {
					if (nkr < 1) {
						px2 += (ax2 - px2) * .5 * (1 - nkr);
						py2 += (ay2 - py2) * .5 * (1 - nkr);
					}
					m = .5 + nkr - d2 / msl;
					while (irl >= 0 && irl < m) {
						k = irl / m;
						ix1 = ax1 + (px2 - ax1) * k;
						iy1 = ay1 + (py2 - ay1) * k;
						ix2 = px2 + (ax2 - px2) * k;
						iy2 = py2 + (ay2 - py2) * k;
						rx = ix1 + (ix2 - ix1) * k;
						ry = iy1 + (iy2 - iy1) * k;
						gpt = arp(o, q, rx, ry);
						if (q == 0)	lgpt = o->gptz + q;
						else	lgpt = o->gptz + q-1;
						q++;
						d = sqrtf(powf(gpt->xx - lgpt->xx, 2) + powf(gpt->yy - lgpt->yy, 2));
						gpt->d = d;
						lgpt = gpt;
						wk++;
						if (ll == 1) {
							ll = 2;
							break;
						}
						rl -= rmct;
						if (rl <= 0) {
							ll = 1;
							irl += rmct + rl;
							rl = 0;
						} else irl += rmct;
					}
					irl -= m;
				}
				if (rl >= -1E-4 && rl <= 0) rl = 0;
			}

			int lj = ig_darray_length(o->pts);
			if (ll <= 1) {
				int wsirl = false;
				if (rl >= 0 || ll == 1) {
					wsirl = false;
					int rmr = 0;
					po = o->pts + lj - 1;
					for (int j = lj - 1; j >= 2; j--) {
						lj = j;
						lpo = po;
						po3 = o->pts + j - 2;
						po2 = o->pts + j - 1;
						po = o->pts + j;
						px = po->xx + po->fx;
						py = po->yy + po->fy;
						px2 = po2->xx + po2->fx;
						py2 = po2->yy + po2->fy;
						px3 = po3->xx + po3->fx;
						py3 = po3->yy + po3->fy;
						ax1 = px + (px2 - px) * .5;
						ay1 = py + (py2 - py) * .5;
						ax2 = px2 + (px3 - px2) * .5;
						ay2 = py2 + (py3 - py2) * .5;
						m = po->ltn + po->fltn;
						wwk = omct * 2 + 2;
						if (po->smu != lpo->smu || po->fsmu != lpo->fsmu) {
							irl *= (lpo->smu + lpo->fsmu) / (po->smu + po->fsmu);
							mct = omct * (po->smu + po->fsmu);
							rmct = 1 / mct;
							sep = msl / mct;
						}
						rl -= rmr * rmct;
						while (irl < m) {
							k = irl / m;
							ix1 = ax1 + (px2 - ax1) * k;
							iy1 = ay1 + (py2 - ay1) * k;
							ix2 = px2 + (ax2 - px2) * k;
							iy2 = py2 + (ay2 - py2) * k;
							rx = ix1 + (ix2 - ix1) * k;
							ry = iy1 + (iy2 - iy1) * k;
							gpt = arp(o, q, rx, ry);
							if (q == 0)	lgpt = o->gptz + q;
							else	lgpt = o->gptz + q-1;
							q++;
							if (wk <= wwk) {
								d = sqrtf(powf(gpt->xx - lgpt->xx, 2) + powf(gpt->yy - lgpt->yy, 2));
								gpt->d = d;
								lgpt = gpt;
								wk++;
							}
							if (ll == 1) {
								ll = 2;
								j = -9999;
								break;
							}
							rl -= rmct;
							if (rl <= 0) {
								ll = 1;
								irl += rmct + rl;
							} else irl += rmct;
						}
						irl -= m;
						rmr = irl / rmct;
						rl += irl;
						wsirl = true;
					}
				}
				if (wsirl) rl -= irl;
			}
			if (ll <= 1) {
				if (rl >= -1E-4 && rl <= 0) rl = 0;
				if (rl >= 0 || ll == 1) {
					po = o->pts + lj - 1;
					po2 = o->pts + lj - 2;
					if (po) {
						px = po->xx + po->fx;
						py = po->yy + po->fy;
					}
					px2 = po2->xx + po2->fx;
					py2 = po2->yy + po2->fy;
					while (rl >= 0 || ll == 1) {
						rx = px2 - (px - px2) * (irl - .5);
						ry = py2 - (py - py2) * (irl - .5);
						gpt = arp(o, q, rx, ry);
						if (q == 0)	lgpt = o->gptz + q;
						else	lgpt = o->gptz + q-1;
						q++;
						if (wk <= wwk) {
							d = sqrtf(powf(gpt->xx - lgpt->xx, 2) + powf(gpt->yy - lgpt->yy, 2));
							gpt->d = d;
							lgpt = gpt;
							wk++;
						}
						if (ll == 1) {
							ll = 2;
							j = -9999;
							break;
						}
						rl -= rmct;
						if (rl <= 0) {
							ll = 1;
							irl += rmct + rl;
						} else irl += rmct;
						if (rl >= -1E-4 && rl <= 0) rl = 0;
					}
				}
			}
			k = wk - 1;
			int gptz_len = ig_darray_length(o->gptz);
			if (k >= gptz_len) k = gptz_len;
			if (k >= 3) {
				d3 = 0;
				for (j = 0; j < k - 1; j++) {
					gpt = o->gptz + j;
					d3 += gpt->d;
				}
				lgpt = o->gptz + 0;
				lgpt2 = o->gptz + 0;
				m = d3 / (k - 2);
				j = 1;
				j2 = 1;
				float v = m;
				for (j = 0; j < k; j++) {
					o->gptz[j].ox = o->gptz[j].xx;
					o->gptz[j].oy = o->gptz[j].yy;
				}
				for (j = 1; j < k; j++) {
					gpt = o->gptz + j;
					while (true) {
						gpt2 = o->gptz + j2;
						if (v < gpt2->d) {
							gpt->xx = lgpt2->ox + (gpt2->ox - lgpt2->ox) * v / gpt2->d;
							gpt->yy = lgpt2->oy + (gpt2->oy - lgpt2->oy) * v / gpt2->d;
							gpt->xx += (gpt->ox - gpt->xx) * powf(j / (float) k, 2);
							gpt->yy += (gpt->oy - gpt->yy) * powf(j / (float) k, 2);
							v += m;
							break;
						} else {
							v -= gpt2->d;
							lgpt2 = gpt2;
							j2++;
							if (j2 >= k) {
								j = k + 1;
								break;
							}
						}
					}
					lgpt = gpt;
				}
			}
			float lpx, lpy;
			for (j = 0; j < q; j++) {
				px = o->gptz[j].xx;
				py = o->gptz[j].yy;
				g->config.pbx[bp] = px;
				g->config.pby[bp] = py;
				g->config.pba[bp] = 0;
				if (px >= g->config.bpx1 && py >= g->config.bpy1 && px <= g->config.bpx2 && py <= g->config.bpy2)
					if (0/*drez && rezc != 3*/) ;//pbu[bp] = 1;
					else g->config.pbu[bp] = 2;
				if (bp >= 1) {
					tx = px - lpx;
					ty = py - lpy;
					g->config.pba[bp] = atan2f(ty, tx);
				}
				lpx = px;
				lpy = py;
				bp++;
			}
			if (q >= 2) {
				g->config.pba[0] = g->config.pba[1];
				o->wehang = g->config.pba[1] + PI;
			} else o->wehang = o->ang;
			int dj = 4;
			
			float olsz = g->config.gsc * lsz * 52 / 32;
			float shsz = g->config.gsc * lsz * 62 / 32;

			float a = o->alive_amt * (1 - o->dead_amt), om, mr;
			a *= a;
			m = 1;

			// glow on player boost
			if (o->tsp > o->fsp && g->config.show_boost) {
				m = o->alive_amt * (1 - o->dead_amt) * fmaxf(0, fminf(1, (o->tsp - o->ssp) / (o->msp - o->ssp)));
				om = m * 0.37f;
				mr = powf(m, 0.5f);
				float glsz = (1.5f * g->config.gsc * lsz * (1 + (62.0f / 32 - 1) * mr));
				float dj = 4;

				for (j = bp - 1; j >= 0; j--) {
					if (g->config.pbu[j] == 2) {
						float ox = tx, oy = ty;
						tx = g->config.pbx[j];
						ty = g->config.pby[j];
						if (tx > ox)	d2 = tx - ox;
						else	d2 = ox - tx;
						if (ty > oy)	d2 += ty - oy;
						else	d2 += oy - ty;
						d2 /= 6;
						if (d2 > 1)	d2 = 1;
						float alpha = d2 * a * mr * .38 * (.6 + .4 * cosf(j / dj - 1.15f * o->sfr));
						px = (mww2 + ((tx - g->config.view_xx) * g->config.gsc));
						py = (mhh2 + ((ty - g->config.view_yy) * g->config.gsc));
						// renderer_push_bp(g->renderer, &(bp_instance) {
						// 	.circ = { .x = px - glsz, .y = py - glsz, .z = 0.2, .w = glsz * 2 },
						// 	.ratios = { .x = 0, .y = 1 },
						// 	.color = { .x = 1, .y = 1, .z = 1, .w = alpha },
						// 	.shadow = 1
						// });
					}
				}
				m = 1 - m;
			}

			// render snake body
			float am = a * m;
			// CODE NOT THE SAME, FIX!!
			float oa = am;

			for (j = bp - 1; j >= 0; j--) {
				if (g->config.pbu[j] >= 1) {
					if (j >= 1 && g->config.pbu[j - 1] == 2) {
						float shsz = (g->config.gsc * lsz * 62.0f / 32.0f) * 0.75f;
						px = (mww2 + ((g->config.pbx[j - 1] - g->config.view_xx) * g->config.gsc));
						py = (mhh2 + ((g->config.pby[j - 1] - g->config.view_yy) * g->config.gsc));

						// shadows
						renderer_push_bp(g->renderer, &(bp_instance) {
							.circ = { .x = px - shsz, .y = py - shsz, .z = 1 - 0, .w = shsz * 2 },
							.ratios = { .x = 0, .y = 1 },
							.color = { .x = 0, .y = 0, .z = 0, .w = g->config.shadow * 0.9f * a },
							.shadow = 1
						});
					}

					ig_vec3* col = g->config.color_groups + o->skin_data[j % o->skin_data_len];

					px = mww2 + ((g->config.pbx[j] - g->config.view_xx) * g->config.gsc);
					py = mhh2 + ((g->config.pby[j] - g->config.view_yy) * g->config.gsc);

					// actual body
					renderer_push_bp(g->renderer, &(bp_instance) {
						.circ = { .x = px + (-g->config.gsc * lsz), .y = py + (-g->config.gsc * lsz), .z = 1 - 0, .w = g->config.gsc * 2 * lsz },
						.ratios = { .x = sinf(g->config.pba[j]), .y = cosf(g->config.pba[j]) },
						.color = { .x = col->x, .y = col->y, .z = col->z, .w = a },
						.shadow = 0,
					});
				}
			}

			// if (!g->snake_null && i == 0) {
			// 	renderer_push_sprite(g->renderer, &(sprite_instance) {
			// 		.rect = { .x = mww2 - 1, .y = mhh2 - 50, .z = 2, .w = 100 },
			// 		.ratios = { .x = sinf(o->ang + PI / 2), .y = cosf(o->ang + PI / 2) },
			// 		.uv_rect = { .x = 3 / 64.0f, .y = 3 / 64.0f, .z = 1 / 64.0f, .w = 1 / 64.0f },
			// 		.color = { .x = 1, .y = 1, .z = 1 }
			// 	});
			// }

			// eyes

			// drawEyes fang, ssc, a=1, a2=1
			/*float a = 1, a2 = 1;
			float ed = 6 * ssc;
			float esp = 6 * ssc;
			float hx = o->xx + o->fx;
			float hy = o->yy + o->fy;

			float ex = cosf(fang) * ed + cosf(fang - PI / 2) * (esp + 0.5f);
			float ey = sinf(fang) * ed + sinf(fang - PI / 2) * (esp + 0.5f);

			if (o->dead_amt == 0)	a *= .75f;
			else	a *= .75 * sqrtf(1 - o->dead_amt);
			
			😭*/

			float ed = 6 * ssc;
			float esp = 6 * ssc;

			float ex = cosf(fang) * ed + cosf(fang - PI / 2) * (esp + 0.5f);
			float ey = sinf(fang) * ed + sinf(fang - PI / 2) * (esp + 0.5f);

			float rd2 = (6 * ssc * g->config.gsc) * 2;

			renderer_push_bp(g->renderer, &(bp_instance) {
				.circ = {
					.x = (mww2 + (ex + hx - g->config.view_xx) * g->config.gsc) - rd2 / 2,
					.y = (mhh2 + (ey + hy - g->config.view_yy) * g->config.gsc) - rd2 / 2,
					.z = (1 - 0),
					.w = rd2
				},
				.ratios = { .x = 0, .y = 1 },
				.color = { .x = 1, .y = 1, .z = 1, .w = a },
				.eye = 1
			});

			ex = cosf(fang) * (ed + 0.5f) + o->rex * ssc + cosf(fang - PI / 2) * esp;
			ey = sinf(fang) * (ed + 0.5f) + o->rey * ssc + sinf(fang - PI / 2) * esp;
			rd2 = (3.5f * ssc * g->config.gsc) * 2;

			renderer_push_bp(g->renderer, &(bp_instance) {
				.circ = {
					.x = (mww2 + (ex + hx - g->config.view_xx) * g->config.gsc) - rd2 / 2,
					.y = (mhh2 + (ey + hy - g->config.view_yy) * g->config.gsc) - rd2 / 2,
					.z = (1 - 0),
					.w = rd2
				},
				.ratios = { .x = 0, .y = 1 },
				.color = { .x = 0, .y = 0, .z = 0, .w = a },
				.eye = 1,
			});

			ex = cosf(fang) * ed + cosf(fang + PI / 2) * (esp + 0.5f);
			ey = sinf(fang) * ed + sinf(fang + PI / 2) * (esp + 0.5f);
			rd2 = (6 * ssc * g->config.gsc) * 2;

			// if (!g->snake_null && o == g->os.snakes)
			// 	printf("iris size = %d, bp size = %d, pupil size = %d\n", (int) rd2, (int) (g->config.gsc * 2 * lsz), (int) ((3.5f * ssc * g->config.gsc) * 2));

			renderer_push_bp(g->renderer, &(bp_instance) {
				.circ = {
					.x = (mww2 + (ex + hx - g->config.view_xx) * g->config.gsc) - rd2 / 2,
					.y = (mhh2 + (ey + hy - g->config.view_yy) * g->config.gsc) - rd2 / 2,
					.z = (1 - 0),
					.w = rd2
				},
				.ratios = { .x = 0, .y = 1 },
				.color = { .x = 1, .y = 1, .z = 1, .w = a },
				.eye = 1,
			});

			ex = cosf(fang) * (ed + 0.5f) + o->rex * ssc + cosf(fang + PI / 2) * esp;
			ey = sinf(fang) * (ed + 0.5f) + o->rey * ssc + sinf(fang + PI / 2) * esp;
			rd2 = (3.5f * ssc * g->config.gsc) * 2;

			renderer_push_bp(g->renderer, &(bp_instance) {
				.circ = {
					.x = (mww2 + (ex + hx - g->config.view_xx) * g->config.gsc) - rd2 / 2,
					.y = (mhh2 + (ey + hy - g->config.view_yy) * g->config.gsc) - rd2 / 2,
					.z = (1 - 0),
					.w = rd2
				},
				.ratios = { .x = 0, .y = 1 },
				.color = { .x = 0, .y = 0, .z = 0, .w = a },
				.eye = 1,
			});

			if (g->config.player_names) {
				if (g->snake_null || o != g->os.snakes + 0) {
					igPushFont(g->renderer->fonts[g->settings_instance.names_font]);
					float ntx = o->xx + o->fx;
					float nty = o->yy + o->fy;
					ntx = mww2 + (ntx - g->config.view_xx) * g->config.gsc;
					nty = mhh2 + (nty - g->config.view_yy) * g->config.gsc;
					int len; ImVec2 nick_txt_size;

					if (g->settings_instance.show_lengths) {
						int snake_length_k = (floorf((g->config.fpsls[o->sct] + o->fam / g->config.fmlts[o->sct] - 1) * 15 - 5) / 1) / 1000;
						if (snake_length_k > 0) {
							char nk_buff[64] = {};
							if (o->nk[0])
								sprintf(nk_buff, "%s @ %dK", o->nk, snake_length_k);
							else
								sprintf(nk_buff, "@ %dK", snake_length_k);

							igCalcTextSize(&nick_txt_size, nk_buff, NULL, 0, 0); len = nick_txt_size.x;
							ImDrawList_AddText_Vec2(draw_list,
								(ImVec2) {
								.x = ntx - len / 2, .y = nty + 32 + 11 * o->sc * g->config.gsc
							}, igColorConvertFloat4ToU32((ImVec4) { .x = g->settings_instance.names_color.x, .y = g->settings_instance.names_color.y, .z = g->settings_instance.names_color.z, .w = a }), nk_buff, NULL);
						} else {
							igCalcTextSize(&nick_txt_size, o->nk, NULL, 0, 0); len = nick_txt_size.x;
							ImDrawList_AddText_Vec2(draw_list,
								(ImVec2) {
								.x = ntx - len / 2, .y = nty + 32 + 11 * o->sc * g->config.gsc
							}, igColorConvertFloat4ToU32((ImVec4) { .x = g->settings_instance.names_color.x, .y = g->settings_instance.names_color.y, .z = g->settings_instance.names_color.z, .w = a }), o->nk, NULL);
						}
					} else {
						igCalcTextSize(&nick_txt_size, o->nk, NULL, 0, 0); len = nick_txt_size.x;
						ImDrawList_AddText_Vec2(draw_list,
								(ImVec2) {
								.x = ntx - len / 2, .y = nty + 32 + 11 * o->sc * g->config.gsc
							}, igColorConvertFloat4ToU32((ImVec4) { .x = g->settings_instance.names_color.x, .y = g->settings_instance.names_color.y, .z = g->settings_instance.names_color.z, .w = a }), o->nk, NULL);
					}
					igPopFont();
				}
			}
		}
	}
	igPopFont();

	if (g->config.show_hud)
		hud(g, input_data);
	igEnd();
}
