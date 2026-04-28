#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

namespace fs = std::filesystem;

std::vector<std::string> get_roms(const std::string& path) {
    std::vector<std::string> roms;
    for (auto& p : fs::directory_iterator(path)) {
 
     if (p.path().extension() == ".smc" || p.path().extension() == ".sfc") {
            roms.push_back(p.path().string());
        }
    }
    return roms;
}

std::string clean_name(const std::string& path) {
    return fs::path(path).stem().string();
}


int main() {
    // --- SDL INIT ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "SDL init failed\n";
        return 1;
    }

    // --- TTF INIT ---
    if (TTF_Init() != 0) {
        std::cerr << "TTF init failed\n";
        return 1;
    }

    // --- OPEN SNES CONTROLLER ---
    SDL_Joystick* js = SDL_JoystickOpen(0);

    // --- WINDOW + RENDERER ---
    SDL_Window* win = SDL_CreateWindow(
        "DeLorean",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    // --- LOAD FONT ---
    TTF_Font* font = TTF_OpenFont("./DejaVuSans.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font\n";
        return 1;
    }

 
  // --- SPLASH SCREEN ---
    Uint32 startTime = SDL_GetTicks();
    bool splash = true;
    while (splash) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) splash = false;
            if (e.type == SDL_KEYDOWN) splash = false;
        }

        Uint32 now = SDL_GetTicks();
        if (now - startTime > 2000) splash = false;

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, 0, 128, 255, 255);
        SDL_Rect logo = { 440, 260, 400, 200 };
        SDL_RenderFillRect(ren, &logo);

        SDL_RenderPresent(ren);
    }

    // --- LOAD ROMS ---
    std::vector<std::string> roms = get_roms("./ROMs/SNES");
    int index = 0;
    bool running = true;
    SDL_Event e;

    // --- MAIN LOOP ---
    while (running) {

        // --- INPUT ---
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            // SNES controller axis
            if (e.type == SDL_JOYAXISMOTION) {
                if (e.jaxis.axis == 1) { // vertical
                    if (e.jaxis.value < -8000) index--;
                    if (e.jaxis.value >  8000) index++;
                }
            }

            // SNES controller button
 
       if (e.type == SDL_JOYBUTTONDOWN) {
                if (e.jbutton.button == 0) {
                    std::string cmd = "mednafen \"" + roms[index] + "\" &";
                    system(cmd.c_str());
                }
            }

            // Keyboard fallback
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_UP) index--;
                if (e.key.keysym.sym == SDLK_DOWN) index++;
            }
        }

        // Wrap index
        if (index < 0) index = roms.size() - 1;
        if (index >= roms.size()) index = 0;

        // --- RENDER ---
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        // Draw ROM wheel
        int startY = 200;
        for (int i = 0; i < roms.size(); i++) {
            int y = startY + (i - index) * 50;

            SDL_Color color = (i == index)
                ? SDL_Color{0, 255, 255}
                : SDL_Color{255, 255, 255};

            SDL_Surface* surf = TTF_RenderText_Solid(font, roms[i].c_str(), color);
            SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);

            SDL_Rect dst = { 100, y, surf->w, surf->h };
            SDL_RenderCopy(ren, tex, NULL, &dst);

            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        SDL_RenderPresent(ren);
    }

    return 0;
}
