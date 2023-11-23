#pragma once

#include "../include/print.h"
#include <iostream>
#include <fstream>
#include <SDL_render.h>
#include <string>
#include <vector>
#include <cmath>
#include <SDL.h>
#include <unordered_map>
#include "color.h"
#include "imageloader.h"


const Color B = {0, 0, 0};
const Color W = {255, 255, 255};

const int mapWidth = 16;
const int mapHeight = 9;

const int BLOCK = 35; //Pixels por block de las paredes (ancho y alto)
const int MINIBLOCK = 20; //Pixeles por block del minimap

const int SCREEN_WIDTH = mapWidth * BLOCK;
const int SCREEN_HEIGHT = mapHeight * BLOCK;

//const int mapPosx = static_cast<int>(SCREEN_WIDTH/2 - (mapWidth/2 * MINIBLOCK));
//const int mapPosy = static_cast<int>(SCREEN_HEIGHT/2 - (static_cast<int>(mapHeight/2) * MINIBLOCK));
const int mapPosx = SCREEN_WIDTH/2 - (static_cast<int>(mapWidth/2) * MINIBLOCK);
const int mapPosy = SCREEN_HEIGHT/2 - (static_cast<int>(mapHeight/2) * MINIBLOCK);

struct Player {
  int x;
  int y;
  float a;
  float fov;
}; 

struct Impact {
  float d;
  std::string mapHit;  // + | -
  int tx;
};

class Raycaster {
public:
  Raycaster(SDL_Renderer* renderer)
    : renderer(renderer) {

    player.x = BLOCK + BLOCK / 2;
    player.y = BLOCK + BLOCK / 2;

    player.a = M_PI / 4.0f;
    player.fov = M_PI / 3.0f;

    scale = 128;
    tsize = 1024;
  }

  std::vector<std::string> map;

  void load_map(const std::string& filename) {
    map.clear();
    std::ifstream file(filename);
    std::string line;
    while (getline(file, line)) {
      map.push_back(line);
    }
    file.close();
  }

  void point(int x, int y, Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawPoint(renderer, x, y);
  }

  void rect(int x, int y, const std::string& mapHit, int level) {
    for(int cx = x; cx < x + MINIBLOCK; cx++) {
      for(int cy = y; cy < y + MINIBLOCK; cy++) {
        int tx = ((cx - x) * tsize) / MINIBLOCK;
        int ty = ((cy - y) * tsize) / MINIBLOCK;
        std::string key = std::to_string(level) + '+';

        Color c = ImageLoader::getPixelColor(key, tx, ty);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b , 255);
        SDL_RenderDrawPoint(renderer, cx, cy);
      }
    }
  }

  Impact cast_ray(float a) {
    float d = 0;
    std::string mapHit;
    int tx;

    while(true) {
      int x = static_cast<int>(player.x + d * cos(a)); 
      int y = static_cast<int>(player.y + d * sin(a)); 
      
      int i = static_cast<int>(x / BLOCK);
      int j = static_cast<int>(y / BLOCK);


      if (map[j][i] != ' ') {
        mapHit = map[j][i];

        int hitx = x - i * BLOCK;
        int hity = y - j * BLOCK;
        int maxhit;

        if (hitx == 0 || hitx == BLOCK - 1) {
          maxhit = hity;
        } else {
          maxhit = hitx;
        }

        tx = maxhit * tsize / BLOCK;

        break;
      }
      
      d += 1;
    }
    return Impact{d, mapHit, tx};
  }

  void draw_stake(int x, float h, Impact i, int level) {
    float start = SCREEN_HEIGHT/2.0f - h/2.0f;
    float end = start + h;

    for (int y = start; y < end; y++) {
      int ty = (y - start) * tsize / h;
      std::string key = std::to_string(level) + i.mapHit;
      Color c = ImageLoader::getPixelColor(key, i.tx, ty);
      SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

      SDL_RenderDrawPoint(renderer, x, y);
    }
  } 
 
  void render(bool openMinimap, bool& shooting, int level) {
    
    // draw right side of the screen
    
    for (int i = 0; i < SCREEN_WIDTH; i++) {
      double a = player.a + player.fov / 2.0 - player.fov * i / SCREEN_WIDTH;
      Impact impact = cast_ray(a);
      float d = impact.d;
      Color c = Color(255, 0, 0);

      int x = i;
      float h = static_cast<float>(SCREEN_HEIGHT/3)/static_cast<float>(d * cos(a - player.a)) * static_cast<float>(scale);
      draw_stake(x, h, impact, level);
    }

    if (shooting){
      ImageLoader::render(renderer, "pov2", SCREEN_WIDTH/2, SCREEN_HEIGHT - 128, 128, 128);
      shooting = false;
    } else {
      ImageLoader::render(renderer, "pov1", SCREEN_WIDTH/2, SCREEN_HEIGHT - 128, 128, 128);
    }

    // draw minimap
    
    if (openMinimap) {
      ImageLoader::render(renderer, "mmbg", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
      for (int x = mapPosx; x < mapPosx + mapWidth * MINIBLOCK; x += MINIBLOCK) {
        for (int y = mapPosy; y < mapPosy + mapHeight * MINIBLOCK; y += MINIBLOCK) {
          int i = static_cast<int>((x - mapPosx) / MINIBLOCK);
          int j = static_cast<int>((y - mapPosy) / MINIBLOCK);
          
          if (map[j][i] != ' ') {
            std::string mapHit;
            mapHit = map[j][i];
            Color c = Color(255, 0, 0);
            rect(x, y, mapHit, level);
          }
        }
      }
      ImageLoader::render(renderer, "mp", static_cast<int>((player.x / BLOCK) * MINIBLOCK) + mapPosx, static_cast<int>((player.y / BLOCK) * MINIBLOCK) + mapPosy, MINIBLOCK - 5, MINIBLOCK - 5);
    }
  }

  Player player;
private:
  int scale;
  SDL_Renderer* renderer;
  int tsize;
};