// STL
#include <vector>
// SDL2
#include <SDL2/SDL.h>

extern "C" {

void processInput(bool& bRunning, bool& bReload) {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_QUIT: {
        bRunning = false;
        break;
      }
      case SDL_KEYDOWN: {
        switch(event.key.keysym.sym) {
          case SDLK_SPACE:
            bReload = true;
            break;
        }
      }
    }
  }
}

void update() {}

void render(SDL_Renderer* pRenderer) {
  SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(pRenderer);
  SDL_RenderPresent(pRenderer);
}

} // extern "C"

