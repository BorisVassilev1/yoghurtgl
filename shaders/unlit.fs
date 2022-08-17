#version 430

#include <rendering.glsl>

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vVertexNormal;
in vec3 vVertexPos;

out vec4 fragColor;

uniform bool	  use_texture;
uniform sampler2D texture_sampler;

void main() {
	fragColor = vec4(outColor.xyz, 1.0);
}