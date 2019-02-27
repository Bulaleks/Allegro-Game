#include <stdio.h>
#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"

#define MAP_W 16
#define MAP_H 16
#define NODE_MAX 16

const float FPS = 60.0;
const int SCR_W = 640, SCR_H = 480, TILE_W = 32, TILE_H = 32;
const int NODE_WALL = -1, NODE_BLANK = -2;

struct obj_soul_t {
	bool alive;
	bool on_route;
	/* AI conf flags */
};

struct obj_t {
	int x;
	int y;

	ALLEGRO_BITMAP *bitmap;

	struct obj_soul_t *soul;
};

struct tile_t {
	int node_w;
	int terrain;
	bool selected;

	struct obj_t *obj;
};

struct node_t {
	int x;
	int y;

	bool last;
};

struct obj_t player, wall;
struct obj_soul_t plr_soul;
struct tile_t tilemap[MAP_W][MAP_H];
struct node_t path[NODE_MAX];
int move_i = 1;
ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_BITMAP *tiles = NULL;
ALLEGRO_FONT *font = NULL;
int mouse = 0;
float scroll_x = 0, scroll_y = 0;


void plr_init(int x, int y)
{
	player.x = x;
	player.y = y;

	player.bitmap = al_create_bitmap(TILE_W, TILE_H);
/*	al_set_target_bitmap(player.bitmap);
	al_draw_filled_rectangle(4, 4, TILE_W-4, TILE_H-4, al_map_rgb(0,0,255));
	al_set_target_backbuffer(display);
*/
	player.soul = &plr_soul;
	player.soul->alive = true;
}


void wall_init(int x, int y)
{
	wall.x = x;
	wall.y = y;

	wall.bitmap = al_create_bitmap(TILE_W, TILE_H);
/*	al_set_target_bitmap(wall.bitmap);
	al_draw_filled_rectangle(0, 0, TILE_W, TILE_H, al_map_rgb(0,0,0));
	al_set_target_backbuffer(display);
*/
	wall.soul = NULL;
}


void wall_create(bool xy, int x, int y, int length)
{
	int i = 0;

	if (!xy) {
		for (i=x; i<(x+length); i++) {
			tilemap[i][y].obj = &wall;
		}
	} else {
		for (i=y; i<(y+length); i++) {
			tilemap[x][i].obj = &wall;
		}
	}
}

void tile_draw(float x, float y, float w, float h)
{
	al_draw_rectangle(x, y, x+w, y+h, al_map_rgb(127,127,127), 2);

	// selected tile (PH)
	al_draw_filled_rectangle(x+TILE_W, y, x+TILE_W+w, y+h, al_map_rgb(255,0,0));
}


void tile_get(ALLEGRO_BITMAP *tileset, ALLEGRO_BITMAP *bitmap, int x, int y)
{
	al_set_target_bitmap(bitmap);

	al_draw_bitmap_region(tileset, x*TILE_W, y*TILE_H, TILE_W, TILE_H, 0, 0, 0);
}


void nodes_refresh(void)
{
	int x = 0, y = 0;

	for (x = 0; x < MAP_W; x++) {
		for (y = 0; y < MAP_H; y++) {
			if (tilemap[x][y].obj == NULL) {
				tilemap[x][y].node_w = NODE_BLANK;
			} else {
				tilemap[x][y].node_w = NODE_WALL;
			}
		}
	}
}


void tile_map_create(void)
{
//	tiles = al_create_bitmap(TILE_W*8, TILE_H*8);
	tiles = al_load_bitmap("tileset_test.bmp");
/*	al_set_target_bitmap(tiles);
	al_clear_to_color(al_map_rgb(0,64,0));
	tile_draw(0, 0, TILE_W, TILE_H);
*/
	al_convert_mask_to_alpha(tiles, al_map_rgb(255,255,255));

	al_set_target_bitmap(wall.bitmap);
	al_draw_bitmap_region(tiles, 0*TILE_W, 1*TILE_H, TILE_W, TILE_H, 0, 0, 0);

	al_set_target_bitmap(player.bitmap);
	al_draw_bitmap_region(tiles, 0*TILE_W, 2*TILE_H, TILE_W, TILE_H, 0, 0, 0);

	al_set_target_backbuffer(display);
	wall_create(0,0,1,2);
	wall_create(1,5,1,7);

	nodes_refresh();

	// Center camera on Player
	scroll_x = TILE_W*player.x;
	scroll_y = TILE_H*player.y + TILE_H/2;
}


void tile_map_draw(void)
{
	int x = 0, y = 0;
	ALLEGRO_TRANSFORM transform;
	float w, h;

	w = al_get_display_width(display);
	h = al_get_display_height(display);

	al_identity_transform(&transform);
	al_translate_transform(&transform, -scroll_x, -scroll_y);
	al_translate_transform(&transform, w*0.5, h*0.5);
	al_use_transform(&transform);

	al_clear_to_color(al_map_rgb(0,0,0));

	al_hold_bitmap_drawing(1);
	for (x=0; x<MAP_W; x++) {
		for (y=0; y<MAP_H; y++) {
			al_draw_bitmap_region(tiles, 0*TILE_W, 0*TILE_H, TILE_W, TILE_H, x*TILE_W, y*TILE_H, 0);
			if (tilemap[x][y].obj != NULL) {
				al_draw_bitmap(tilemap[x][y].obj->bitmap, x*TILE_W, y*TILE_H, 0);
			}
			if (tilemap[x][y].selected) {
				al_draw_bitmap_region(tiles, 0*TILE_W, 2*TILE_H, TILE_W, TILE_H, x*TILE_W, y*TILE_H, 0);
			}
		}
	}
	al_hold_bitmap_drawing(0);

	al_identity_transform(&transform);
	al_use_transform(&transform);
}


int plr_action(int keycode)
{
	int tx = player.x, ty = player.y, tscroll_x = scroll_x, tscroll_y = scroll_y;
	/* select */
	/* move */
	tilemap[tx][ty].obj = NULL;
	switch (keycode) {
		case ALLEGRO_KEY_A:
			if (tx > 0) {
				tx--;
				tscroll_x -= TILE_W;
			}
			break;

		case ALLEGRO_KEY_D:
			if (tx < MAP_W-1) {
				tx++;
				tscroll_x += TILE_W;
			}
			break;

		case ALLEGRO_KEY_W:
			if (ty > 0) {
				ty--;
				tscroll_y -= TILE_H;
			}
			break;

		case ALLEGRO_KEY_S:
			if (ty < MAP_H-1) {
				ty++;
				tscroll_y += TILE_H;
			}
			break;

		case ALLEGRO_KEY_E:
			if (tx < MAP_W-1 && ty > 0) {
				tx++;	ty--;
				tscroll_x += TILE_W;	tscroll_y -= TILE_H;
			}
			break;

		case ALLEGRO_KEY_Q:
			if (tx > 0 && ty > 0) {
				tx--;	ty--;
				tscroll_x -= TILE_W;	tscroll_y -= TILE_H;
			}
			break;

		case ALLEGRO_KEY_X:
			if (tx < MAP_W-1 && ty < MAP_H-1) {
				tx++;	ty++;
				tscroll_x += TILE_W;	tscroll_y += TILE_H;
			}
			break;

		case ALLEGRO_KEY_Z:
			if (tx > 0 && ty < MAP_H-1) {
				tx--;	ty++;
				tscroll_x -= TILE_W;	tscroll_y += TILE_H;
			}
			break;

		default:
			break;
	}
	if (tilemap[tx][ty].obj == NULL) {
		player.x = tx; player.y = ty;
		scroll_x = tscroll_x; scroll_y = tscroll_y;
	} else if (tilemap[tx][ty].obj == &wall) {
//		return -1;
	}
	tilemap[player.x][player.y].obj = &player;
	/* interact */
	/* info */

	return 0;
}


void plr_tele(int x, int y)
{
	tilemap[player.x][player.y].obj = NULL;
	player.x = x;	player.y = y;
	tilemap[player.x][player.y].obj = &player;
}


void plr_step(void)
{
	printf("node %d x= %d, y= %d\nlast = %d\n", move_i, path[move_i].x, path[move_i].y, path[move_i].last);
	plr_tele(path[move_i].x, path[move_i].y);
	printf("player at (%d, %d)\n", player.x, player.y);

	if (path[move_i].last == true) {
		player.soul->on_route = false;
		move_i = 1;
	}

	move_i++;
}


bool lee(int ax, int ay, int bx, int by)
{
	int dx[4] = {1, 0, -1, 0};
	int dy[4] = {0, 1, 0, -1};
	int d, x, y, k;
	bool stop;

	for (d = 0; d < NODE_MAX; d++) {
		path[d].x = 0;
		path[d].y = 0;
		path[d].last = false;
		move_i = 1;
	}

	if (tilemap[bx][by].node_w == NODE_WALL) {
		printf("---WALL---\n");
		return false;
	}

	/* wave */
	/*
	a(x,y) - origin
	b(x,y) - destination
	i = 0;
	origin.node_w = i;
	do {
	neighbour tiles .node_w = i+1;
	i++;
	} while(destination !reached);
	*/
	d = 0;
	tilemap[ax][ay].node_w = 0;
	do {
		stop = true;
		for (x = 0; x < MAP_W; x++) {
			for (y = 0; y < MAP_H; y++) {
				if (tilemap[x][y].node_w == d) {
					for (k = 0; k < 4; k++) {
						int ix = x + dx[k], iy = y + dy[k];
						if (ix >= 0 && ix < MAP_W && iy >= 0 && iy < MAP_H
									&& tilemap[ix][iy].node_w == NODE_BLANK) {
							stop = false;
							tilemap[ix][iy].node_w = d + 1;
						}
					}
				}
			}
		}
		d++;
	} while (!stop && tilemap[bx][by].node_w == NODE_BLANK && d < NODE_MAX-1);

	if (tilemap[bx][by].node_w == NODE_BLANK) return false;

	/* backtrace */
	d = tilemap[bx][by].node_w;
	path[d].last = true;
	x = bx;
	y = by;
	while (d > 0) {
		path[d].x = x;
		path[d].y = y;
		d--;
		
		for (k = 0; k < 4; k++) {
			int ix = x + dx[k], iy = y + dy[k];
			if (ix >= 0 && ix < MAP_W && iy >= 0 && iy < MAP_H
									  && tilemap[ix][iy].node_w == d) {
				x = x + dx[k];
				y = y + dy[k];
				break;
			}
		}
	}
	path[0].x = ax;
	path[0].y = ay;

	return true;
}


//???
void obj_response(void)
{

}
//???

void obj_behave(void)
{
	/* AI for objs with mind != NULL */
}


int main(void)
{
	ALLEGRO_EVENT_QUEUE *ev_que = NULL;
	ALLEGRO_TIMER *timer = NULL, *move_tmr = NULL;
	bool redraw = true;
	int selected_tile_x = 0, selected_tile_y = 0;
	
	if (!al_init()) {
		fprintf(stderr, "Failed al_init\n");
		return -1;
	}
	if (!al_init_image_addon()) {
		fprintf(stderr, "Failed init_image\n");
		return -1;
	}
	if (!al_init_primitives_addon()) {
		fprintf(stderr, "Failed init_primitives\n");
		return -1;
	}
	al_init_font_addon();
	if (!al_install_mouse()) {
		fprintf(stderr, "Failed to install mouse\n");
		return -1;
	}
	if (!al_install_keyboard()) {
		fprintf(stderr, "Failed to install keyboard\n");
		return -1;
	}
	timer = al_create_timer(1.0 / FPS);
	if (!timer) {
		fprintf(stderr, "Failed to create timer\n");
		return -1;
	}
	move_tmr = al_create_timer(1.0 / 2);
	if (!move_tmr) {
		fprintf(stderr, "Failed to create move timer\n");
		return -1;
	}
	display = al_create_display(SCR_W, SCR_H);
	if (!display) {
		fprintf(stderr, "Failed to create display\n");
		al_destroy_timer(timer);
		al_destroy_timer(move_tmr);
		return -1;
	}
	ev_que = al_create_event_queue();
	if (!ev_que) {
		fprintf(stderr, "Failed to create event queue\n");
		al_destroy_timer(timer);
		al_destroy_timer(move_tmr);
		al_destroy_display(display);
		return -1;
	}

	plr_init(3, 3);
	wall_init(0, 0);
	tile_map_create();
	tilemap[player.x][player.y].obj = &player;

	al_register_event_source(ev_que, al_get_display_event_source(display));
	al_register_event_source(ev_que, al_get_timer_event_source(timer));
	al_register_event_source(ev_que, al_get_timer_event_source(move_tmr));
	al_register_event_source(ev_que, al_get_mouse_event_source());
	al_register_event_source(ev_que, al_get_keyboard_event_source());

	al_start_timer(timer);
	al_start_timer(move_tmr);

	while (1) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(ev_que, &ev);
		
		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			break;
		}
		if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) break;
			if (plr_action(ev.keyboard.keycode) == -1) {
				player.soul->alive = false;
			}
		}
		if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			mouse = ev.mouse.button;
			if (mouse == 2) {
				if (lee(player.x, player.y, selected_tile_x, selected_tile_y)) {	
					player.soul->on_route = true;
				} else {
					player.soul->on_route = false;
				}
			}
		}
		if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
			mouse = 0;
		}
		if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
			tilemap[selected_tile_x][selected_tile_y].selected = false;	
			
/*			printf("scroll x = %f, y = %f;\n", scroll_x, scroll_y);
			printf("mouse x = %d, y = %d;\n", ev.mouse.x, ev.mouse.y);
*/
			if (mouse == 1) {
				float x = ev.mouse.dx;
				float y = ev.mouse.dy;

				scroll_x -= x;
				scroll_y -= y;
			} else {
				selected_tile_x = (ev.mouse.x + (scroll_x - SCR_W/2)) / TILE_W;
				selected_tile_y = (ev.mouse.y + (scroll_y - SCR_H/2)) / TILE_H;

				if (selected_tile_x > MAP_W-1) selected_tile_x = MAP_W-1;
				if (selected_tile_x < 0) selected_tile_x = 0;
				if (selected_tile_y > MAP_H-1) selected_tile_y = MAP_H-1;
				if (selected_tile_y < 0) selected_tile_y = 0;
				
				tilemap[selected_tile_x][selected_tile_y].selected = true;
			}
		}
		if (player.soul->on_route == true && ev.timer.source == move_tmr
										  && ev.type == ALLEGRO_EVENT_TIMER) {
			plr_step();
		}
		if (ev.type == ALLEGRO_EVENT_TIMER) {
			redraw = true;
		}

		if (redraw && al_is_event_queue_empty(ev_que)) {
			redraw = false;

			nodes_refresh();
			
			tile_map_draw();

			al_flip_display();

			if (!player.soul->alive) {
				al_rest(3.0);
				break;
			}
		}
	}

	/* FREE DESTROY CLOSE */
	al_destroy_bitmap(tiles);
	al_destroy_timer(timer);
	al_destroy_timer(move_tmr);
	al_destroy_display(display);
	al_destroy_event_queue(ev_que);

	return 0;
}
