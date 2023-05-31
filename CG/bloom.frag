#version 330 core

in vec2 TexCoords;

uniform sampler2D tex;
uniform vec2 texelSize;
uniform sampler2D weights;
uniform int weightsSize;
uniform vec2 dir;
uniform float bloom_spread = 1;
uniform float bloom_intensity = 2;

out vec4 outColor;
void main() {
    float inverseWeightsSize = 1.0 / (weightsSize - 1);
    outColor = texture2D(tex, TexCoords) * texture2D(weights, vec2(0.0, 0.0));
    for(int i = 1; i < weightsSize; i++) {
        outColor += texture2D(tex, TexCoords.st + dir * texelSize) *
                    texture2D(weights, vec2(i * inverseWeightsSize, 0.0));

        outColor += texture2D(tex, TexCoords.st - dir * texelSize) *
                    texture2D(weights, vec2(i * inverseWeightsSize, 0.0));
    }

    // vec4 sum = vec4(0.0);
    // for (int n = 0; n < 9; ++n) {
    //     uv_y = (tex_coord.y * size.y) + (bloom_spread * float(n - 4));
    //     vec4 h_sum = vec4(0.0);
    //     h_sum += texelFetch(t0, ivec2(uv_x - (4.0 * bloom_spread), uv_y), 0);
    //     h_sum += texelFetch(t0, ivec2(uv_x - (3.0 * bloom_spread), uv_y), 0);
    //     h_sum += texelFetch(t0, ivec2(uv_x - (2.0 * bloom_spread), uv_y), 0);
    //     h_sum += texelFetch(t0, ivec2(uv_x - bloom_spread, uv_y), 0);
    //     h_sum += texelFetch(t0, ivec2(uv_x, uv_y), 0);
    //     h_sum += texelFetch(t0, ivec2(uv_x + bloom_spread, uv_y), 0);
    //     h_sum += texelFetch(t0, ivec2(uv_x + (2.0 * bloom_spread), uv_y), 0);
    //     h_sum += texelFetch(t0, ivec2(uv_x + (3.0 * bloom_spread), uv_y), 0);
    //     h_sum += texelFetch(t0, ivec2(uv_x + (4.0 * bloom_spread), uv_y), 0);
    //     sum += h_sum / 9.0;
    // }

    // pixel = texture(t0, tex_coord) - ((sum / 9.0) * bloom_intensity);
}
