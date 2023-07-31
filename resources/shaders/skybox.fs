#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform samplerCube skybox1;

uniform float p;

void main() {
    FragColor = mix(texture(skybox, TexCoords),texture(skybox1, TexCoords), 0.001);
}