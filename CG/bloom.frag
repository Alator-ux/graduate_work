#version 410

in vec2 TexCoords;

uniform sampler2D tex;
uniform vec2 texelSize;
uniform sampler1D weights;
uniform int weightsSize;
uniform vec2 dir;

out vec4 outColor;
uniform float offset[5] = float[](0.0, 1.0, 2.0, 3.0, 4.0);
uniform float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216,
                                  0.0540540541, 0.0162162162);
 
void main(void) {
    outColor = texture2D(tex, TexCoords) * weight[0];//texture(weights, 0.0);
    for (int i = 1; i < weightsSize; i++) {
        float w = texture(weights, float(i+1) / float(weightsSize)).r;
        outColor += texture2D(tex, TexCoords + vec2(offset[i]) * dir * texelSize) * weight[i];
        outColor += texture2D(tex, TexCoords - vec2(offset[i]) * dir * texelSize) * weight[i];
    }
}
