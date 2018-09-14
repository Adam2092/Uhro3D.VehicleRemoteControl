//
// Copyright (c) 2008-2018 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "Sample.h"

// Extended by Yue Kang with LibCluon
#include "cluon-complete.hpp"
#include "messages.hpp"

//#include <Urho3D/ThirdParty/SDL/SDL.h>
//#include <Urho3D/ThirdParty/GLEW/glew.h>

namespace Urho3D
{

class Node;
class Scene;

}

class Vehicle;

/// Vehicle example.
/// This sample demonstrates:
///     - Creating a heightmap terrain with collision
///     - Constructing a physical vehicle with rigid bodies for the hull and the wheels, joined with constraints
///     - Defining attributes (including node and component references) of a custom component so that it can be saved and loaded
class VehicleRemoteControl : public Sample
{
    URHO3D_OBJECT(VehicleRemoteControl, Sample);

public:
    /// Construct.
    explicit VehicleRemoteControl(Context* context);

    /// Setup after engine initialization and before running the main loop.
    void Start() override;

private:
    /// Create static scene content.
    void CreateScene();
    /// Create the vehicle.
    void CreateVehicle();
    /// Construct an instruction text to the UI.
    void CreateInstructions();
    /// Subscribe to necessary events.
    void SubscribeToEvents();
    /// Handle application update. Set controls to vehicle.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle application post-update. Update camera position after vehicle has moved.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

    /// The controllable vehicle component.
    WeakPtr<Vehicle> vehicle_;


    // OD4Session for message transfer
    std::shared_ptr<cluon::OD4Session> od4{nullptr};
    std::mutex dataMutex{};
    opendlv::proxy::PedalPositionRequest ppr;
    opendlv::proxy::GroundSteeringRequest gsr;
    uint16_t CID;
    uint16_t FREQ_SHM; // SharedMemory update frequency
    uint16_t FREQ_MSG; // Message broadcast frequency

//    // Image pointer for the camera
//    std::shared_ptr<Urho3D::Image> imagePtr;

//    // Pointer to SDL window
//    SDL_Window *window;

    // Pointer to GL
    
    // Camera image size
    uint16_t imgHeight, imgWidth;

    // Cluon SharedMemory for image transfer
    std::unique_ptr<cluon::SharedMemory> shm;
    // 
    bool isImgSharing;
};
