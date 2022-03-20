#include <SDL.h>
#include <iostream>
#include <stdexcept>

#include "emulator.h"
#include "invaders.h"

struct Input {
    union {
        struct {
            uint8_t credit : 1;
            uint8_t start2 : 1;
            uint8_t start1 : 1;
            uint8_t : 1;
            uint8_t shot1 : 1;
            uint8_t left1 : 1;
            uint8_t right1 : 1;
            uint8_t : 1;
        };
        uint8_t value;
    } port1;

    union {
        struct {
            uint8_t dip3 : 1;
            uint8_t dip5 : 1;
            uint8_t tilt : 1;
            uint8_t dip6 : 1;
            uint8_t shot2 : 1;
            uint8_t left2 : 1;
            uint8_t right3 : 1;
            uint8_t dip7 : 1;
        };
        uint8_t value;
    } port2;
} input;

struct ShiftRegister {
    uint16_t data;
    uint8_t offset;
} sr;

uint8_t in_callback(uint8_t port) {
    switch (port) {
    case 1:
        return input.port1.value;
    case 2:
        return input.port2.value;
    case 3:
        return sr.data >> sr.offset;
    }
    throw std::runtime_error("Invalid port read");
}

void out_callback(uint8_t port, uint8_t A) {
    switch (port) {
    case 2:
        sr.offset = A & 0x7;
        return;
    case 4:
        sr.data = (sr.data << 8) | A;
        return;
    case 3:
    case 5:
    case 6:
        return; // not implemented
    }
    throw std::runtime_error("Invalid port write");
}

int main() {
    Intel8080 i8080;
    i8080.in_callback = in_callback;
    i8080.out_callback = out_callback;

    auto loadRom = [&i8080](unsigned char *rom, unsigned int len,
                            unsigned off) {
        for (unsigned i = 0; i < len; i++) {
            i8080.memory[i + off] = rom[i];
        }
    };

    loadRom(roms_invaders_h, roms_invaders_h_len, 0x0000);
    loadRom(roms_invaders_g, roms_invaders_g_len, 0x0800);
    loadRom(roms_invaders_f, roms_invaders_f_len, 0x1000);
    loadRom(roms_invaders_e, roms_invaders_e_len, 0x1800);

    input.port2.dip3 = 0;
    input.port2.dip5 = 0;
    input.port2.dip6 = 0;
    input.port2.dip7 = 1;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;

    constexpr int WW = 224;
    constexpr int WH = 256;
    if (SDL_CreateWindowAndRenderer(WW, WH, 0, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't create window and renderer: %s", SDL_GetError());
        return 1;
    }

    bool running = true;
    while (running) {
        auto start = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_c:
                    input.port1.credit = 1;
                    break;
                case SDLK_RETURN:
                    input.port1.start1 = 1;
                    break;
                case SDLK_SPACE:
                    input.port1.shot1 = 1;
                    break;
                case SDLK_LEFT:
                    input.port1.left1 = 1;
                    break;
                case SDLK_RIGHT:
                    input.port1.right1 = 1;
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_c:
                    input.port1.credit = 0;
                    break;
                case SDLK_RETURN:
                    input.port1.start1 = 0;
                    break;
                case SDLK_SPACE:
                    input.port1.shot1 = 0;
                    break;
                case SDLK_LEFT:
                    input.port1.left1 = 0;
                    break;
                case SDLK_RIGHT:
                    input.port1.right1 = 0;
                    break;
                }
                break;
            case SDL_QUIT:
                running = false;
                break;
            }
        }

        if (i8080.halted) {
            running = false;
            break;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_Point points[WW * WH];
        int count = 0;

        for (int i = 0; i < 2; i++) {
            i8080.interrupt(i + 1);
            i8080.execute(2000000 / 120);
            for (int x = (i == 0 ? 0 : 112); x < (i == 0 ? 112 : WH); x++) {
                for (int y = 0; y < WW; y++) {
                    int index = (x * WH + y) / 8, offset = (x * WH + y) % 8;
                    if ((i8080.memory[0x2400 + index] >> offset) & 0x1) {
                        points[count].x = x;
                        points[count].y = WW - y;
                        ++count;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawPoints(renderer, points, count);
        SDL_RenderPresent(renderer);

        auto elapsed = SDL_GetTicks() - start;
        constexpr Uint32 delay = 1000 / 60;
        if (elapsed < delay) {
            SDL_Delay(delay - elapsed);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}