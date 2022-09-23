#version 430

#include <rendering.glsl>

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vVertexNormal;
in vec3 vVertexPos;

out vec4 fragColor;

uniform vec3 color0 = pow(vec3(0.1, 0.5, 0.1), vec3(2.2));
uniform vec3 color1 = pow(vec3(0.9, 1, 0.7), vec3(2.2));

void main() {
	vec3 albedo = mix(color0, color1, vColor.x * vColor.x);

	vec3 color = calcAllLights(vVertexPos, vVertexNormal, vVertexNormal, vTexCoord, albedo);
	// vec3 color2 = calcAllLights(vVertexPos, -vVertexNormal, vVertexNormal, vTexCoord, albedo);
	// color += 0.5 * color2;

	// color = vec3(materials[material_index].albedo);
	// color = vVertexNormal;

	// color = color / (color + vec3(1.0));
	// color = vec3(vColor.y);
    color = pow(color, vec3(1.0/2.2));

	fragColor = vec4(color, 1.0);
}