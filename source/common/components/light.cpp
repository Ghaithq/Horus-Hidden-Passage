#include "light.hpp"
#include "../ecs/entity.hpp"
#include <glm/glm.hpp>
#include "../deserialize-utils.hpp"

namespace our {
    void LightComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        std::string type_name = data.value("light_type", "point");
        if(type_name =="directional")
            lightType=LightType::DIRECTIONAL;
        else if(type_name =="spot")
            lightType=LightType::SPOT;
        else
            lightType=LightType::POINT;
        specular=data.value<glm::vec3>("specular", specular);
        color = data.value("color", color);
        ambient=data.value("ambient", ambient);
        direction = data.value("direction", direction);
        diffuse=data.value("diffuse", diffuse);
        spot_angle.inner=data.value("angle_inner",0.f);
        spot_angle.outer=data.value("angle_outer",0.f);
        glm::vec3 att = data.value("attenuation", glm::vec3(1.0f));
        attenuation.constant=att[0];
        attenuation.linear=att[1];
        attenuation.quadratic=att[2];
        
        
    }

    
}