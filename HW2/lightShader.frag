#version 330 core

struct Material {
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    float shininess;
};

in vec3 Normal;
in vec3 FragPos;
in vec3 NormalColor;

out vec4 color;

uniform vec3 viewPos;
uniform Material material;

void main()
{
    // Properties
    vec3 norm = Normal;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(1.0f, 1.0f, 1.0f);
    color = vec4(result, 1.0);
}
