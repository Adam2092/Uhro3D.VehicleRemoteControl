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

// Duplicated and modified by Yue Kang from VehicleDemo.cpp

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/Constraint.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/Image.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/Math/Color.h>

#include "Vehicle.h"
#include "VehicleRemoteControl.h"

#include <Urho3D/DebugNew.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <utility>
#include <memory>
#include <cstdlib>
#include <cstdio>

const float CAMERA_DISTANCE = 40.0f;

// Temp global counter for cout debug in HandlePostUpdate
// Will be removed
//unsigned count = 0;

URHO3D_DEFINE_APPLICATION_MAIN(VehicleRemoteControl)


VehicleRemoteControl::VehicleRemoteControl(Context* context) :
    Sample(context)
{
    // Register factory and attributes for the Vehicle component so it can be created via CreateComponent, and loaded / saved
    Vehicle::RegisterObject(context);

    CID = 113;
    FREQ = 30;

    // By using this macro, static functions in ProcessUtils.h (e.g. ParseArguments()) should be called
    auto arguments = Urho3D::GetArguments();
//    std::cout << "Received " << arguments.Size() << " arguments." <<std::endl;
    std::string NAME{"/vircam0"};
    if (0 != arguments.Size())
    {
        // The following part converts vector into char**
        std::vector<char*> argString;
        argString.reserve(arguments.Size());
        for (unsigned int i=0; i<arguments.Size(); i++)
        {
            argString.push_back(const_cast<char*>(arguments[i].CString()));
        }

        // And then parses the arguments using cluon
        auto commandlineArguments = cluon::getCommandlineArguments(arguments.Size(), &argString[0]);
        if (0 == commandlineArguments.count("cid"))
            std::cout << "No cid set. Using default parameter: --cid=113" << std::endl;
        else CID = std::stoi(commandlineArguments["cid"]);

        if (0 == commandlineArguments.count("freq"))
            std::cout << "No freq set. Using default parameter: --freq=30" << std::endl;
        else FREQ = std::stoi(commandlineArguments["freq"]);
        if (FREQ >= 40)
            std::cerr << "WARNING: Frequency too high. SharedMemory-related functions (which are time-triggered delegates) might violate allocated time slice." << std::endl;
        
        if (0 == commandlineArguments.count("name"))
            std::cout << "No virtual device name set. Using default parameter: --name=vircam0" << std::endl;
        else NAME = commandlineArguments["name"];
    }
    else
    {
        std::cout << "No argument detected. Using default parameters: --cid=113 --freq=50 --name=vircam0" << std::endl;
    }

    od4 = std::shared_ptr<cluon::OD4Session>(new cluon::OD4Session{CID});

    if (od4->isRunning())
    {
        std::cout << "VehicleRemoteControl::VehicleRemoteControl() OK." << std::endl;
    }
    else
    {
        std::cerr << "Failed to start OD4Session." << std::endl;
    }

    imgWidth = 640;
    imgHeight = 480;
    uint32_t const BPP = 32; // bits per pixel
    uint32_t const SIZE = imgWidth*imgHeight*BPP/8;
    shm = std::unique_ptr<cluon::SharedMemory>(new cluon::SharedMemory(NAME, SIZE));
    isImgSharing = false;
    if (shm && shm->valid())
    {
        std::cout << "SharedMemory initialized: " << shm->name() << std::endl;
    }
    else
    {
        std::cerr << "Failed to initialize SharedMemory." << std::endl;
    }

}

void VehicleRemoteControl::Start()
{
    // Execute base class startup
    Sample::Start();

    // Create static scene content
    CreateScene();

    // Create the controllable vehicle
    CreateVehicle();

    // Create the UI content
    CreateInstructions();

    // Subscribe to necessary events
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_RELATIVE);

    if(od4->isRunning() == 0)
    {
        std::cout << "WARNING from In VehicleRemoteControl::Start(): NO OD4 SESSION RUNNING!!!" << std::endl;
    }
    else std::cout << "VehicleRemoteControl::Start() OK." << std::endl;
}

void VehicleRemoteControl::CreateScene()
{
    auto* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create scene subsystem components
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<PhysicsWorld>();

    // Create camera and define viewport. We will be doing load / save, so it's convenient to create the camera outside the scene,
    // so that it won't be destroyed and recreated, and we don't have to redefine the viewport on load
    cameraNode_ = new Node(context_);
    auto* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(500.0f);

    /* Added another viewport for camera sensor here by Yue Kang*/
    GetSubsystem<Renderer>()->SetNumViewports(2);
    GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

    // Create static scene content. First create a zone for ambient lighting and fog control
    Node* zoneNode = scene_->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    zone->SetFogStart(300.0f);
    zone->SetFogEnd(500.0f);
    zone->SetBoundingBox(BoundingBox(-2000.0f, 2000.0f));

    // Create a directional light with cascaded shadow mapping
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
    auto* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
    light->SetSpecularIntensity(0.5f);

    // Create heightmap terrain with collision
    Node* terrainNode = scene_->CreateChild("Terrain");
    terrainNode->SetPosition(Vector3::ZERO);
    auto* terrain = terrainNode->CreateComponent<Terrain>();
    terrain->SetPatchSize(64);
    terrain->SetSpacing(Vector3(2.0f, 0.1f, 2.0f)); // Spacing between vertices and vertical resolution of the height map
    terrain->SetSmoothing(true);
    terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.png"));
    terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
    // The terrain consists of large triangles, which fits well for occlusion rendering, as a hill can occlude all
    // terrain patches and other objects behind it
    terrain->SetOccluder(true);

    auto* body = terrainNode->CreateComponent<RigidBody>();
    body->SetCollisionLayer(2); // Use layer bitmask 2 for static geometry
    auto* shape = terrainNode->CreateComponent<CollisionShape>();
    shape->SetTerrain();

    // Create 1000 mushrooms in the terrain. Always face outward along the terrain normal
    const unsigned NUM_MUSHROOMS = 500;
    for (unsigned i = 0; i < NUM_MUSHROOMS; ++i)
    {
        Node* objectNode = scene_->CreateChild("Mushroom");
        Vector3 position(Random(2000.0f) - 1000.0f, 0.0f, Random(2000.0f) - 1000.0f);
        position.y_ = terrain->GetHeight(position) - 0.1f;
        objectNode->SetPosition(position);
        // Create a rotation quaternion from up vector to terrain normal
        objectNode->SetRotation(Quaternion(Vector3::UP, terrain->GetNormal(position)));
        objectNode->SetScale(3.0f);
        auto* object = objectNode->CreateComponent<StaticModel>();
        object->SetModel(cache->GetResource<Model>("Models/Mushroom.mdl"));
        object->SetMaterial(cache->GetResource<Material>("Materials/Mushroom.xml"));
        object->SetCastShadows(true);

        auto* body = objectNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2);
        auto* shape = objectNode->CreateComponent<CollisionShape>();
        shape->SetTriangleMesh(object->GetModel(), 0);
    }

    if(od4->isRunning() == 0)
    {
        std::cout << "WARNING from In VehicleRemoteControl::CreateScene(): NO OD4 SESSION RUNNING!!!" << std::endl;
    }
    else std::cout << "VehicleRemoteControl::CreateScene() OK." << std::endl;


//    // Trying to have a separated SDL window for the camera sensor, not functioning yet
//    /*SDL_Window *window;*/ //
//    SDL_Init(SDL_INIT_EVERYTHING);              // Initialize SDL2

//    // Create an application window with the following settings:
//    window = SDL_CreateWindow(
//        "An SDL2 window",                  // window title
//        SDL_WINDOWPOS_UNDEFINED,           // initial x position
//        SDL_WINDOWPOS_UNDEFINED,           // initial y position
//        imgWidth,                          // width, in pixels
//        imgHeight,                         // height, in pixels
//        SDL_WINDOW_OPENGL                  // flags
//    );
}

void VehicleRemoteControl::CreateVehicle()
{
    Node* vehicleNode = scene_->CreateChild("Vehicle");
    vehicleNode->SetPosition(Vector3(0.0f, 2.0f, 0.0f));

    // Create the vehicle logic component
    vehicle_ = vehicleNode->CreateComponent<Vehicle>();
    // Create the rendering and physics components
    vehicle_->Init();
    vehicle_->isManualControl = true;

    auto pedalRequest = [this](cluon::data::Envelope &&env) {
//        std::lock_guard<std::mutex> lck(dataMutex);
        ppr = cluon::extractMessage<opendlv::proxy::PedalPositionRequest>(std::move(env));
    };

    auto steerRequest = [this](cluon::data::Envelope &&env) {
//        std::lock_guard<std::mutex> lck(dataMutex);
        gsr = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(env));
    };

    if(od4->isRunning())
    {
        std::cout << "VehicleRemoteControl::CreateVehicle() OK." << std::endl;
        od4->dataTrigger(opendlv::proxy::PedalPositionRequest::ID(), pedalRequest);
        od4->dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(), steerRequest);
        std::cout << "dataTrigger function set." << std::endl;
    }
    else 
    {
        std::cout << "WARNING from In VehicleRemoteControl::CreateVehicle(): NO OD4 SESSION RUNNING!!!" << std::endl;
    }

    // Create window for camera sensor viewport
        // The following functions have been called in Vehicle::Init()
        // vehicle_->cameraSensorNode_ = scene_->CreateChild("CameraSensor");
    //    vehicle_->cameraSensorNode_->SetPosition(/*coor*/);
    auto* cameraSensor = vehicle_->cameraSensorNode_->CreateComponent<Camera>();
    cameraSensor->SetFarClip(300.0f); // Currently the same with the main viewport, might be customized afterwards
    cameraSensor->SetZoom(1.0f); // Currently set to no zoom, might be customized afterwards
    auto* graphics = GetSubsystem<Graphics>();
    SharedPtr<Viewport> CS_viewport(new Viewport(context_, scene_, cameraSensor, IntRect(graphics->GetWidth()-32-imgWidth, 32, graphics->GetWidth()-32, 32+imgHeight)));
    GetSubsystem<Renderer>()->SetViewport(1, CS_viewport);

//    // Initiate image pointer
//    imagePtr = std::shared_ptr<Urho3D::Image>(new Urho3D::Image(context_));
//    imagePtr-> SetSize(640, 480, 3);
}

void VehicleRemoteControl::CreateInstructions()
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    auto* instructionText = ui->GetRoot()->CreateChild<Text>();
//    instructionText->SetText(
//        "Use WASD keys to drive, mouse/touch to rotate camera\n"
//        "F5 to save scene, F7 to load"
//    );
    instructionText->SetText(
        "Use mouse / touch to rotate camera\n"
        "F5 to save scene, F7 to load\n"
        "C to toggle between WASD drive and message-based drive\n"
        "M to toggle between enabling / disabling ShM image sharing\n"
        "(Temp) I to take screenshot"
    );
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
    // The text has multiple rows. Center them in relation to each other
    instructionText->SetTextAlignment(HA_CENTER);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4 + 120);
}

void VehicleRemoteControl::SubscribeToEvents()
{
    // Subscribe to Update event for setting the vehicle controls before physics simulation
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(VehicleRemoteControl, HandleUpdate));

    // Subscribe to PostUpdate event for updating the camera position after physics simulation
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(VehicleRemoteControl, HandlePostUpdate));

    // Unsubscribe the SceneUpdate event from base class as the camera node is being controlled in HandlePostUpdate() in this sample
    UnsubscribeFromEvent(E_SCENEUPDATE);
}

void VehicleRemoteControl::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    auto* input = GetSubsystem<Input>();

    if (vehicle_)
    {
        auto* ui = GetSubsystem<UI>();
//        if (input->GetKeyPress(KEY_Q))
//        {
//            bool mouseLocked = eventData[MouseModeChanged::P_MOUSELOCKED].GetBool();
//            input->SetMouseVisible(!mouseLocked);
//        }

        // Toggle between the two control
        if (input->GetKeyPress(KEY_C))
        {
            vehicle_->isManualControl = !(vehicle_->isManualControl);
            if (vehicle_->isManualControl) std::cout << "Switched to manual (WSAD) control." << std::endl;
            else std::cout << "Switched to message-based control." << std::endl;
        }
        // Toggle between enabling / disabling SharedMemory image sharing
        if (input->GetKeyPress(KEY_M))
        {
            isImgSharing = !(isImgSharing);
            if (isImgSharing) std::cout << "ShM sharing enabled." << std::endl;
            else std::cout << "ShM sharing disabled." << std::endl;
        }
        // Get movement controls and assign them to the vehicle component. If UI has a focused element, clear controls
        if (!ui->GetFocusElement())
        {
            if (vehicle_->isManualControl) {
                vehicle_->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
                vehicle_->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
                vehicle_->controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
                vehicle_->controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
            }
            else if(od4->isRunning())
            {
//                // TODO: overwrite this by implementing a dynamic model for the vehicle
                vehicle_->setSteering(gsr.groundSteering());
                vehicle_->setAccPedal(ppr.position());
            }

            // Add yaw & pitch from the mouse motion or touch input. Used only for the camera, does not affect motion
            if (touchEnabled_)
            {
                for (unsigned i = 0; i < input->GetNumTouches(); ++i)
                {
                    TouchState* state = input->GetTouch(i);
                    if (!state->touchedElement_)    // Touch on empty space
                    {
                        auto* camera = cameraNode_->GetComponent<Camera>();
                        if (!camera)
                            return;

                        auto* graphics = GetSubsystem<Graphics>();
                        vehicle_->controls_.yaw_ += TOUCH_SENSITIVITY * camera->GetFov() / graphics->GetHeight() * state->delta_.x_;
                        vehicle_->controls_.pitch_ += TOUCH_SENSITIVITY * camera->GetFov() / graphics->GetHeight() * state->delta_.y_;
                    }
                }
            }
            else
            {
                vehicle_->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
                vehicle_->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;
            }
            // Limit pitch
            vehicle_->controls_.pitch_ = Clamp(vehicle_->controls_.pitch_, 0.0f, 80.0f);

            // Check for loading / saving the scene
            if (input->GetKeyPress(KEY_F5))
            {
                File saveFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/VehicleRemoteControl.xml",
                    FILE_WRITE);
                scene_->SaveXML(saveFile);
            }
            if (input->GetKeyPress(KEY_F7))
            {
                File loadFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/VehicleRemoteControl.xml", FILE_READ);
                scene_->LoadXML(loadFile);
                // After loading we have to reacquire the weak pointer to the Vehicle component, as it has been recreated
                // Simply find the vehicle's scene node by name as there's only one of them
                Node* vehicleNode = scene_->GetChild("Vehicle", true);
                if (vehicleNode)
                    vehicle_ = vehicleNode->GetComponent<Vehicle>();
            }

            // Temp: 
            if (input->GetKeyPress(KEY_I))
            {
                auto* graphics = GetSubsystem<Graphics>();
                std::shared_ptr<Urho3D::Image> scrPtr = std::shared_ptr<Urho3D::Image>(new Urho3D::Image(context_));
                scrPtr-> SetSize(graphics->GetWidth(), graphics->GetHeight(), 3);
                graphics->TakeScreenShot(*scrPtr);
                if (scrPtr->SaveBMP("Test.bmp"))
                    std::cout << "Screenshot saved." << std::endl;
                IntRect* temp = new IntRect(graphics->GetWidth()-32-imgWidth, 32, graphics->GetWidth()-32, 32+imgHeight);
                
                Image* camImg = scrPtr->GetSubimage(*temp);
//                camImg->Resize(imgWidth, imgHeight);
                if (camImg->SaveBMP("TestCam.bmp"))
                {
                    std::cout << "Camera saved." << std::endl;
                    // Test for SDL window
//                    // Create an application window with the following settings:
//                    window = SDL_CreateWindow(
//                        "An SDL2 window",                  // window title
//                        SDL_WINDOWPOS_UNDEFINED,           // initial x position
//                        SDL_WINDOWPOS_UNDEFINED,           // initial y position
//                        imgWidth,                          // width, in pixels
//                        imgHeight,                         // height, in pixels
//                        SDL_WINDOW_OPENGL                  // flags
//                    );

//                    SDL_Surface* loadImg = SDL_LoadBMP("TestCam.bmp");
//                    if (window != NULL)
//                    {
//                        SDL_Renderer* ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
//                        std::cout << "0..." << SDL_GetError() << std::endl;
//                        if (ren != NULL)
//                        {
//                            std::cout << "1..." << SDL_GetError() << std::endl;
//                            SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, loadImg);
//                            std::cout << "2..." << SDL_GetError() << std::endl;
//                            SDL_RenderCopy(ren, tex, NULL, NULL);
//                            std::cout << "3..." << SDL_GetError() << std::endl;
//                            SDL_RenderPresent(ren);
//                            std::cout << "4..." << SDL_GetError() << std::endl;
//                        }
//                    }
//                    free(camImg);
                }
            }
        }
        else vehicle_->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_LEFT | CTRL_RIGHT, false);
    } // end of if(vehicle_)
}


void VehicleRemoteControl::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!vehicle_)
        return;

    Node* vehicleNode = vehicle_->GetNode();

    // Send od4 messages in time-triggered way
    if(od4->isRunning())
    {
        WeakPtr<RigidBody> body = vehicle_->hullBody_;
        body->ReAddBodyToWorld();

        Vector3 LinearVel = body->GetLinearVelocity();
        Vector3 AngularVel = body->GetAngularVelocity();

        Vector3 pos = body->GetPosition();//.Normalized(); //necessary?
        Quaternion rota = body->GetRotation();

        auto sendMsgs{[&LinearVel, &AngularVel, &pos, &rota, this]() -> bool
            {
                opendlv::sim::KinematicState kinematicMsg;
                kinematicMsg.vx((float)LinearVel.x_);
                kinematicMsg.vy(LinearVel.y_);
                kinematicMsg.vz(LinearVel.z_);
                kinematicMsg.rollRate(AngularVel.x_);
                kinematicMsg.pitchRate(AngularVel.y_);
                kinematicMsg.yawRate(AngularVel.z_);
                od4->send(kinematicMsg);

                opendlv::sim::Frame msg;
                msg.x(pos.x_);
                msg.y(pos.y_);
                msg.z(pos.z_);
                msg.roll(rota.RollAngle());
                msg.pitch(rota.PitchAngle());
                msg.yaw(rota.YawAngle());
                od4->send(msg);

                return false; // "return true" blocks the process
            }// end of lambda function
        };// end of sendMsgs

        od4->timeTrigger(FREQ,sendMsgs);
    }//end of "send messages"

    // Physics update has completed. Position camera behind vehicle
    // NOTICE: this "camera" represents the main viewport in the window, NOT the camera sensor on the car
    Quaternion dir(vehicleNode->GetRotation().YawAngle(), Vector3::UP);
    dir = dir * Quaternion(vehicle_->controls_.yaw_, Vector3::UP);
    dir = dir * Quaternion(vehicle_->controls_.pitch_, Vector3::RIGHT);

//    Vector3 cameraTargetPos = vehicleNode->GetPosition() - dir * Vector3(0.0f, 0.0f, CAMERA_DISTANCE);
    Vector3 cameraTargetPos = vehicleNode->GetPosition() - dir * Vector3(-15.0f, 0.0f, CAMERA_DISTANCE); // Added an offset to avoid the vehicle being covered by the camera image
    Vector3 cameraStartPos = vehicleNode->GetPosition();

    // Raycast camera against static objects (physics collision mask 2)
    // and move it closer to the vehicle if something in between
    Ray cameraRay(cameraStartPos, cameraTargetPos - cameraStartPos);
    float cameraRayLength = (cameraTargetPos - cameraStartPos).Length();
    PhysicsRaycastResult result;
    scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, cameraRay, cameraRayLength, 2);
    if (result.body_)
        cameraTargetPos = cameraStartPos + cameraRay.direction_ * (result.distance_ - 0.5f);

    cameraNode_->SetPosition(cameraTargetPos);
    cameraNode_->SetRotation(dir);

    // ShM image sharing
    if (od4->isRunning() && isImgSharing && shm->valid())
    {
        auto imgShare{[this]() -> bool
            {
               auto* graphics = GetSubsystem<Graphics>();
                std::shared_ptr<Urho3D::Image> scrPtr = std::shared_ptr<Urho3D::Image>(new Urho3D::Image(context_));
                scrPtr-> SetSize(graphics->GetWidth(), graphics->GetHeight(), 3);
                graphics->TakeScreenShot(*scrPtr);
                IntRect* temp = new IntRect(graphics->GetWidth()-32-imgWidth, 32, graphics->GetWidth()-32, 32+imgHeight);
                
                Image* camImgOrigin = scrPtr->GetSubimage(*temp);
                SharedPtr<Image> camImg{camImgOrigin->ConvertToRGBA()};
                
//                std::cout << "Cam image is ";
//                if (!camImg->IsCompressed()) std::cout << "not ";
//                std::cout << "compressed." << std::endl;
//                std::cout << "The camImage is " << camImg->GetWidth() << " x " << camImg->GetHeight() << " x " << camImg->GetDepth() << " in size." << std::endl;
//                Color samplePix{camImg->GetPixel(camImg->GetWidth()/2, camImg->GetHeight()/2)};
//                std::cout << "The sample pixel is (" << samplePix.r_ << ", " << samplePix.g_ << ", " << samplePix.b_ << ") at coor. (" << camImg->GetWidth()/2 << ", " << camImg->GetHeight()/2 << ")" << std::endl;
//                unsigned pixUInt = samplePix.ToUInt();
//                std::cout << "ToUInt() direct result: " << pixUInt << std::endl;
//                std::cout << "Try to decode: ";
//                unsigned r = (pixUInt >> 0u) & 0xffu;
//                unsigned g = (pixUInt >> 8u) & 0xffu;
//                unsigned b = (pixUInt >> 16u) & 0xffu;
//                std:: cout << "(" << r << ", " << g << ", " << b << ")" << std::endl;
                // Send image to ShM
                shm->lock();
//                std::cout << "Testing: sending to ShM..." << std::endl;
//                std::cout << "Shm Name: " << shm->name() << "; Size: " << shm->size() << std::endl;

                uint8_t *dataPtr = reinterpret_cast<uint8_t *>(shm->data());
//                unsigned *dataPtr = reinterpret_cast<unsigned *>(shm->data());
//                unsigned char *dataPtr = reinterpret_cast<unsigned char *>(shm->data());
//                unsigned char *imgDataPtr = camImg->GetData();
//                uint32_t *imgDataPtr = reinterpret_cast<uint32_t *>(camImg->GetData());
                uint8_t *imgDataPtr = reinterpret_cast<uint8_t *>(camImg->GetData());

//                for (unsigned i=0; i<imgWidth*imgHeight*4; i++) *dataPtr++ = *imgDataPtr++;
                memcpy(dataPtr, camImg->GetData(), imgWidth*imgHeight*4);

                shm->unlock();
                shm->notifyAll();
                free(camImgOrigin);
                return false;
            }
        };
        od4->timeTrigger(FREQ,imgShare);
    }
}
