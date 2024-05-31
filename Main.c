#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#define TITLE "M3U"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FRAME_LIMIT 60
#define FRAME_INTERVAL 0.5
#define VSYNC true

#define PIXEL_SIZE 5
#define BUFFER_WIDTH (SCREEN_WIDTH / PIXEL_SIZE)
#define BUFFER_HEIGHT (SCREEN_HEIGHT / PIXEL_SIZE)

#define ASSERT(Function, ...) if (!(Function)) { fprintf(stderr, __VA_ARGS__); exit(1); }

typedef struct {
    Uint8 R, G, B, A;
} Color;

static struct {
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_Texture *Texture;
    Uint32 *Pixels;
    const Uint8 *State;
    bool Focused, Quit;
} Engine;

void DrawPixel(int x, int y, Color m_Color) {
    if (x < 0 || x >= BUFFER_WIDTH || y < 0 || y >= BUFFER_HEIGHT) return;
    Engine.Pixels[(y * BUFFER_WIDTH) + x] = (m_Color.A << 24) | (m_Color.B << 16) | (m_Color.G << 8) | m_Color.R;
}

int main(int argc, char *argv[]) {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO), "SDL failed to initialize: %s", SDL_GetError());
    ASSERT((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG), "SDL_image initialization failed: %s\n", IMG_GetError());
    ASSERT(!TTF_Init(), "SDL_ttf initialization failed: %s\n", TTF_GetError());

    Uint32 RendererFlags = SDL_RENDERER_ACCELERATED;
    if (VSYNC) RendererFlags |= SDL_RENDERER_PRESENTVSYNC;

    Engine.Window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED_DISPLAY(0), SDL_WINDOWPOS_CENTERED_DISPLAY(0), SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    Engine.Renderer = SDL_CreateRenderer(Engine.Window, -1, RendererFlags);
    Engine.Texture = SDL_CreateTexture(Engine.Renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, BUFFER_WIDTH, BUFFER_HEIGHT);
    Engine.Pixels = malloc(BUFFER_WIDTH * BUFFER_HEIGHT * 4);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetRenderDrawBlendMode(Engine.Renderer, SDL_BLENDMODE_BLEND);

    SDL_Event Event;

    Uint32 FrameStart, FrameTime;
    int FrameCount = 0;
    Uint32 LastUpdate = SDL_GetTicks();

    while (!Engine.Quit) {
        FrameStart = SDL_GetTicks();
        while (SDL_PollEvent(&Event) != 0) {
            switch (Event.type) {
                case SDL_QUIT: Engine.Quit = true; break;
                case SDL_WINDOWEVENT:
                if (Event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                    Engine.Focused = true;
                else if (Event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                    Engine.Focused = false;
                default: break;
            }
        }
        if (Engine.Quit) break;
        if (Engine.Focused) {
            Engine.State = SDL_GetKeyboardState(NULL);
            if (Engine.State[SDL_SCANCODE_ESCAPE]) {
                SDL_Event QuitEvent;
                QuitEvent.type = SDL_QUIT;
                SDL_PushEvent(&QuitEvent);
            }
        }

        FrameCount++;
        if (SDL_GetTicks() - LastUpdate >= FRAME_INTERVAL * 1000) {
            char NewTitle[256];
            float FPS = (FrameCount / FRAME_INTERVAL);

            snprintf(NewTitle, sizeof(NewTitle), "%s - FPS: %.0f", TITLE, FPS);
            SDL_SetWindowTitle(Engine.Window, NewTitle);

            FrameCount = 0;
            LastUpdate = SDL_GetTicks();
        }

        memset(Engine.Pixels, 0, BUFFER_WIDTH * BUFFER_HEIGHT * 4);

        DrawPixel(10, 10, (Color){.R = 200, .G = 80, .B = 0, .A = 80});

        SDL_UpdateTexture(Engine.Texture, NULL, Engine.Pixels, BUFFER_WIDTH * sizeof(Uint32));

        SDL_RenderClear(Engine.Renderer);
        SDL_RenderCopy(Engine.Renderer, Engine.Texture, NULL, NULL);
        SDL_RenderPresent(Engine.Renderer);

        FrameTime = (SDL_GetTicks() - FrameStart);
        if (FrameTime < 1000 / FRAME_LIMIT) SDL_Delay((1000 / FRAME_LIMIT) - FrameTime);
    }

    SDL_DestroyTexture(Engine.Texture);
    SDL_DestroyRenderer(Engine.Renderer);
    SDL_DestroyWindow(Engine.Window);
    return 0;
}
