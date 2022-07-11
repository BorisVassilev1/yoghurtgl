#version 460

layout (location = 0) in vec3 vertexPos;
layout (location = 2) in vec2 texCoord;

out vec3 outVertexPos;
out vec2 outTexCoord;

void main() {
    outVertexPos = vec3(vertexPos.xy, 0.1);
    gl_Position = vec4(vertexPos.xy, 0.0, 1.0);
    // outTexCoord = (vec2(vertexPos.x, -vertexPos.y) + vec2(1.0))/ 2;
    outTexCoord = texCoord;
}