#version 460

in vec3 outVertexPos;
in vec2 outTexCoord;

out vec4 fragColor;

uniform sampler2D texture_sampler;

void main() {
    fragColor = texture(texture_sampler, outTexCoord);
    // fragColor = vec4(vec3(1.0), 1.0);
}