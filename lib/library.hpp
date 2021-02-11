#pragma once
// STL
#include <array>
#include <vector>
#include <string>
#include <unordered_map>
// SDL2
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

enum class Direction : uint8_t { None, Up, Down, Left, Right };
enum class Movement  : uint8_t { None, Forward, Backword, Left, Right };
enum class GameState : uint8_t { Menu, Game, Dead };

struct TextureData {
  SDL_Texture* pTexture = nullptr;
  SDL_Rect     rect = {};
};

struct MapData {
  uint32_t width;
  uint32_t height;
  float    cellWidth;
  float    cellHeight;
};

struct MenuItem {
  std::string  text;
  SDL_Texture* pTexture = nullptr;
  SDL_Rect     textRect = {};
};

struct GameData {
  bool running    = false;
  bool reload     = false;
  bool bShowDebug = true;

  int speed  = 1;
  int step   = 40;
  int width  = 800;
  int height = 600;

  SDL_Window   *pWindow   = nullptr;
  SDL_Renderer *pRenderer = nullptr;

  TTF_Font *pFont     = nullptr;
  TTF_Font *pMenuFont = nullptr;
  Uint64 score = 0;

  SDL_Color backgroundColor = SDL_Color{16, 16, 32, SDL_ALPHA_OPAQUE};
  SDL_Color forgroundColor  = SDL_Color{128, 128, 128, SDL_ALPHA_OPAQUE};

  Direction direction = Direction::Right;
  Movement  movement  = Movement::None;
  SDL_Point snakeHead = {1, 1};
  std::vector<SDL_Point> snakeBody;

  TextureData debugInfo;

  std::array<SDL_Point, 5> borders {
    SDL_Point{0, 0},
    SDL_Point{width, 0},
    SDL_Point{width, height},
    SDL_Point{0, height},
    SDL_Point{0, 0}
  };

  std::array<SDL_Point, 8> apples;

  std::unordered_map<std::string, SDL_Texture*> m_mTextures;
  std::array<MenuItem, 2> m_aMenu = {
    MenuItem{"Start"},
    MenuItem{"Quit"}
  };

  GameState state = GameState::Menu;

  SDL_Point mousePosition;
};

