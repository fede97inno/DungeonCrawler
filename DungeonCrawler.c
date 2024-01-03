#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define ROOM_W_NUMBER 20
#define ROOM_H_NUMBER 20

#include "DungeonCrawler.h"
#include "maze.h"

extern float tile_scale;
extern float camera_x;
extern float camera_y;
extern float camera_speed;
extern int drawn_tile_counter;

const text_color red_text = {255, 0, 0};
const text_color white_text = {255, 255, 255};

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("Error init SLD2 : %s", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("GameWindow", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_SIZE_W, WINDOW_SIZE_H, 0);

    if (!window)
    {
        SDL_Log("Error creating SDL2 Window : %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); // | -> logic or, it bitmask with or

    if (!renderer)
    {
        SDL_Log("Error in creating SDL2 Renderer : %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    SDL_Log("Hello SDL2!");
    
    //LOAD AN IMAGE:
    //unsigned char *data = stbi_load(filename, &x, &y, &n, 0);     
    //stbi_load return memory block pointer on heap
    //at the end we need to free the memory on the heap  stbi_image_free(data)
    //x = width, y = height, n = # 8-bit components per pixel 4 in our case RGBA
    //at this point we have only metadata w-h-n when we want to allocate memory on gpu than we map the gpu memory texture object
    
    //staging buffer is a block of memory exposed at gpu but on the system RAM and gpu can access them with
    //bus mastering, SDL abstract this process and do this with SDL_UpdateTexture();
    
    int width, height, channels;
    unsigned char *pixels = stbi_load("2D Pixel Dungeon Asset Pack/character and tileset/Dungeon_Tileset.png", &width, &height, &channels, 4);

    if (!pixels)
    {
        SDL_Log("Error loading image");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    SDL_Log("Image Info:\nwidth : %d\nheight : %d\nchannels : %d\n", width, height, channels);

    //static if we want to rarely move memory to gpu, streaming if we want to do this often
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
    
    if (!texture)
    {
        SDL_Log("Error creating texture : %s", SDL_GetError());
        stbi_image_free(pixels);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    //SDL_UpdateTexture(texture, what to update if NULL means the entire of texture, pointer to data, horizontal line till the end of the image);
    if(SDL_UpdateTexture(texture, NULL, pixels, width * 4))
    {
        SDL_Log("Error updating texture : %s", SDL_GetError());
        stbi_image_free(pixels);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    stbi_image_free(pixels);

    const char* font_path = "IMMORTAL.ttf";
    dungeon_gfx_load_font(font_path, renderer);

    int window_opened = 1;
    
    actor actor_gold_key = { 0, 0 , 100, 9, 9 };
    actor actor_silver_key = { 0, 0, 150, 8, 8 };
    //CALCULATE DELTATIME:
    //we need to information to calculate the time, both are exposed by SDL lib, we use a performance counter
    //it's a timer that can only increase his value;
    //with PerformanceCounter we access the current clock
    //with PerformanceFrequency we access to how many unit of the counter there are in a second (const value)

    const Uint64 clock_frequency = SDL_GetPerformanceFrequency(); //how many clocks for making a second, it's a const this value doesn't change during the execution 
    Uint64 clock_now;
    double previous_frame_time = SDL_GetPerformanceCounter();
    double deltatime;

    while (window_opened)
    {
        drawn_tile_counter = 0;
        clock_now = SDL_GetPerformanceCounter();
        double actual_frame_time = (double)clock_now;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                window_opened = 0;
            }

            if (event.type == SDL_MOUSEWHEEL)
            {
                tile_scale += event.wheel.y * deltatime;
                if (tile_scale < 0.1)
                {
                    tile_scale = 0.1;
                }
                
            }
            // if(event.type == SDL_KEYDOWN)    //we dont need this, input goes into the array of SDL_GetKeyboardState(NULL);
            // {
            //     if(event.key.keysym.sym == SDLK_UP)     //here stay the symbol that has been pressed
            //     {
            //         rect_y--;                           
            //     }
            //     if(event.key.keysym.sym == SDLK_DOWN)
            //     {
            //         rect_y++;
            //     }
            //     if(event.key.keysym.sym == SDLK_LEFT)
            //     {
            //         rect_x--;
            //     }
            //     if(event.key.keysym.sym == SDLK_RIGHT)
            //     {
            //         rect_x++;     
            //     }
            // }
        }
        
        const Uint8 *inputs_array = SDL_GetKeyboardState(NULL); //return an array with pressed keys: 1 if pressed, 0 not pressed
        if (inputs_array[SDL_SCANCODE_UP])
        {
            actor_gold_key.y -= actor_gold_key.speed * deltatime;       
        }
        if (inputs_array[SDL_SCANCODE_DOWN])
        {
            actor_gold_key.y += actor_gold_key.speed * deltatime;       
        }
        if (inputs_array[SDL_SCANCODE_LEFT])
        {
            actor_gold_key.x -= actor_gold_key.speed * deltatime;       
        }
        if (inputs_array[SDL_SCANCODE_RIGHT])
        {
            actor_gold_key.x += actor_gold_key.speed * deltatime;       
        }

        if (inputs_array[SDL_SCANCODE_W])
        {
            actor_silver_key.y -= actor_silver_key.speed * deltatime;  
            camera_y -= camera_speed * deltatime;    
        }
        if (inputs_array[SDL_SCANCODE_S])
        {
            actor_silver_key.y += actor_silver_key.speed * deltatime;     
            camera_y += camera_speed * deltatime;    
        }
        if (inputs_array[SDL_SCANCODE_A])
        {
            actor_silver_key.x -= actor_silver_key.speed * deltatime;   
            camera_x -= camera_speed * deltatime;    
        }
        if (inputs_array[SDL_SCANCODE_D])
        {
            actor_silver_key.x += actor_silver_key.speed * deltatime;  
            camera_x += camera_speed * deltatime;     
        }

        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < ROOM_H_NUMBER; i++)
        {
            for (int j = 0; j < ROOM_W_NUMBER; j++)
            {
                  dungeon_gfx_draw_room(renderer, texture, j * TILE_W * 8 * tile_scale, i * TILE_H * 8 * tile_scale, maze[i][j]); //[y][x]
            }
        }
        
        dungeon_gfx_draw_actor(renderer, texture, &actor_gold_key);

        dungeon_gfx_draw_actor(renderer, texture, &actor_silver_key);

        //dungeon_gfx_draw_text(renderer, 200, 200, "Red Text Exemple!0", &red_text);
        //dungeon_gfx_draw_text(renderer, 200, 236, "White Text Exemple!1", &white_text);


        SDL_RenderPresent(renderer);                        //swap from back and front buffer

        SDL_Log("Tile drawn : %d", drawn_tile_counter);

        deltatime = (actual_frame_time - previous_frame_time) / clock_frequency;
        previous_frame_time = actual_frame_time;

        //SDL_Log("delta time : %f", deltatime);    //log for deltatime
    }
    
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}