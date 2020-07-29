// STL
#include <cstdlib>
#include <iostream>
// SDL2
#include <SDL2/SDL.h>

namespace SDL {
constexpr int SUCCESS = 0;
} // namespace SDL
constexpr auto cWidth = 800;
constexpr auto cHeight = 600;

void (*processInput)(bool &bRunning, bool &bReload);

void (*update)();

void (*render)(SDL_Renderer *pRenderer);

static void updateDLL() {
  static void *pDLL = nullptr;
  if(std::system("ninja") != 0) {
    return;
  }
  SDL_UnloadObject(pDLL);
  auto pObject = SDL_LoadObject("lib/libhot-reload-lib.so");
  if(pObject == nullptr) {
    throw std::runtime_error("Can not load so");
  }
  auto processInputFunc = SDL_LoadFunction(pObject, "processInput");
  if(processInputFunc == nullptr) {
    throw std::runtime_error("Can not load \"processInput\"");
  }
  auto updateFunc = SDL_LoadFunction(pObject, "update");
  if(updateFunc == nullptr) {
    throw std::runtime_error("Can not load \"update\"");
  }
  auto renderFunc = SDL_LoadFunction(pObject, "render");
  if(processInputFunc == nullptr) {
    throw std::runtime_error("Can not load \"render\"");
  }
  pDLL = pObject;
  processInput = reinterpret_cast<void(*)(bool&, bool&)>(processInputFunc);
  update       = reinterpret_cast<void(*)()>(updateFunc);
  render       = reinterpret_cast<void(*)(SDL_Renderer*)>(renderFunc);
  std::cout << "Load DLL successfully\n";
}

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) != SDL::SUCCESS) {
    std::cerr << "Unable to initialize SDL: " << SDL_GetError() << '\n';
    return EXIT_FAILURE;
  }

  auto pWindow = SDL_CreateWindow("Hotreload", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, 800, 600, 0);
  if (pWindow == nullptr) {
    std::cerr << "Unable to create SDL window: " << SDL_GetError() << '\n';
    return EXIT_FAILURE;
  }

  auto pRenderer = SDL_CreateRenderer(pWindow, 1, SDL_RENDERER_ACCELERATED);

  bool bReload  = true;
  bool bRunning = true;
  while (bRunning) {
    try {
      if(bReload) {
        updateDLL();
        bReload = false;
      }
    } catch(const std::runtime_error& exp) {
      std::cerr << exp.what() << '\n';
    }
    processInput(bRunning, bReload);
    update();
    render(pRenderer);
  }

  SDL_DestroyRenderer(pRenderer);
  SDL_DestroyWindow(pWindow);
  SDL_Quit();

  return EXIT_SUCCESS;
}
