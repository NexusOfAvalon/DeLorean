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

    // LOAD FLUX FRAMES
    SDL_Texture* flux1 = nullptr;
    SDL_Texture* flux2 = nullptr;
    SDL_Texture* flux3 = nullptr;
    SDL_Texture* flux4 = nullptr;

    SDL_Surface* f1 = IMG_Load("./Splashscreen/flux1.png");
    SDL_Surface* f2 = IMG_Load("./Splashscreen/flux2.png");
    SDL_Surface* f3 = IMG_Load("./Splashscreen/flux3.png");
    SDL_Surface* f4 = IMG_Load("./Splashscreen/flux4.png");

    if (f1) { flux1 = SDL_CreateTextureFromSurface(ren, f1); SDL_FreeSurface(f1); }
    if (f2) { flux2 = SDL_CreateTextureFromSurface(ren, f2); SDL_FreeSurface(f2); }
    if (f3) { flux3 = SDL_CreateTextureFromSurface(ren, f3); SDL_FreeSurface(f3); }
    if (f4) { flux4 = SDL_CreateTextureFromSurface(ren, f4); SDL_FreeSurface(f4); }

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

        Uint32 elapsed = now - startTime;
        SDL_Texture* flux = nullptr;

        if (elapsed < 3000)       flux = flux1;
        else if (elapsed < 6000)  flux = flux2;
        else if (elapsed < 9000)  flux = flux3;
        else if (elapsed < 12000) flux = flux4;

        if (flux) {
            SDL_Rect flux_rect;
            SDL_QueryTexture(flux, NULL, NULL, &flux_rect.w, &flux_rect.h);

            float fscale = 0.5f;
            flux_rect.w *= fscale;
            flux_rect.h *= fscale;

            flux_rect.x = (1280 - flux_rect.w) / 2;
            flux_rect.y = 200;

            SDL_RenderCopy(ren, flux, NULL, &flux_rect);
        }

        SDL_RenderPresent(ren);
    }

    if (bg_texture) SDL_DestroyTexture(bg_texture);
    if (logo_texture) SDL_DestroyTexture(logo_texture);
    if (flux1) SDL_DestroyTexture(flux1);
    if (flux2) SDL_DestroyTexture(flux2);
    if (flux3) SDL_DestroyTexture(flux3);
    if (flux4) SDL_DestroyTexture(flux4);

    // LOAD ROMS
    std::vector<std::string> roms = get_roms("/home/Dr.E_Brown/DeLorean/ROMs/SNES");
    int index = 0;
    bool running = true;
    SDL_Event e;

    // MAIN LOOP
    while (running) {

        while (SDL_PollEvent(&e)) {

            // Quit event
            if (e.type == SDL_QUIT)
                running = false;

            // TEMP DEBUG
            if (e.type == SDL_JOYBUTTONDOWN) {
                std::cout << "Button pressed: " << (int)e.jbutton.button << "\n";
            }

            // D-PAD NAVIGATION
            if (e.type == SDL_JOYBUTTONDOWN && js) {

                if (e.jbutton.button == 6) index--; // UP
                if (e.jbutton.button == 7) index++; // DOWN
            }

            // FACE BUTTONS (A/B/X/Y)
            if (e.type == SDL_JOYBUTTONDOWN && js) {

                if (e.jbutton.button == 1) { /* A */ }
                if (e.jbutton.button == 2) { /* B */ }
                if (e.jbutton.button == 0) { /* X */ }
                if (e.jbutton.button == 3) { /* Y */ }
            }

            // START launches game, SELECT closes game
            if (e.type == SDL_JOYBUTTONDOWN && js) {

                if (e.jbutton.button == 9) {
                    std::string cmd = "mednafen \"" + roms[index] + "\" &";
                    system(cmd.c_str());
                }

                if (e.jbutton.button == 8) {
                    system("pkill -9 -f mednafen");
                }
            }

            // Keyboard fallback
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_UP) index--;
                if (e.key.keysym.sym == SDLK_DOWN) index++;
            }
        }

        if (index < 0) index = roms.size() - 1;
        if (index >= roms.size()) index = 0;

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        // TITLE TEXT
        SDL_Color titleColor = {255, 255, 0};
        SDL_Surface* tSurf = TTF_RenderText_Solid(font, "DeLorean - Where we're going, we don't need roads.", titleColor);
        SDL_Texture* tTex = SDL_CreateTextureFromSurface(ren, tSurf);
        SDL_Rect tRect = {100, 20, tSurf->w, tSurf->h};
        SDL_RenderCopy(ren, tTex, NULL, &tRect);
        SDL_FreeSurface(tSurf);
        SDL_DestroyTexture(tTex);

        // ROM LIST
        int startY = 150;
        for (int i = 0; i < roms.size(); i++) {
            int y = startY + (i - index) * 50;

            SDL_Color color = (i == index)
                ? SDL_Color{0, 255, 255}
                : SDL_Color{255, 255, 255};

            if (i == index) {
                SDL_Surface* arrowSurf = TTF_RenderText_Solid(font, ">", color);
                SDL_Texture* arrowTex = SDL_CreateTextureFromSurface(ren, arrowSurf);
                SDL_Rect aRect = {60, y, arrowSurf->w, arrowSurf->h};
                SDL_RenderCopy(ren, arrowTex, NULL, &aRect);
                SDL_FreeSurface(arrowSurf);
                SDL_DestroyTexture(arrowTex);
            }

            SDL_Surface* surf = TTF_RenderText_Solid(font, clean_name(roms[i]).c_str(), color);
            SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);

            SDL_Rect dst = {100, y, surf->w, surf->h};
            SDL_RenderCopy(ren, tex, NULL, &dst);

            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }

        // INSTRUCTIONS
        SDL_Color instColor = {200, 200, 200};

        SDL_Surface* inst1 = TTF_RenderText_Solid(font, "Press START to launch game", instColor);
        SDL_Surface* inst2 = TTF_RenderText_Solid(font, "Press SELECT to close game", instColor);

        SDL_Texture* instTex1 = SDL_CreateTextureFromSurface(ren, inst1);
        SDL_Texture* instTex2 = SDL_CreateTextureFromSurface(ren, inst2);

        SDL_Rect i1 = {100, 650, inst1->w, inst1->h};
        SDL_Rect i2 = {100, 680, inst2->w, inst2->h};

        SDL_RenderCopy(ren, instTex1, NULL, &i1);
        SDL_RenderCopy(ren, instTex2, NULL, &i2);

        SDL_FreeSurface(inst1);
        SDL_FreeSurface(inst2);
        SDL_DestroyTexture(instTex1);
        SDL_DestroyTexture(instTex2);

        SDL_RenderPresent(ren);
    }

    return 0;
}
