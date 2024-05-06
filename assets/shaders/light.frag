#version 330 core

// We include the common light functions and structures.
// Note that GLSL doesn't support "#include" by default but we the library "stb_include" to recursively include the files as a string preprocessing phase.
float calculate_lambert(vec3 normal, vec3 light_direction){
    return max(0.0f, dot(normal, -light_direction));
}

// This will be used to compute the phong specular.
float calculate_phong(vec3 normal, vec3 light_direction, vec3 view, float shininess){
    vec3 reflected = reflect(light_direction, normal);
    return pow(max(0.0f, dot(view, reflected)), shininess);
}


// // This contains all the material properties for a single pixel.
// // We have an extra property "emissive" which is used when the pixel itself emits light.
struct Material {
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 emissive;
    float shininess;
};

uniform vec4 tint;
uniform sampler2D tex;

in Varyings {
    vec4 color;
    vec2 tex_coord;
    // We will need the vertex position in the world space,
    vec3 world;
    // the view vector (vertex to eye vector in the world space),
    vec3 view;
    // and the surface normal in the world space.
    vec3 normal;
} fs_in;

// // These type constants match their peers in the C++ code.
#define TYPE_DIRECTIONAL    0
#define TYPE_POINT          1
#define TYPE_SPOT           2

// // Now we will use a single struct for all light types.
struct Light {
    // This will hold the light type.
    int type;
    // These defines the colors and intensities of the light.
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 color;

    // Position is used for point and spot lights. Direction is used for directional and spot lights.
    vec3 position, direction;
    // Attentuation factors are used for point and spot lights.
    float attenuation_constant;
    float attenuation_linear;
    float attenuation_quadratic;
    // Cone angles are used for spot lights.
    float inner_angle, outer_angle;
};

// // This will define the maximum number of lights we can receive.
#define MAX_LIGHT_COUNT 16

// // Now we recieve the material, light array and the actual number of lights sent from the cpu.
uniform Material material;
uniform Light lights[MAX_LIGHT_COUNT];
uniform int light_count;

out vec4 frag_color;

void main() {
    // First we normalize the normal and the view. These are done once and reused for every light type.
    vec3 normal = normalize(fs_in.normal);  // Although the normal was already normalized, it may become shorter during interpolation.
    vec3 view = normalize(fs_in.view);

    // Make sure that the actual light count never exceeds the maximum light count.
    int count = min(light_count, MAX_LIGHT_COUNT);

    // We will accumulate the result of all the lights in this variable.
    vec3 accumulated_light = vec3(0.0);

    // Now we will loop over all the lights.
    for(int index = 0; index < count; index++){
        Light light = lights[index];
        vec3 light_direction;
        float attenuation = 1;
        if(light.type == TYPE_DIRECTIONAL)
            light_direction = light.direction; // If light is directional, use its direction as the light direction
        else {
            // If not directional, compute the direction from the position.
            light_direction = fs_in.world - light.position;
            float distance = length(light_direction);
            light_direction /= distance;

            // And compute the attenuation.
            attenuation *= 1.0f / (light.attenuation_constant +
            light.attenuation_linear * distance +
            light.attenuation_quadratic * distance * distance);

            if(light.type == TYPE_SPOT){
                // If it is a spot light, comput the angle attenuation.
                float angle = acos(dot(light.direction, light_direction));
                attenuation *= smoothstep(light.outer_angle, light.inner_angle, angle);
            }
        }

        // Now we compute the 3 components of the light separately.
        vec3 diffuse =light.color* material.diffuse * light.diffuse * calculate_lambert(normal, light_direction);
        vec3 specular = light.color*material.specular * light.specular * calculate_phong(normal, light_direction, view, material.shininess);
        vec3 ambient = material.ambient * light.ambient;

        // Then we accumulate the light components additively.
        accumulated_light += (diffuse + specular) * attenuation + ambient;
        frag_color = tint*fs_in.color * vec4(accumulated_light,1.0)*texture(tex,fs_in.tex_coord);
    }

}

