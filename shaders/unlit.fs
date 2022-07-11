#version 430

#include <rendering.glsl>

in vec4 outColor;
in vec2 outTexCoord;
in vec3 mvVertexNormal;
in vec3 mvVertexPos;

out vec4 fragColor;

uniform bool	  use_texture;
uniform sampler2D texture_sampler;

void main() {
	fragColor = vec4(outColor.xyz, 1.0);
}