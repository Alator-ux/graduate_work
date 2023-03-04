#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TPos;

struct PointLight {
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 atten;
};

struct DirectionLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct FlashLight {
    vec3 pos;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float cutOff;
    vec3 atten;
};

uniform PointLight pLight;
uniform DirectionLight dirLight;
uniform FlashLight flashLight;

struct ObjMaterial {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emission;
    float shininess;
};
uniform ObjMaterial material;

uniform sampler2D text;

uniform vec3 viewPos;

out vec4 outColor;

void main()
{
    vec3 viewDir = normalize(viewPos - FragPos);

    // Direction light
    vec3 lightDir = -dirLight.direction;
    vec3 lightReflDir = reflect(lightDir, Normal);

    float NdotL = max(dot(Normal, lightDir), 0);
    float RdotV = max(dot(lightReflDir, viewDir), 0);

    vec3 spec = pow(RdotV, material.shininess) * dirLight.specular * material.specular;
    vec3 diff = NdotL * material.diffuse * dirLight.diffuse;

    vec3 lc2 = spec + diff;
    // -------------------


    vec3 res = lc2;
    res += dirLight.ambient * material.ambient + material.emission;
    //res *= vec3(texture(text, TPos));

    outColor = vec4(min(res, 1.0f), 1.0f);
}