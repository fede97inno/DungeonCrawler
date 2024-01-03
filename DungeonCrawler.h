#define TILE_W 16
#define TILE_H 16
#define WINDOW_SIZE_W 512
#define WINDOW_SIZE_H 512

#include <SDL.h>
#include "stb_image.h"
#include "stb_truetype.h"

typedef struct actor 
{
    float x;
    float y;
    float speed;
    int tile_x;
    int tile_y;
}actor;

typedef struct text_color{
    Uint8 r;
    Uint8 g;
    Uint8 b;
}text_color;

int dungeon_gfx_get_tile(const int offset_x, const int offset_y, const int spritesheet_w, const int spritesheet_h, const int tile_w, const int tile_h,int* x, int* y);
int dungeon_gfx_draw_actor(SDL_Renderer* renderer, SDL_Texture* texture, actor* actor);
int dungeon_gfx_draw_tile(SDL_Renderer* renderer, SDL_Texture* texture, const float pos_x, const float pos_y,const int tile_x_index, const int tile_y_index);
int dungeon_gfx_load_font(const char* path, SDL_Renderer* renderer);
int dungeon_gfx_draw_text(SDL_Renderer* renderer, const float x, const float y, const char* str, const text_color* color);
int dungeon_gfx_draw_room(SDL_Renderer* renderer, SDL_Texture* texture, const float x, const float y, const unsigned char bitmask);