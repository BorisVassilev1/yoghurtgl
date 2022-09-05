#version 430
// Triangles
layout(triangles, fractional_even_spacing, ccw) in;

layout(std140, binding=0) uniform Matrices {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 cameraWorldMatrix;
};

in vec4 tcColor[];
in vec2 tcTexCoord[];
in vec3 tcVertexNormal[];

out vec4 teColor;
out vec2 teTexCoord;
out vec3 teVertexNormal;
out vec3 teVertexPos;

uniform sampler2D texture_sampler;
uniform mat4 worldMatrix;

void main() {
	// barycentric coordinates
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;
	// barycentric interpolation
	vec2 texCoord = u * tcTexCoord[0] + v * tcTexCoord[1] + w * tcTexCoord[2];
    vec3 normal = u * tcVertexNormal[0] + v * tcVertexNormal[1] + w * tcVertexNormal[2];
    vec4 color = u * tcColor[0] + v * tcColor[1] + w * tcColor[2];

	vec4 pos0 = gl_in[0].gl_Position;
	vec4 pos1 = gl_in[1].gl_Position;
	vec4 pos2 = gl_in[2].gl_Position;
	// barycentric interpolation
	vec4 pos = u * pos0 + v * pos1 + w * pos2;

    pos.xyz += texture(texture_sampler, texCoord).x * normal * 0.135;

    pos = worldMatrix * pos;
	gl_Position = projectionMatrix * viewMatrix * pos;
    teVertexPos = pos.xyz;
	teTexCoord	= texCoord;
    teVertexNormal = normal;
    teColor = color;
}