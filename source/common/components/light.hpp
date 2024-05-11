#pragma once

#include "../ecs/component.hpp"

#include <glm/mat4x4.hpp>

namespace our {

    // An enum that defines the type of the light
    enum class LightType {
        DIRECTIONAL,
        POINT,
        SPOT
    };

    class LightComponent : public Component {
    public:
        LightType lightType; // The type of the light
        glm::vec3 diffuse, specular, ambient;
        glm::vec3 color;
        glm::vec3 direction;
        glm::vec3 position;
        bool enabled=true;
        
        struct {
            float constant, linear, quadratic;
        } attenuation; // Used for Point and Spot Lights only
        struct {
            float inner, outer;
        } spot_angle; // Used for Spot Lights only

        // The ID of this component type is "Camera"
        static std::string getID() { return "Light"; }

        // Reads camera parameters from the given json object
        void deserialize(const nlohmann::json& data) override;

        
    };

}