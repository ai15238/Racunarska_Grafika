#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform samplerCube skybox1;

uniform float p;

void main() {
    FragColor = texture(skybox, TexCoords);
}