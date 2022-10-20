#version 460

in vec3 outVertexPos;
in vec2 outTexCoord;

out vec4 fragColor;

uniform sampler2D  sampler_color;
uniform sampler2D  sampler_depth;
uniform usampler2D sampler_stencil;

uniform int viewMode = 0;

void main() {
	if (viewMode == 0) {
		fragColor = texture(sampler_color, outTexCoord);
	} else if (viewMode == 1) {
		fragColor = vec4(vec3(texture(sampler_depth, outTexCoord).x), 1.0);
	} else if (viewMode == 2) {
		fragColor = vec4(vec3(texture(sampler_stencil, outTexCoord).x), 1.0);
	}
}