#version 460

in vec3 outVertexPos;
in vec2 outTexCoord;

out vec4 fragColor;

uniform sampler2D  texture_sampler;
uniform sampler2D  texture_sampler_depth;
uniform usampler2D texture_sampler_stencil;

uniform int viewMode = 0;

void main() {
	if (viewMode == 0) {
		fragColor = texture(texture_sampler, outTexCoord);
	} else if (viewMode == 1) {
		fragColor = vec4(vec3(texture(texture_sampler_depth, outTexCoord).x), 1.0);
	} else if (viewMode == 2) {
		fragColor = vec4(vec3(texture(texture_sampler_stencil, outTexCoord).x), 1.0);
	}
}