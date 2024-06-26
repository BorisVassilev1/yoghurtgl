const float TWO_PI = 6.28318530718;

layout(std140, binding = 0) uniform Matrices {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 cameraWorldMatrix;
};

out vec4 vColor;
out vec2 vTexCoord;
out vec3 vVertexNormal;
out vec3 vVertexPos;

uniform mat4 worldMatrix;
uniform float time;

layout(location = 0) in vec4 bladeData0;
layout(location = 1) in vec4 bladeData1;
layout(location = 2) in uint bladeData2;

uint PCGHash(inout uint seed) {
	seed	  = seed * 747796405u + 2891336453u;
	uint word = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
	return (word >> 22u) ^ word;
}

float randomFloat(inout uint seed) { return float(PCGHash(seed)) / 4294967296.0; }

vec3 bezierCurve(float x, vec3 v0, vec3 v1, vec3 v2) {
	return mix(mix(v0, v1, x), mix(v1, v2, x), x);
}

vec3 bezierDerivative(float x, vec3 v0, vec3 v1, vec3 v2) {
	return v0 * (2*x-2) + (2*v2-4*v1) * x + 2 * v1;
}

uniform float curvature = 0.6;
uniform float facingOffset = 0.8;
uniform float height = 1.0;
uniform float width = 0.1;

uniform float NORMAL_TERRAIN_BLEND_START = 40;
uniform float NORMAL_TERRAIN_BLEND_END = 120;

uniform uint blade_triangles;
uniform uint LOD = 0;

uniform float WIDEN_BLADES = 0.04;

void main() {
	uint seed = bladeData2;
	vec3 bladePosition = bladeData0.xyz;
	float windStrength = bladeData0.w;
	vec2 facing = bladeData1.xy;
	vec2 size = bladeData1.zw;
	size.y *= height;

	float distanceAlongBlade = (gl_VertexID / 2) / float(blade_triangles / 2);
	float leftOrRight = (gl_VertexID % 2 - 0.5) * 2. + float(gl_VertexID == 14);

	// float distanceAlongBlade = color.x;
	// float leftOrRight = color.y;


	// construct everything on the y/z plane
	vec3 target = vec3(0, size.y, facingOffset);
	
	// bobbing still in 2d
	float bobbingOffset = TWO_PI * randomFloat(seed) - distanceAlongBlade * 3.14;
	float bobbingFreq = 4.;
	float bobbingStrength = 0.15;
	target.y += (bobbingStrength * sin(time * bobbingFreq + bobbingOffset) - windStrength) * 1;
	target.z -= (bobbingStrength * sin(time * bobbingFreq + bobbingOffset) - windStrength ) * 0.5;

	// calculate the mid point
	vec3 lineNormal = normalize(vec3(0, facingOffset, -size.y));
	vec3 mid = target * 0.7 + lineNormal * curvature;
	

	// generate vertex data in 2d
	vec3 vertexPos = bezierCurve(distanceAlongBlade, vec3(0), mid, target);
	vec3 derivative = bezierDerivative(distanceAlongBlade, vec3(0), mid, target);
	vec3 curveNormal = vec3(0, derivative.z, -derivative.y);

	curveNormal.x -= 0.5 * (leftOrRight);

	// construct the rotation matrix
	vec3 orthogonal = vec3(facing.y, 0, -facing.x);
	mat3 rotateFacing = mat3(-orthogonal, vec3(0, 1, 0), vec3(-facing.x, 0, facing.y));

	// rotate blade accordingly
	vertexPos = vertexPos * rotateFacing;
	curveNormal = curveNormal * rotateFacing;

	vec4 vPos = vec4(vertexPos, 1.);	

	// offset globally
	vPos.xyz += bladePosition;

	// blade width
	vPos.xyz += orthogonal * width * leftOrRight * pow((1 - distanceAlongBlade), 0.7);

	vPos = worldMatrix * vPos;

	vec3 camPos = (cameraWorldMatrix * vec4(0, 0, 0, 1)).xyz;
	float viewDistance = length(vPos.xyz - camPos);

	// blend normal with terrain normal
	float normalBlendFactor = smoothstep(NORMAL_TERRAIN_BLEND_START, NORMAL_TERRAIN_BLEND_END, viewDistance);
	curveNormal = mix(curveNormal, vec3(0, 1, 0), normalBlendFactor);

	vColor.x = distanceAlongBlade;
	vColor.y = leftOrRight;

	vTexCoord = vec2((leftOrRight + 1.) / 2., distanceAlongBlade);
	vVertexNormal = normalize(worldMatrix * vec4(curveNormal, 0.0)).xyz;


	vec3 viewSpaceVNormal = (viewMatrix * vec4(facing.x, 0, facing.y, 0.0)).xyz;
	float normalCameraAngle = dot(viewSpaceVNormal, vec3(0, 0, 1));
	float wideningCoef = (1. - smoothstep(0.1, 0.5, abs(normalCameraAngle)));
	float dir = smoothstep(-0.3, 0.3, normalCameraAngle) * 2. - 1.;

    vVertexPos = vPos.xyz;

	vec4 viewSpaceVPos = projectionMatrix * viewMatrix * vPos;
	viewSpaceVPos.x += (1. + wideningCoef) * leftOrRight * dir * 0.04;
	gl_Position = viewSpaceVPos;
}
