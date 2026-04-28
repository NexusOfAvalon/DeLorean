#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

namespace fs = std::filesystem;

std::vector<std::string> get_roms(const std::string& path) {
    std::vector<std::string> roms;
    try {
        for (auto& p : fs::directory_iterator(path)) {
            if (p.path().extension() == ".smc" || p.path().extension() == ".sfc") {
                roms.push_back(p.path().string());
            }
        }
    } catch (...) {
        std::cerr << "ROM directory not found: " << path << "\n";
    }
    return roms;
}

std::string clean_name(const std::string& path) {
    return fs::path(path).stem().string();
}

int main() {

    // SDL INIT
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "SDL init failed\n";
        return 1;
    }

    // TTF INIT
    if (TTF_Init() != 0) {
        std::cerr << "TTF init failed\n";
        return 1;
    }

    // IMAGE INIT
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "PNG init failed\n";
    }

    // OPEN SNES CONTROLLER
    SDL_Joystick* js = nullptr;
    if (SDL_NumJoysticks() > 0) {
        js = SDL_JoystickOpen(0);
    }

    // WINDOW + RENDERER
    SDL_Window* win = SDL_CreateWindow(
        "DeLorean",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    // LOAD FONT
    TTF_Font* font = TTF_OpenFont("./DejaVuSans.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font\n";
        return 1;
    }

    // SPLASH SCREEN
    Uint32 startTime = SDL_GetTicks();
    bool splash = true;

    SDL_Texture* bg_texture = nullptr;
    SDL_Texture* logo_texture = nullptr;
    SDL_Rect logo_rect = {0, 0, 0, 0};

    SDL_Surface* bg_surface = IMG_Load("./Splashscreen/background.png");
    SDL_Surface* logo_surface = IMG_Load("./Splashscreen/logo.png");

    if (bg_surface) {
        bg_texture = SDL_CreateTextureFromSurface(ren, bg_surface);
        SDL_FreeSurface(bg_surface);
    }
    if (logo_surface) {
        logo_texture = SDL_CreateTextureFromSurface(ren, logo_surface);
        SDL_FreeSurface(logo_surface);
    }

    while (splash) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) splash = false;
            if (e.type == SDL_KEYDOWN) splash = false;
        }

        Uint32 now = SDL_GetTicks();
        if (now - startTime > 12000) splash = false;

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        if (bg_texture) {
            SDL_Rect bg_rect = {0, 0, 1280, 720};
            SDL_RenderCopy(ren, bg_texture, NULL, &bg_rect);
        }

        if (logo_texture) {
            SDL_QueryTexture(logo_texture, NULL, NULL, &logo_rect.w, &logo_rect.h);

            float scale = 0.25f;
            logo_rect.w *= scale;
            logo_rect.h *= scale;

            logo_rect.x = (1280 - logo_rect.w) / 2;
            logo_rect.y = 20;

            SDL_RenderCopy(ren, logo_texture, NULL, &logo_rect);
        }

        SDL_RenderPresent(ren);
    }

    if (bg_texture) SDL_DestroyTexture(bg_texture);
    if (logo_texture) SDL_DestroyTexture(logo_texture);

    // LOAD ROMS
    std::vector<std::string> roms = get_roms("/home/Dr.E_Brown/DeLorean/ROMs/SNES");
    int index = 0;
    bool running = true;
    SDL_Event e;

    // MAIN LOOP
    while (running) {

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (e.type == SDL_JOYAXISMOTION && js) {
                if (e.jaxis.axis == 1) {
                    if (e.jaxis.value < -8000) index--;
                    if (e.jaxis.value >  8000) index++;
                }
            }

            if (e.type == SDL_JOYBUTTONDOWN && js) {
                if (e.jbutton.button == 0) {
                    std::string cmd = "mednafen \"" + roms[index] + "\" &";
                    system(cmd.c_str());
                }
                if (e.jbutton.button == 6) {
                    system("pkill mednafen");
                }
            }

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_UP) index--;
                if (e.key.keysym.sym == SDLK_DOWN) index++;
            }
        }

        if (index < 0) index = roms.size() - 1;
        if (index >= roms.size()) index = 0;

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        int startY = 200;
        for (int i = 0; i < roms.size(); i++) {
            int y = startY + (i - index) * 50;

            SDL_Color color = (i == index)
                ? SDL_Color{0, 255, 255}
                : SDL_Color{255, 255, 255};

            SDL_Surface* surf = TTF_RenderText_Solid(font, roms[i].c_str(), color);
            SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);

            SDL_Rect dst = {100, y, surf->w, surf->h};
            SDL_RenderCopy(ren, tex, NULL, &dst);

            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        SDL_RenderPresent(ren);
    }

    return 0;
}
