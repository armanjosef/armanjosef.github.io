#include <SDL2/SDL.h>
#include <emscripten.h>
#include <stdlib.h>

typedef struct Context {
    SDL_Renderer *renderer;
    int iteration;
} Context;

void mainLoop(void *arg) {
    Context *ctx = (Context *) arg;
    SDL_Renderer *renderer = ctx->renderer;
    int iteration = ctx->iteration;

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect rect;
    rect.x = iteration;
    rect.y = iteration;
    rect.w = 50;
    rect.h = 50;
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);

    if (iteration == 255) {
        ctx->iteration = 0;
    } else {
        ctx->iteration++;
    }
}

int main() {
    SDL_Init (SDL_INIT_VIDEO);
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_CreateWindowAndRenderer(255, 255, 0, &window, &renderer);

    Context ctx;
    ctx.renderer = renderer;
    ctx.iteration = 0;

    int infinite_loop = 1;

    int fps =  -1;

    emscripten_set_main_loop_arg(mainLoop, &ctx, fps, infinite_loop);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}