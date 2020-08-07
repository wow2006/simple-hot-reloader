// STL
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
// SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
// Internal
#include "../lib/library.hpp"
#include "watchFile.hpp"

namespace SDL {
constexpr int SUCCESS = 0;
} // namespace SDL
constexpr auto cWidth = 800;
constexpr auto cHeight = 600;

using ProcessInputFunc = void (*)(GameData *pData);
static ProcessInputFunc processInput;

using UpdateFunc = void (*)(GameData *pData);
static UpdateFunc update;

using RendererFunc = void (*)(SDL_Renderer *pRenderer, const GameData *pData);
static RendererFunc renderer;

static void updateDLL() {
  static void *pDLL = nullptr;
  SDL_UnloadObject(pDLL);
  auto pObject = SDL_LoadObject("lib/libhot-reload-lib.so");
  if (pObject == nullptr) {
    throw std::runtime_error("Can not load so");
  }
  auto processInputFunc = SDL_LoadFunction(pObject, "processInput");
  if (processInputFunc == nullptr) {
    throw std::runtime_error("Can not load \"processInput\"");
  }
  auto updateFunc = SDL_LoadFunction(pObject, "update");
  if (updateFunc == nullptr) {
    throw std::runtime_error("Can not load \"update\"");
  }
  auto renderFunc = SDL_LoadFunction(pObject, "render");
  if (processInputFunc == nullptr) {
    throw std::runtime_error("Can not load \"render\"");
  }
  pDLL = pObject;
  processInput = reinterpret_cast<ProcessInputFunc>(processInputFunc);
  update = reinterpret_cast<UpdateFunc>(updateFunc);
  renderer = reinterpret_cast<RendererFunc>(renderFunc);

  const time_t curr_time = time(nullptr);
  const tm *tm_local = localtime(&curr_time);
  std::cout << tm_local->tm_hour << ":" << tm_local->tm_min << ":"
            << tm_local->tm_sec << " Load DLL successfully\n";
}

static void LogOutputFunction(void *userdata, int category,
                              SDL_LogPriority priority, const char *message) {
  (void)userdata;
  (void)category;
  (void)priority;
  std::cerr << "SDL_Message: " << message << '\n';
}

static auto loadImage(const char *path, SDL_Renderer *pRenderer)
    -> SDL_Texture * {
  const auto pImage = IMG_Load(path);
  if (pImage == nullptr) {
    fprintf(stderr, "Could not load image: %s\n", IMG_GetError());
    return nullptr;
  }

  const auto pTexture = SDL_CreateTextureFromSurface(pRenderer, pImage);
  if (pTexture == nullptr) {
    fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
  }

  SDL_FreeSurface(pImage);
  return pTexture;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  if (SDL_Init(SDL_INIT_VIDEO) != SDL::SUCCESS) {
    std::cerr << "Unable to initialize SDL: " << SDL_GetError() << '\n';
    return EXIT_FAILURE;
  }

  const auto imgInit = IMG_Init(IMG_INIT_PNG);
  if ((imgInit & IMG_INIT_PNG) != IMG_INIT_PNG) {
    std::cerr << "Unable to initialize IMG: " << IMG_GetError() << '\n';
    return EXIT_FAILURE;
  }

  if (TTF_Init() < 0) {
    std::cerr << "Unable to initialize TTF: " << TTF_GetError() << '\n';
    return EXIT_FAILURE;
  }

  GameData gameData;
  gameData.reload = true;

  const auto libraryFilePath =
      std::filesystem::path(BASE_DIR) / "lib" / "library.cpp";
  WatchFile file({{libraryFilePath.c_str(), [&gameData]() {
                     if (std::system("ninja") == 0) {
                       gameData.reload = true;
                     }
                   }}});

  SDL_LogSetOutputFunction(LogOutputFunction, nullptr);

  auto pWindow =
      SDL_CreateWindow("Hotreload", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, cWidth + 1, cHeight + 1, 0);
  if (pWindow == nullptr) {
    std::cerr << "Unable to create SDL window: " << SDL_GetError() << '\n';
    return EXIT_FAILURE;
  }

  auto pRenderer = SDL_CreateRenderer(pWindow, 1, SDL_RENDERER_ACCELERATED);
  if (pRenderer == nullptr) {
    std::cerr << "Unable to create SDL Renderer: " << SDL_GetError() << '\n';
    return EXIT_FAILURE;
  }

  const auto pAppleTexture = loadImage("apple.png", pRenderer);
  if (pAppleTexture == nullptr) {
    return EXIT_FAILURE;
  }
  gameData.pApple = pAppleTexture;

  const auto pTexture = loadImage("snakeHead.png", pRenderer);
  if (pTexture == nullptr) {
    return EXIT_FAILURE;
  }
  gameData.pSnake = pTexture;
  gameData.snakeBody.reserve(8);

  std::mt19937 rng(16);
  std::uniform_int_distribution<int> gen(
      0, gameData.width / gameData.step); // uniform, unbiased

  for (auto &apple : gameData.apples) {
    const auto index = gen(rng) * gameData.step;
    apple = {
        index,
        1,
    };
  }

  const SDL_Color White = {255, 255, 255, 255};
  gameData.pFont = nullptr;
  const std::string fontfile = "ubuntu.ttf";
  if (!(gameData.pFont = TTF_OpenFont(fontfile.c_str(), 16))) {
    std::cerr << "Error: Unable to open font\n";
    return EXIT_FAILURE;
  }

  constexpr auto FPS = 60.F;
  constexpr auto FrameDelay = static_cast<uint32_t>(1000.F / FPS);
  uint32_t frameStart = 0;
  uint32_t frameTime = 0;

  gameData.mTextures.resize(2);

  gameData.running = true;
  while (gameData.running) {
    frameStart = SDL_GetTicks();
    try {
      if (gameData.reload) {
        updateDLL();
        gameData.reload = false;
      }
    } catch (const std::runtime_error &exp) {
      std::cerr << exp.what() << '\n';
    }

    processInput(&gameData);
    update(&gameData);
    renderer(pRenderer, &gameData);

    frameTime = SDL_GetTicks() - frameStart;
    if (FrameDelay > frameTime) {
      SDL_Delay(FrameDelay - frameTime);
    }
    frameTime = SDL_GetTicks() - frameStart;
    char buffer[512];
    sprintf(buffer, "%d ms, %F", frameTime, 1000.0 / static_cast<double>(frameTime));

    auto& [pTexture, textRect] = gameData.mTextures.front();
    SDL_DestroyTexture(pTexture);
    const auto pTextSurface = TTF_RenderText_Solid(gameData.pFont, buffer, White);
    TTF_SizeText(gameData.pFont, buffer, &textRect.w, &textRect.h);
    pTexture = SDL_CreateTextureFromSurface(pRenderer, pTextSurface);
    SDL_FreeSurface(pTextSurface);
  }


  for(const auto& [pTexture, _] : gameData.mTextures) {
    SDL_DestroyTexture(pTexture);
  }
  SDL_DestroyTexture(gameData.pApple);
  SDL_DestroyTexture(gameData.pSnake);
  SDL_DestroyRenderer(pRenderer);
  SDL_DestroyWindow(pWindow);
  IMG_Quit();
  TTF_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;
}
