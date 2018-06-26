//#include <cluon/OD4Session.hpp>
//#include <cluon/Envelope.hpp>

#include "cluon-complete.hpp"
//#include "messages.hpp"

//#include <cstdint>
//#include <chrono>
#include <iostream>
//#include <thread>

#include <Urho3D/ThirdParty/SDL/SDL.h>

int main(int argc, char** argv)
{
    auto cmdlArg = cluon::getCommandlineArguments(argc, argv);
//    unsigned CID = 113;
    std::string NAME{"/vircam0"};

//    if ( 0 == cmdlArg.count("cid")) std::cout << "No CID argument detected, using default (--cid=113) now." << std::endl;
//    else CID = std::stoi(cmdlArg["cid"]);
    if ( 0 == cmdlArg.count("name")) std::cout << "No NAME argument detected, using default (--name=\"/vircam0\") now." << std::endl;
    else NAME = cmdlArg["name"];
    
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Camera Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderClear(ren);
    
    SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 8, 8);

    cluon::SharedMemory shm{NAME};
    if (shm.valid())
    {
        while(1)
        {
            shm.wait();
            shm.lock();
            shm.unlock();
        }
    }

    return 0;
}
