#version 430

#include <rendering.glsl>

out vec4 outColor;
out vec2 outTexCoord;
out vec3 mvVertexNormal;
out vec3 mvVertexPos;

uniform mat4 worldMatrix;

void main() {
	vec4 mvPos = worldMatrix * vec4(position, 1.0);
	
	gl_PointSize = 5.0;
	gl_Position = projectionMatrix * viewMatrix * mvPos;
	
	outColor = color;
	outTexCoord = texCoord;
	mvVertexNormal = normalize(worldMatrix * vec4(normal, 0.0)).xyz;
    mvVertexPos = mvPos.xyz;
}