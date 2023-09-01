#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
     sampler2D texture_diffuse1;
     //sampler2D texture_specular1;

     float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 1

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLight[NR_POINT_LIGHTS];
uniform Material material;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = CalcDirLight(dirLight, normal, viewDir);

    for (int i = 0; i < NR_POINT_LIGHTS; i++){
        result += CalcPointLight(pointLight[i], normal, FragPos, viewDir);
    }

    FragColor = texture(material.texture_diffuse1, TexCoords)*vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    //vec3 halfwayDir = normalize(lightDir + viewDir);
    //float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    //vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords).xxx);

    return (ambient + diffuse);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    //vec3 halfwayDir = normalize(lightDir + viewDir);
    //float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (distance * distance);

    vec3 ambient = light.ambient * attenuation * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * attenuation * vec3(texture(material.texture_diffuse1, TexCoords));
    //vec3 specular = light.specular * spec * attenuation * vec3(texture(material.texture_specular1, TexCoords).xxx);

    return(ambient + diffuse);
}