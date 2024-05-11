#version 330

uniform sampler2D tex;

in vec2 tex_coord;
out vec4 frag_color;


#define STEPS 16

#define STRENGTH 0.03

void main(){
    // To apply radial blur, we compute the direction outward from the center to the current pixel
    vec2 step_vector = (tex_coord - 0.5) * (STRENGTH / STEPS);
    // Then we sample multiple pixels along that direction and compute the average
    for(int i = 0; i < STEPS; i++){
        frag_color += texture(tex, tex_coord + step_vector * i);
    }
    frag_color /= STEPS;
}