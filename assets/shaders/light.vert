#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
// Now we need to the surface normal to compute the light so we will send it as an attribute.
layout(location = 3) in vec3 normal;

// We will need to do the light processing in the world space so we will break our transformations into 2 stages:
// 1- Object to World.
uniform mat4 object_to_world;
uniform mat4 object_to_world_inv_transpose; // The inverse transpose will be used to transform the surface normal.
// 2- World to Homogenous Clipspace.
uniform mat4 view_projection;
// The camera position will be used for specular computation.
uniform vec3 camera_position;
uniform mat4 transform;
out Varyings {
    vec4 color;
    vec2 tex_coord;
    // We will need to send the vertex position in the world space,
    vec3 world;
    // the view vector (vertex to eye vector in the world space),
    vec3 view;
    // and the surface normal in the world space.
    vec3 normal;
} vs_out;

void main() {
    // First we compute the world position.
    vs_out.world =(object_to_world * vec4(position, 1.0)).xyz;
    // Then we compute the view vector (vertex to eye vector in the world space) to be used for specular computation later.
    vs_out.view = camera_position - vs_out.world;
    // Then we compute normal in the world space (Note that w=0 since this is a vector).
    vs_out.normal = normalize((object_to_world_inv_transpose * vec4(normal, 0.0f)).xyz);
    // Finally, we compute the position in the homogenous clip space and send the rest of the data.
    gl_Position =view_projection * vec4(vs_out.world, 1.0);
    vs_out.color = color;
    vs_out.tex_coord = tex_coord;
}

// void main(){
//     //TODO: (Req 7) Change the next line to apply the transformation matrix
//     gl_Position = transform*vec4(position, 1.0);
//     vs_out.color = color;
//     vs_out.tex_coord = tex_coord;
// }