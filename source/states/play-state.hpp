#pragma once

#include <application.hpp>
#include "components/light.hpp"
#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>
#include "components/mesh-renderer.hpp"
#include "components/camera.hpp"
#include "components/free-camera-controller.hpp"
#include "components/movement.hpp"

// This state shows how to use the ECS framework and deserialization.
class Playstate : public our::State
{

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    float time;

    void onInitialize() override
    {
        // First of all, we get the scene configuration from the app config
        auto &config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if (config.contains("assets"))
        {
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our world
        if (config.contains("world"))
        {
            world.deserialize(config["world"]);
        }
        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
        pickUpText = this->getPickUpText();
        pickUpText->localTransform.position.y = -10;
        cameraEntity = getCamera();
        initializeObjectiveItems();
        initializeCounterDisplay();
        initializeTorchLocations();
        time = 0;
    }

    void onDraw(double deltaTime) override
    {
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        // And finally we use the renderer system to draw the scene
        this->checkItemFound();
        this->checkPlayerWin();
        this->onStaircase();
        this->lightTorch();
        this->hideCeiling();
        this->playFootSteps();
        renderer.render(&world);
        // Get a reference to the keyboard object
        auto &keyboard = getApp()->getKeyboard();

        if (keyboard.justPressed(GLFW_KEY_ESCAPE))
        {
            // If the escape  key is pressed in this frame, go to the play state
            getApp()->changeState("menu");
        }
        time += (float)deltaTime;
    }

    void onDestroy() override
    {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        // // Clear the world
        world.clear();
        // // and we delete all the loaded assets to free memory on the RAM and the VRAM
        our::clearAllAssets();
        // Destroy all the members of playstate
        pickUpText = nullptr;
        objectiveItems.clear();
        cameraEntity = nullptr;
        counterDisplay.clear();
        torchLocations.clear();
        itemsHeight.clear();
        gate = nullptr;
        itemCount=0;
        torchCount=0;
        ceiling=nullptr;
    }

private:
    // private utility functions

    // Text that tells the player to pick up the item
    our::Entity *pickUpText = NULL;
    // positions of the items to be picked up
    std::vector<our::Entity *> objectiveItems;
    // To get the position of the player
    our::Entity *cameraEntity = NULL;
    // Counter entities kept in a list
    std::vector<our::Entity *> counterDisplay;
    // The count of items collected so far
    int itemCount = 0;
    // Max. number of items
    const int MAX_ITEMS = 5;
    //Number of torches lit
    int torchCount=0;
    //Max. Number of torches to be lit
    const int MAX_TORCHES=5;
    // Pointer to the exit gate
    our::Entity *gate = nullptr;
    // Torch locations that the player has to ignite during the game
    std::vector<our::Entity *> torchLocations;
    //vector of the original height of items
    std::vector<float> itemsHeight;
    //pointer to the ceiling
    our::Entity* ceiling=nullptr;
    //playTime of footsteps
    float playTime=0;




    our::Entity *getCamera()
    {
        our::CameraComponent *camera = nullptr;
        our::FreeCameraControllerComponent *controller = nullptr;
        for (auto entity : world.getEntities())
        {
            camera = entity->getComponent<our::CameraComponent>();
            controller = entity->getComponent<our::FreeCameraControllerComponent>();
            if (camera && controller)
                return entity;
        }
        return NULL;
    }

    our::Entity *getPickUpText()
    {
        for (auto entity : world.getEntities())
            if (entity->name == "textE")
                return entity;

        return NULL;
    }
    // fills the objectiveItemsPositions vector at the start of the game
    void initializeObjectiveItems()
    {
        for (auto entity : world.getEntities())
            if (entity->name == "objectiveItem")
            {
                itemsHeight.push_back(entity->localTransform.position.y);
                objectiveItems.push_back(entity);
                entity->localTransform.position.y=-10;
            }
    }
    void initializeTorchLocations()
    {
        for (auto entity : world.getEntities())
            if (entity->name == "torch")
                torchLocations.push_back(entity);
    }
    // check if the one of the objective items is found & collect it e
    void checkItemFound()
    {
        const float minDistance = 0.75f;
        our::Entity *foundEntity = nullptr;
        const glm::vec3 &position = cameraEntity->localTransform.position;
        for (auto entity : objectiveItems)
        {
            if (entity->localTransform.position.y>=0&&abs(position.x - entity->localTransform.position.x) + abs(position.z - entity->localTransform.position.z) <= minDistance)
            {
                getApp()->itemFound=true;
                foundEntity = entity;
                break;
            }
        }
        if (foundEntity && getApp()->getKeyboard().justPressed(GLFW_KEY_E))
        {
            foundEntity->localTransform.position.y = -10;
            foundEntity->localTransform.position.x = -100;
            counterDisplay[itemCount]->localTransform.position.z = 1;
            counterDisplay[++itemCount]->localTransform.position.z = -1.5;
            if (itemCount == MAX_ITEMS)
            {
                for (auto entity : world.getEntities())
                {
                    if (entity->name == "gate")
                        gate = entity;
                    if(entity->name=="ceiling")
                        ceiling=entity;
                    if(ceiling&&gate)
                        break;
                }
                gate->getComponent<our::MovementComponent>()->linearVelocity = {0, 0.3, 0};
                ceiling->getComponent<our::MovementComponent>()->linearVelocity = {0, 2, 0};
                
            }
        }
        else if (!foundEntity)
        {
            getApp()->itemFound=false;
        }
    }
    void initializeCounterDisplay()
    {
        counterDisplay.resize(MAX_ITEMS + 1);
        for (auto entity : world.getEntities())
            if (entity->name.substr(0, 7) == "counter")
                counterDisplay[entity->name[7] - '0'] = entity;
    }
    void checkPlayerWin()
    {
        float x = cameraEntity->localTransform.position.x;
        float z = cameraEntity->localTransform.position.z;
        if (itemCount == MAX_ITEMS && x <= -10.45 && z <= 1.2 && z >= -1.2 && gate->localTransform.position.y >= 4.2)
            getApp()->changeState("win");
        if (time >= 150)
            getApp()->changeState("lose");
    }
    void onStaircase()
    {
        float x = cameraEntity->localTransform.position.x;
        float z = cameraEntity->localTransform.position.z;
        float &y = cameraEntity->localTransform.position.y;
        // handle stair elevation increase
        if (x <= -0.71 && x >= -3.43762 && z >= 4.50953 && z <= 6.54807)
        {
            // handle under the stairs secret place
            if (z < 4.8 || abs(y - 1.3) > 0.1)
                y = 0.73582 * z - 2.018205;
        }
        else if (z < 4.5 || x <= -3.45)
            y = 1.3;
    }

    void lightTorch()
    {
        const float minDistance = 1.0f;
        our::Entity *foundEntity = nullptr;
        const glm::vec3 &position = cameraEntity->localTransform.position;
        bool* isEnabled=nullptr;
        for (auto entity : torchLocations)
        {
            isEnabled=&entity->getComponent<our::LightComponent>()->enabled;
            if (!*isEnabled&&entity->localTransform.position.y>=-1 && abs(position.x - entity->localTransform.position.x) + abs(position.z - entity->localTransform.position.z) <= minDistance)
            {
                getApp()->torchFound=true;
                foundEntity = entity;
                break;
            }
        }
        if (foundEntity && getApp()->getKeyboard().justPressed(GLFW_KEY_E))
        {
            torchCount++;
            std::cout<<torchCount<<std::endl;
            *isEnabled=true;
            foundEntity->getComponent<our::LightComponent>()->ambient=glm::vec3(1,1,1);
            pickUpText->localTransform.position.y = -10;
            if(torchCount==MAX_TORCHES)
            {
                int i=0;
                for(const float& height : itemsHeight)
                {
                    objectiveItems[i++]->localTransform.position.y=height;
                }
            }
        }
        if(!foundEntity)
            getApp()->torchFound=false;
    }
    void hideCeiling()
        {
            if(ceiling && ceiling->localTransform.position.y>=30)
            {
                ceiling->localTransform.position.y=-100;
            }
        }
    void playFootSteps(){
        // if(getApp()->getKeyboard().isPressed(GLFW_KEY_W)||getApp()->getKeyboard().isPressed(GLFW_KEY_A)||getApp()->getKeyboard().isPressed(GLFW_KEY_S)||getApp()->getKeyboard().isPressed(GLFW_KEY_D))
        // {
        //         ma_engine_play_sound(getApp()->getSoundEngine(), ".\\assets\\audios\\walking2.mp3", NULL);
        // }
    }
};