clang.exe -o DungeonCrawler.exe -I .\SDL2-2.28.5\include -Xlinker /subsystem:console DungeonCrawler.c gfx.c -L .\SDL2-2.28.5\lib\x64 -lSDL2 -lSDL2main -lshell32