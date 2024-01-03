#define NORTH 8
#define SOUTH 4
#define WEST 2
#define EAST 1

#include "DungeonCrawler.h"

//Here we want to implement a function that given a tile index it return the position in the spritesheet
//If we need to return more than one value we use pointers at the value that we want to modify

float tile_scale = 1;
float camera_x = 0;
float camera_y = 0;
float camera_speed = 150;

int drawn_tile_counter = 0;

typedef struct DungeonGlyph
{
    SDL_Texture* texture;
    SDL_Rect rect;
    int advance;
}DungeonGlyph;

DungeonGlyph dungeon_glyphs[95];

int dungeon_gfx_get_tile(const int offset_x, const int offset_y, const int spritesheet_w, const int spritesheet_h, const int tile_w, const int tile_h,int* x, int* y)
{
    if (offset_x < 0 || offset_y < 0)
    {
        return -1;
    }

    int x_pos = tile_w * offset_x;
    int y_pos = tile_h * offset_y;

    if (x_pos > spritesheet_w - tile_w || y_pos > spritesheet_h - tile_h)
    {
        return -1;
    }

    *x = x_pos;
    *y = y_pos;

    return 0;
}

int dungeon_gfx_draw_actor(SDL_Renderer* renderer, SDL_Texture* texture, actor* actor)
{
    //When we want the function to fill data with pointers, you need to instantiate the variable without a pointer 
    //and than pass the address (&) to the function

    dungeon_gfx_draw_tile(renderer, texture, actor->x, actor->y, actor->tile_x, actor->tile_y);

    return 0;
}

int dungeon_gfx_draw_tile(SDL_Renderer* renderer, SDL_Texture* texture, const float pos_x, const float pos_y,const int tile_x_index, const int tile_y_index)
{
    SDL_FRect destination_rect = { pos_x - camera_x, pos_y - camera_y, 16 * tile_scale, 16 * tile_scale}; //create a rectangle (x pos ,y pos, width final size, height final size)
    
    SDL_FRect camera_rect = {0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H};

    if (!SDL_HasIntersectionF(&destination_rect, &camera_rect))
    {
        return 0;
    }
    
    int width;
    int height;

    SDL_QueryTexture(texture, NULL, NULL, &width, &height);

                                                                //If we want to have a more fluid movement we add +0.5 to .x and .y so we will draw in the next pixel instead to have the number break from the casting of float to int
    SDL_Rect tile_rect = { 0, 0, TILE_W, TILE_H};

    dungeon_gfx_get_tile(tile_x_index, tile_y_index, width, height, TILE_W, TILE_H, &tile_rect.x, &tile_rect.y);
    SDL_RenderCopyF(renderer, texture, &tile_rect, &destination_rect);
    
    drawn_tile_counter++;
    
    return 0;
}

int dungeon_gfx_load_font(const char* path, SDL_Renderer* renderer)
{
    void* font_bytes = SDL_LoadFile(path, NULL);

    int fonts_number = stbtt_GetNumberOfFonts((unsigned char*)font_bytes);
    SDL_Log("Number of fonts: %d", fonts_number);

    stbtt_fontinfo font_info;
    stbtt_InitFont(&font_info, font_bytes, 0);

    const float font_scale = stbtt_ScaleForPixelHeight(&font_info, 36); //scale datas for our screen, font_info, height for what screen we are working on
    SDL_Log("Font Scale: %f", font_scale);

    for (int i = 0; i < 96; i++)
    {
        int glyph_index = stbtt_FindGlyphIndex(&font_info, i + 32);     //88 stands for X
        
        stbtt_GetGlyphHMetrics(&font_info, glyph_index, &dungeon_glyphs[i].advance, NULL);  //take infos for glyph
        dungeon_glyphs[i].advance *= font_scale;
        
        const unsigned char* font_pixels = stbtt_GetGlyphBitmap(&font_info, font_scale, font_scale, glyph_index, &dungeon_glyphs[i].rect.w, &dungeon_glyphs[i].rect.h, &dungeon_glyphs[i].rect.x, &dungeon_glyphs[i].rect.y);
        
        if(dungeon_glyphs[i].rect.w == 0 || dungeon_glyphs[i].rect.h == 0) 
        {
            continue;
        }
        
        //dungeon_glyphs[i].rect.w *= font_scale;
        //dungeon_glyphs[i].rect.h *= font_scale;
        //dungeon_glyphs[i].rect.x *= font_scale;
        //dungeon_glyphs[i].rect.y *= font_scale;
        //SDL_Log("Width = %d, Height = %d, x_offset = %d, y_offset = %d",dungeon_glyphs[i].rect.w, dungeon_glyphs[i].rect.h, dungeon_glyphs[i].rect.x, dungeon_glyphs[i].rect.y);

        const size_t memory_size = (size_t)(dungeon_glyphs[i].rect.w * dungeon_glyphs[i].rect.h * 4);
        unsigned char* font_memory = SDL_malloc(memory_size);
        
        for (int j = 0; j < dungeon_glyphs[i].rect.w * dungeon_glyphs[i].rect.h; j++)
        {
            font_memory[j * 4] = 255;
            font_memory[j * 4 + 1] = 255;
            font_memory[j * 4 + 2] = 255;
            font_memory[j * 4 + 3] = font_pixels[j];
        }
        
        stbtt_FreeBitmap((unsigned char*)font_pixels, NULL);
        
        dungeon_glyphs[i].texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, dungeon_glyphs[i].rect.w, dungeon_glyphs[i].rect.h); //made this global and exter in dc.c
        
        if (!dungeon_glyphs[i].texture)
        {
            SDL_Log("Error creating Font texture : %s", SDL_GetError());
            SDL_free(font_memory);
            SDL_free(font_bytes);
            
            return -1;
        }

        SDL_SetTextureBlendMode(dungeon_glyphs[i].texture, SDL_BLENDMODE_BLEND);
        SDL_UpdateTexture(dungeon_glyphs[i].texture, NULL, font_memory, dungeon_glyphs[i].rect.w * 4);

        SDL_free(font_memory);
    }
    
    SDL_free(font_bytes);

    return 0;
}

int dungeon_gfx_draw_text(SDL_Renderer* renderer, const float x, const float y, const char* str, const text_color* color)
{
    SDL_Rect src = { 0, 0, 0, 0};
    SDL_FRect dest = { 0, 0, 0, 0};

    float current_x = x;
    float current_y = y;

    for (int i = 0; str[i] != 0; i++)
    {
        DungeonGlyph* current_glyph = &(dungeon_glyphs[str[i] - 32]);
        
        dest.x = current_x + current_glyph->rect.x;
        dest.y = current_y + current_glyph->rect.y;
        dest.w = current_glyph->rect.w;
        dest.h = current_glyph->rect.h;

        src.w = current_glyph->rect.w;
        src.h = current_glyph->rect.h;

        SDL_SetTextureColorMod(current_glyph->texture, color->r, color->g, color->b);
        SDL_RenderCopyF(renderer, current_glyph->texture, &src, &dest);

        current_x += current_glyph->advance;
    }

    return 0;
}

int dungeon_gfx_draw_room(SDL_Renderer* renderer, SDL_Texture* texture, const float x, const float y, const unsigned char bitmask)
{
    const int room_grid_width = 8;
    const int room_grid_height = 8;

    for (int i = 0; i < room_grid_height; i++)
    {
        for (int j = 0; j < room_grid_width; j++)
        {
            dungeon_gfx_draw_tile(renderer, texture, x + j * TILE_W * tile_scale, y + i * TILE_H * tile_scale, 9,6);
        }
    }

    if (bitmask & NORTH)
    {
        for (int i = 0; i < room_grid_width; i++)
        {
            dungeon_gfx_draw_tile(renderer, texture, x + i * TILE_W * tile_scale, y, 9,7);
        }
    }
    if (bitmask & SOUTH)
    {
        for (int i = 0; i < room_grid_width; i++)
        {
            dungeon_gfx_draw_tile(renderer, texture, x + i * TILE_W * tile_scale, y + (room_grid_height - 1) * tile_scale * TILE_H, 9,7);
        }
    }
    if (bitmask & WEST)
    {
        for (int i = 0; i < room_grid_height; i++)
        {
            dungeon_gfx_draw_tile(renderer, texture, x, y + i * TILE_H * tile_scale, 9,7);
        }
    }
    if (bitmask & EAST)
    {
        for (int i = 0; i < room_grid_height; i++)
        {
            dungeon_gfx_draw_tile(renderer, texture, x + (room_grid_width - 1) * TILE_W * tile_scale, y + i * TILE_H * tile_scale, 9,7);
        }
    }

    return 0;
}