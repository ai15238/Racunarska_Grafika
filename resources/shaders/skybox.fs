#version 330 core
out vec4 FragColor;
out vec4 BrightColor;

in vec3 TexCoords;

uniform samplerCube skybox;

uniform float p;

void main() {
    FragColor = texture(skybox, TexCoords);
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}