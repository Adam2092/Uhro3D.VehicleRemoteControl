//#include <cluon/OD4Session.hpp>
//#include <cluon/Envelope.hpp>

#include "cluon-complete.hpp"
#include "messages.hpp"

#include <cstdint>
#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char** argv)
{
    auto cmdlArg = cluon::getCommandlineArguments(argc, argv);
    unsigned CID = 113;

    if ( 0 == cmdlArg.cound("cid")) std::cout << "No CID argument detected, using default (--cid=113) now." << std::endl;
    else CID = std::stoi(cmdlArg["cid"]);

    cluon::OD4Session od4(CID, [](cluon::data::Envelope &&envelope) noexcept 
    {
        if (envelope.dataType() == opendlv::sim::Frame::ID()) 
        {
            opendlv::sim::Frame receivedMsg = cluon::extractMessage<opendlv::sim::Frame>(std::move(envelope));
            std::cout << "Received a message of Frame data:" << std::endl;
            std::cout << " x = " << receivedMsg.x() << ", y = " << receivedMsg.y() << ", z = " << receivedMsg.z() << "." << std::endl;
            std::cout << " roll = " << receivedMsg.roll() << ", pitch = " << receivedMsg.pitch() << ", yaw = " << receivedMsg.yaw() << "." << std::endl;
//            std::cout << " x = " << receivedMsg.x() << std::endl;
        }
        else if (envelope.dataType() == opendlv::sim::KinematicState::ID()) 
        {
            opendlv::sim::KinematicState receivedMsg = cluon::extractMessage<opendlv::sim::KinematicState>(std::move(envelope));
            std::cout << "Received a message of K-State data:" << std::endl;
            std::cout << " vx = " << receivedMsg.vx() << ", vy = " << receivedMsg.vy() << ", vz = " << receivedMsg.vz() << "." << std::endl;
            std::cout << " rollRate = " << receivedMsg.rollRate() << ", pitchRate = " << receivedMsg.pitchRate() << ", yawRate = " << receivedMsg.yawRate() << "." << std::endl;
//            std::cout << " vx = " << receivedMsg.vx() << std::endl;
        }
    });

    while(od4.isRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
//        std::cout << "\e[A" << "\e[A" << "\e[A" << "\e[A" << "\e[A" << "\e[A";
    }
    return 0;
}
