#version 430
layout(binding = 1) uniform samplerCube tex;

in vec3 vTexCoord;
out vec4 fragColor;

void main() {
    fragColor = textureLod(tex, vTexCoord, 0);
    fragColor.w = 1.0;
}
