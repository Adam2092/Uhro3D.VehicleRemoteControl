//#include <cluon/OD4Session.hpp>
//#include <cluon/Envelope.hpp>

#include "cluon-complete.hpp"
//#include "messages.hpp"

//#include <cstdint>
//#include <chrono>
#include <iostream>
#include <thread>

//#include <Urho3D/ThirdParty/SDL/SDL.h>
#include <SDL2/SDL.h>

void set_pixel(SDL_Surface* surface, int i, int j, uint32_t pixel)
{
    uint8_t* target_pixel = (uint8_t*)surface->pixels + j*surface->pitch + i*4;
    *(uint32_t*)target_pixel = pixel;
}

int main(int argc, char** argv)
{
    unsigned imgWidth = 640, imgHeight = 480;
    auto cmdlArg = cluon::getCommandlineArguments(argc, argv);
//    unsigned CID = 113;
    std::string NAME{"/vircam0"};
//    unsigned FREQ = 20; // not necessary as data-triggered

//    if ( 0 == cmdlArg.count("cid")) std::cout << "No CID argument detected, using default (--cid=113) now." << std::endl;
//    else CID = std::stoi(cmdlArg["cid"]);
    if ( 0 == cmdlArg.count("name")) std::cout << "No NAME argument detected, using default (--name=/vircam0) now." << std::endl;
    else NAME = cmdlArg["name"];
//    if ( 0 == cmdlArg.count("freq")) std::cout << "No FREQ argument detected, using default (--freq=20) now." << std::endl;
//    else FREQ = std::stoi(cmdlArg["freq"]); // not necessary as data-triggered
    
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Camera Viewer", 5 /*SDL_WINDOWPOS_UNDEFINED*/, 5 /*SDL_WINDOWPOS_UNDEFINED*/, imgWidth, imgHeight, 0 /*SDL_WINDOW_INPUT_GRABBED*/);
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    SDL_LockSurface(surface);
    for (int i=0; i<imgWidth; i++)
        for (int j=0; j<imgHeight; j++)
            {
                float b = i * (255.0 / imgWidth);
//                b = 0;
                float g = j * (255.0 / imgHeight);
//                g = 0;
                float r = (i*j) * (255.0 / (imgWidth*imgHeight));
                r = 0;
                float a = (i+j) * (255.0 / (imgWidth+imgHeight)); 
                uint32_t pixel = ((uint8_t)a << 24u) | ((uint8_t)r << 16u) | ((uint8_t)g << 8u) | (uint8_t)b;
//                if (i%64 == 0 && j%48 == 0)
//                {
//                    std::cout << "(" << i << ", " << j << "): " << std::endl;
//                    std::cout << "r=" << r << ", g=" << g << ", b=" << b << ", a=" << a << std::endl;
//                    std::cout << "pixel=" << pixel << std::endl;
//                }
                set_pixel(surface, i, j, pixel);
            }
    SDL_UnlockSurface(surface);
    SDL_UpdateWindowSurface(window);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // temporary wait for the main sim to be initialised
    
//    std::cout << NAME << std::endl;
    cluon::SharedMemory shm{NAME};
    if (shm.valid())
    {
        std::cout << "SharedMemory " << NAME << " connected." << std::endl << "Press M in Urho3D sim window to toggle Camera view." << std::endl;
        std::cout << "Press ESC in Camera Viewer to quit (before quitting the main Urho3D sim window!!)" << std::endl;
    }
//    int delay = 1000/FREQ; // not necessary as data-triggered
    while (shm.valid())
    {
//        std::this_thread::sleep_for(std::chrono::milliseconds(delay)); // not necessary as data-triggered
        
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_KEYUP:
                case SDL_KEYDOWN:
//                    std::cout << "Keyboard detected. Quitting Camera Viewer..." << std::endl;
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                }
                default: break;
            }
        }
        
        shm.wait();
        shm.lock();
        SDL_LockSurface(surface);
        unsigned *dataPtr = reinterpret_cast<unsigned *>(shm.data());
        for (int i=0; i<imgWidth; i++)
            for (int j=0; j<imgHeight; j++)
            {
            unsigned pixel = *dataPtr;
                // Rearrange RGBA to BGRA (from low to high)
                uint8_t r = (pixel >> 0u) & 0xffu;
                uint8_t g = (pixel >> 8u) & 0xffu;
                uint8_t b = (pixel >> 16u) & 0xffu;
                uint8_t a = (pixel >> 24u) & 0xffu;
                pixel = (a << 24u) | (r << 16u) | (g << 8u) | b;
//                set_pixel(surface, i, j, *dataPtr);
                set_pixel(surface, i, j, pixel);
                dataPtr++;
            }
        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
        
        shm.unlock();
//        shm.notifyAll(); // Consumers do not need to notify
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
