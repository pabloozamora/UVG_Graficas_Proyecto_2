#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <SDL_mixer.h>
#include "../include/print.h"
#include "color.h"
#include "imageloader.h"
#include "raycaster.h"
#include "fps.h"
#include "../include/FastNoiseLite.h"

SDL_Window* window;
SDL_Renderer* renderer;

bool mainTitle = true;
bool musicOn = false;
bool win = false;
int level = 1;
bool openMinimap = false;
bool shooting = false;

FastNoiseLite floorNoise;

void clear() {
  SDL_SetRenderDrawColor(renderer, 56, 56, 56, 255);
  SDL_RenderClear(renderer);
}

//Intento de tiles

/*void draw_floor() {
    Color floorColor(255, 255, 255, 255);// floor color
    const int floorTile = 40; // Pixels in a floor tile
    for (int x = 0; x <= SCREEN_WIDTH; x += floorTile) {
        floorColor = floorColor * -1;
        for (int y = SCREEN_HEIGHT / 2; y <= SCREEN_HEIGHT; y += floorTile) {
            floorColor = floorColor * -1;
            // Set the draw color inside the loop to change color for each tile
            SDL_SetRenderDrawColor(renderer, floorColor.r, floorColor.g, floorColor.b, 255);
            SDL_Rect rect = {
                x,
                y,
                floorTile, // Width of the floor tile
                floorTile  // Height of the floor tile
            };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}*/

// Intento de usar noise para patrones de suelo

/* void draw_floor(SDL_Renderer* renderer, int ox, int oy) {
  Color floorColor(120, 55, 38, 255);
  floorNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  floorNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
  floorNoise.SetFrequency(4.0f);
  float zoom = 200.0f;

  for (int x = 0; x <= SCREEN_WIDTH; x++){
    for (int y = SCREEN_HEIGHT/2; y <= SCREEN_HEIGHT; y++){
      float noise = floorNoise.GetNoise(x * zoom + ox, y * zoom + oy);
      floorColor = floorColor * noise;
      SDL_SetRenderDrawColor(renderer, floorColor.r, floorColor.g, floorColor.b, floorColor.a);
      SDL_RenderDrawPoint(renderer, x, y);
    }
  }
  

} */

void draw_floor() {
  // floor color
  SDL_SetRenderDrawColor(renderer, 121, 56, 136, 255);
  SDL_Rect rect = {
    0, 
    SCREEN_HEIGHT / 2,
    SCREEN_WIDTH,
    SCREEN_HEIGHT
  };
  SDL_RenderFillRect(renderer, &rect);
}


int main(int argv, char** args) {

  SDL_Init(SDL_INIT_VIDEO);
  ImageLoader::init();
  Mix_Init(MIX_INIT_MP3);

  // Sonidos
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
  Mix_Music * soundtrack = Mix_LoadMUS("../assets/soundtrack.mp3");
  Mix_Chunk * selectSound = Mix_LoadWAV("../assets/selectsound.mp3");
  Mix_Chunk * gunshot = Mix_LoadWAV("../assets/gunshot.mp3");
  Mix_Chunk * success = Mix_LoadWAV("../assets/success.mp3");

  // Soundtrack
  //Mix_PlayMusic(soundtrack, -1);

  window = SDL_CreateWindow("Proyecto 2: Raycasting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // Nivel 1

  ImageLoader::loadImage("1+", "../assets/wall3.png");
  ImageLoader::loadImage("1-", "../assets/wall1.png");
  ImageLoader::loadImage("1|", "../assets/wall2.png");
  ImageLoader::loadImage("1*", "../assets/wall4.png");
  ImageLoader::loadImage("1g", "../assets/goal.png");

  // Nivel 2
  ImageLoader::loadImage("2+", "../assets/wall2_3.png");
  ImageLoader::loadImage("2-", "../assets/wall2_1.png");
  ImageLoader::loadImage("2|", "../assets/wall2_2.png");
  ImageLoader::loadImage("2*", "../assets/wall2_4.png");
  ImageLoader::loadImage("2g", "../assets/goal2.png");

  // Nivel 3
  ImageLoader::loadImage("3+", "../assets/wall3_3.png");
  ImageLoader::loadImage("3-", "../assets/wall3_1.png");
  ImageLoader::loadImage("3|", "../assets/wall3_2.png");
  ImageLoader::loadImage("3*", "../assets/wall3_4.png");
  ImageLoader::loadImage("3g", "../assets/goal3.png");

  // Pantallas
  ImageLoader::loadImage("main1", "../assets/maintitle1.png");
  ImageLoader::loadImage("main2", "../assets/maintitle2.png");
  ImageLoader::loadImage("main3", "../assets/maintitle3.png");
  ImageLoader::loadImage("success", "../assets/success.png");

  ImageLoader::loadImage("mp", "../assets/mapPlayer.png");
  ImageLoader::loadImage("mmbg", "../assets/mmbackground.png");

  ImageLoader::loadImage("pov1", "../assets/gun1.png");
  ImageLoader::loadImage("pov2", "../assets/gun2.png");


  Raycaster r = { renderer };
  r.load_map("../assets/map1.txt");

  clear();


  bool running = true;
  int speed = 10;
  while(running) {
        startFPS();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym ){
                case SDLK_a:
                    if (!mainTitle) {
                      r.player.a += 3.14/24;
                      if (r.player.a > 2 * M_PI) r.player.a -= 2 * M_PI;
                    }
                    break;
                case SDLK_d:
                    if (!mainTitle) {
                      r.player.a -= 3.14/24;
                      if (r.player.a < 2 * M_PI) r.player.a -= 2 * M_PI;
                    }
                    break;
                case SDLK_w:
                    if (mainTitle) {
                      if (level > 1) {
                        Mix_PlayChannel(0, selectSound, 0);
                        level--;
                      }
                    } else if (!win) {

                      int nextX =  r.player.x + 2 *speed * cos(r.player.a);
                      int nextY = r.player.y + 2 * speed * sin(r.player.a);

                      if (r.map[nextY / BLOCK][nextX / BLOCK] == ' '){
                        r.player.x += speed * cos(r.player.a);
                        r.player.y += speed * sin(r.player.a);
                      } else if (r.map[nextY / BLOCK][nextX / BLOCK] == 'g') {
                        win = true;
                        Mix_HaltMusic();
                        Mix_PlayChannel(0, success, 0);
                        musicOn = false;
                      }
                      //SDL_Log("x: %d, y: %d", static_cast<int>((r.player.x / BLOCK) * MINIBLOCK) + mapPosx, static_cast<int>((r.player.y / BLOCK) * MINIBLOCK) + mapPosy);
                    }
                    break;
                case SDLK_s:
                    if (mainTitle) {
                      if (level < 3) {
                        Mix_PlayChannel(0, selectSound, 0);
                        level ++;
                      }
                    } else if (!win) {

                      int nextX =  r.player.x - speed * cos(r.player.a);
                      int nextY = r.player.y - speed * sin(r.player.a);

                      if (r.map[nextY / BLOCK][nextX / BLOCK] == ' '){
                        r.player.x -= speed * cos(r.player.a);
                        r.player.y -= speed * sin(r.player.a);
                      } 
                      //SDL_Log("x: %d, y: %d", static_cast<int>((r.player.x / BLOCK) * MINIBLOCK) + mapPosx, static_cast<int>((r.player.y / BLOCK) * MINIBLOCK) + mapPosy);
                    }
                    break;
                case SDLK_m:
                    if (!mainTitle && !win) {
                      openMinimap = !openMinimap;
                    }
                    break;
                case SDLK_RETURN:
                    if (mainTitle) {
                      mainTitle = false;
                    }
                    break;
                case SDLK_ESCAPE:
                    if (win) {
                      if (level < 3) level++;
                      mainTitle = true;
                    }
                    break;
                default:
                    break;
                }
              }
            if (event.type == SDL_MOUSEMOTION) {
              int mouseX, mouseY;
              SDL_GetMouseState(&mouseX, &mouseY);
              if (mouseX >= SCREEN_WIDTH - SCREEN_WIDTH/3) {
                if (!mainTitle) {
                      r.player.a -= 3.14/24;
                      if (r.player.a > 2 * M_PI) r.player.a -= 2 * M_PI;
                    }
                break;
              }
              if (mouseX <= SCREEN_WIDTH/3) {
                if (!mainTitle) {
                      r.player.a += 3.14/24;
                      if (r.player.a < 2 * M_PI) r.player.a -= 2 * M_PI;
                    }
                break;
              }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
              if (event.button.button == SDL_BUTTON_LEFT) {
                if (!mainTitle) {
                  Mix_PlayChannel(0, gunshot, 0);
                  shooting = true;
                }
              }
              break;
            }
          }

        clear();

        std::string currentLevel = "main" + std::to_string(level);

        if (!mainTitle) {
          draw_floor();
          r.render(openMinimap, shooting, level);
          if (win) {
            ImageLoader::render(renderer, "success", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
          }
        } else {
          ImageLoader::render(renderer, currentLevel, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
          win = false;
          r.player.x = BLOCK + BLOCK / 2;
          r.player.y = BLOCK + BLOCK / 2;
          r.player.a = M_PI / 4.0f;
          r.player.fov = M_PI / 3.0f;
          if (!musicOn) {
            musicOn = true;
            Mix_PlayMusic(soundtrack, -1);
          }
        }

        // render

        SDL_RenderPresent(renderer);
        endFPS(window);
    }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}