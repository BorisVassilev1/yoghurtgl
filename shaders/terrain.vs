#version 430

#include <rendering.glsl>

out vec4 vColor;
out vec2 vTexCoord;
out vec3 vVertexNormal;
out vec4 vVertexPos;

uniform mat4 worldMatrix;

void main() {
    
    vec4 mvPos = vec4(position, 1.0);
    // mvPos.y += texture(texture_sampler, texCoord).x * 0.135;
	// mvPos = worldMatrix * mvPos;
	
	gl_PointSize = 5.0;
	gl_Position = mvPos;
	
	vColor = color;
	vTexCoord = texCoord;
	vVertexNormal = normalize(worldMatrix * vec4(normal, 0.0)).xyz;
    vVertexPos = worldMatrix * mvPos;
}