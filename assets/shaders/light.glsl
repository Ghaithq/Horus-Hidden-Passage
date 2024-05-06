
#ifndef OUR_LIGHT_COMMON_GLSL_INCLUDED
#define OUR_LIGHT_COMMON_GLSL_INCLUDED

    // This will be used to compute the diffuse factor.
    float calculate_lambert(vec3 normal, vec3 light_direction){
        return max(0.0f, dot(normal, -light_direction));
    }

    // This will be used to compute the phong specular.
    float calculate_phong(vec3 normal, vec3 light_direction, vec3 view, float shininess){
        vec3 reflected = reflect(light_direction, normal);
        return pow(max(0.0f, dot(view, reflected)), shininess);
    }


    // This contains all the material properties for a single pixel.
    // We have an extra property "emissive" which is used when the pixel itself emits light.
    struct Material {
        vec3 diffuse;
        vec3 specular;
        vec3 ambient;
        vec3 emissive;
        float shininess;
    };

    

#endif
