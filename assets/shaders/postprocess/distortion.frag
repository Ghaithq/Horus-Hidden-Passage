#version 330 core

uniform sampler2D tex;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
    // Sample the texture
    vec4 color = texture(tex, tex_coord);
    
    // Apply noise
    float noiseIntensity = 0.05; // Adjust noise intensity
    float noise = noiseIntensity * (fract(sin(dot(tex_coord, vec2(12.9898,78.233))) * 43758.5453));
    color.rgb += noise;
    
    // Apply scratches
    float scratchIntensity = 0.1; // Adjust scratch intensity
    float scratch = scratchIntensity * (fract(sin(dot(tex_coord, vec2(23.123, 45.678))) * 87658.234));
    color.rgb -= scratch;
    
    // Apply color distortion
    float distortionIntensity = 0.015; // Adjust distortion intensity
    vec2 distortion = distortionIntensity * vec2(sin(tex_coord.y * 10.0), cos(tex_coord.x * 10.0));
    vec2 distortedCoord = tex_coord + distortion;
    color.rgb = texture(tex, distortedCoord).rgb;
    
    // Set output color
    frag_color = color;
}