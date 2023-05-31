#version 330 core

in vec2 TexCoords;
uniform sampler2D tex;
uniform vec2 texelSize;

out vec4 outColor;
const int SAMPLES = 2;
void main()
{
    vec4 color = vec4(0.0);
    for (int i = 0; i < SAMPLES; i++) {
        for (int j = 0; j < SAMPLES; j++) {
            vec2 offset = vec2(float(i), float(j)) * texelSize;
            color += texture2D(tex, TexCoords.st + offset);
        }
    }
    outColor = vec4(1.0,0.0,0.0,1.0);//color / (SAMPLES * SAMPLES);
}