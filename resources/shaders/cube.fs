#version 330 core
out vec4 FragColor;


uniform vec3 cameraPos;
uniform samplerCube skybox;

in vec3 Normal;
in vec3 FragPos;


void main()
{
    float ratio = 1.0/1.52;
    vec3 I = normalize(FragPos - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb,1.0);
}



