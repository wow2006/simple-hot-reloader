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

using FirstInitializationFunc = void (*)(GameData *);
static FirstInitializationFunc firstInitialization;

using InitializeFunc = void (*)(GameData *);
static InitializeFunc initialize;

using ProcessInputFunc = void (*)(GameData *pData);
static ProcessInputFunc processInput;

using UpdateFunc = void (*)(GameData *pData);
static UpdateFunc update;

using RendererFunc = void (*)(const GameData *pData);
static RendererFunc renderer;

using CleanupFunc = void (*)(GameData *);
static CleanupFunc cleanup;

static void updateDLL() {
  static void *pDLL = nullptr;
  SDL_UnloadObject(pDLL);

  auto pObject = SDL_LoadObject("lib/libhot-reload-lib.so");
  if (pObject == nullptr) {
    throw std::runtime_error("Can not load so");
  }

  auto firstInitializationFunc = SDL_LoadFunction(pObject, "firstInitialization");
  if (firstInitializationFunc == nullptr) {
    throw std::runtime_error("Can not load \"firstInitialization\"");
  }

  auto initializeFunc = SDL_LoadFunction(pObject, "initialize");
  if (firstInitializationFunc == nullptr) {
    throw std::runtime_error("Can not load \"initialize\"");
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

  auto cleanupFunc = SDL_LoadFunction(pObject, "cleanup");
  if (cleanupFunc == nullptr) {
    throw std::runtime_error("Can not load \"cleanup\"");
  }

  pDLL = pObject;
  firstInitialization = reinterpret_cast<FirstInitializationFunc>(firstInitializationFunc);
  initialize          = reinterpret_cast<InitializeFunc>(initializeFunc);
  processInput        = reinterpret_cast<ProcessInputFunc>(processInputFunc);
  update              = reinterpret_cast<UpdateFunc>(updateFunc);
  renderer            = reinterpret_cast<RendererFunc>(renderFunc);
  cleanup             = reinterpret_cast<CleanupFunc>(cleanupFunc);

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

static void GameInitialization(GameData *pGameData) {
  if (SDL_Init(SDL_INIT_VIDEO) != SDL::SUCCESS) {
    std::cerr << "Unable to initialize SDL: " << SDL_GetError() << '\n';
  }

  const auto imgInit = IMG_Init(IMG_INIT_PNG);
  if ((imgInit & IMG_INIT_PNG) != IMG_INIT_PNG) {
    std::cerr << "Unable to initialize IMG: " << IMG_GetError() << '\n';
  }

  if (TTF_Init() < 0) {
    std::cerr << "Unable to initialize TTF: " << TTF_GetError() << '\n';
  }

  SDL_LogSetOutputFunction(LogOutputFunction, nullptr);

  auto pWindow = SDL_CreateWindow("Hotreload", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cWidth + 1, cHeight + 1, 0);
  if (pWindow == nullptr) {
    std::cerr << "Unable to create SDL window: " << SDL_GetError() << '\n';
  }
  pGameData->pWindow = pWindow;

  auto pRenderer = SDL_CreateRenderer(pWindow, 1, SDL_RENDERER_ACCELERATED);
  if (pRenderer == nullptr) {
    std::cerr << "Unable to create SDL Renderer: " << SDL_GetError() << '\n';
  }
  pGameData->pRenderer = pRenderer;
}

static void GameCleaning(GameData *pGameData) {
  SDL_DestroyRenderer(pGameData->pRenderer);
  SDL_DestroyWindow(pGameData->pWindow);

  IMG_Quit();
  TTF_Quit();
  SDL_Quit();
}

int main() {
  GameData gameData;
  GameInitialization(&gameData);

  // TODO(Hussein): get files to watch from json
  const auto libraryFilePath =
      std::filesystem::path(BASE_DIR) / "lib" / "library.cpp";
  WatchFile file({{libraryFilePath.c_str(), [&gameData]() {
                     if (std::system("ninja") == 0) {
                       gameData.reload = true;
                     }
                   }}});

  constexpr auto FPS = 60.F;
  constexpr auto FrameDelay = static_cast<uint32_t>(1000.F / FPS);
  uint32_t frameStart = 0;
  uint32_t frameTime = 0;

  try {
    updateDLL();
    firstInitialization(&gameData);
  } catch (const std::runtime_error &exp) {
    std::cerr << exp.what() << '\n';
  }

  gameData.running = true;
  while (gameData.running) {
    frameStart = SDL_GetTicks();
    try {
      if (gameData.reload) {
        cleanup(&gameData);
        updateDLL();
        initialize(&gameData);
        gameData.reload = false;
      }
    } catch (const std::runtime_error &exp) {
      std::cerr << exp.what() << '\n';
    }

    processInput(&gameData);
    update(&gameData);
    renderer(&gameData);

    frameTime = SDL_GetTicks() - frameStart;
    if (FrameDelay > frameTime) {
      SDL_Delay(FrameDelay - frameTime);
    }
    frameTime = SDL_GetTicks() - frameStart;
    char buffer[512];
    sprintf(buffer, "%d ms, %F", frameTime,
            1000.0 / static_cast<double>(frameTime));

    const SDL_Color White = {255, 255, 255, 255};
    auto &[pTexture, textRect] = gameData.debugInfo;
    SDL_DestroyTexture(pTexture);
    const auto pTextSurface =
        TTF_RenderText_Solid(gameData.pFont, buffer, White);
    TTF_SizeText(gameData.pFont, buffer, &textRect.w, &textRect.h);
    pTexture = SDL_CreateTextureFromSurface(gameData.pRenderer, pTextSurface);
    SDL_FreeSurface(pTextSurface);
  }

  GameCleaning(&gameData);

  return EXIT_SUCCESS;
}
